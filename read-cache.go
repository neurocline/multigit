// read-cache.go

package main

import (
	"bytes"
	"compress/zlib"
	"crypto/sha1"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
)

// -----------------------------------------------------------------------------------------------

// Redundant, in crypto/sha1
type Sha1 [20]byte

// Actually, I need to show the file layout and then
// the marshall/unmarshall code, because we can't/shouldn't
// just map a Go struct to memory, like we're used to in C.
const CACHE_SIGNATURE = 0x44495243

type cache_header struct {
	signature uint32
	version   uint32
	entries   uint32
	sha1hash  Sha1
}

const cache_header_bytes = 3*4 + 20

type cache_time struct {
	sec  uint32
	nsec uint32
}

// Stored packed on disk with a 2-byte length preceding
// the cstring-stored name
type cache_entry struct {
	ctime    cache_time
	mtime    cache_time
	st_dev   uint32
	st_ino   uint32
	st_mode  uint32
	st_uid   uint32
	st_gid   uint32
	st_size  uint32
	sha1hash Sha1
	name     string
}

var active_cache []cache_entry

var db_location string
var DEFAULT_DB_ENVIRONMENT string = ".dircache/objects"

// -----------------------------------------------------------------------------------------------

func usage(msg string) {
	fmt.Fprintf(os.Stderr, "read-tree: %s\n", msg)
	os.Exit(1)
}

func sha1_to_hex(sha1hash Sha1) string {
	return hex.EncodeToString(sha1hash[:])
}

func get_sha1_hex(hexvalue string) (sha1hash Sha1, err error) {
	var hashvalue []byte
	hashvalue, err = hex.DecodeString(hexvalue)
	if err != nil {
		return sha1hash, fmt.Errorf("malformed hex")
	}

	copy(sha1hash[:], hashvalue)
	return sha1hash, nil
}

// Turn a Sha1 into a path in the object database. This assumes
// that objects are binned into sub-directories 00/, 01/, ..FF/
// and the name inside the sub-directory is missing the first octet.
func sha1_file_name(sha1hash Sha1) string {
	// Do init if needed
	if db_location == "" {
		db_location := os.Getenv("DB_ENVIRONMENT")
		if db_location == "" {
			db_location = DEFAULT_DB_ENVIRONMENT
		}
	}

	hv := sha1_to_hex(sha1hash)
	return filepath.Join(db_location, hv[0:2], hv[2:])
}

func read_sha1_file(sha1hash Sha1) (buf []byte, filetype string) {

	// Open the object
	path := sha1_file_name(sha1hash)

	f, err := os.Open(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s\n", err)
		return nil, ""
	}

	// Decode it into memory. We read the whole thing at once for
	// convenience and then decode the header. Should we read some
	// bytes first and check the header before doing the rest? I
	// say no, because the header is only wrong for corrupt data,
	// which is not a common path.
	var b bytes.Buffer
	r, err := zlib.NewReader(f)
	io.Copy(&b, r)
	r.Close()
	f.Close()
	buf = b.Bytes()

	// The first few bytes contain the object type and size,
	// in the form <tag><space><ascii decimal size>\0, and
	// the rest of the data follows.

	// We could do this with a regexp, or by hand. The following code does
	// it by hand, but I'm thinking the right answer is to do it with a regex.
	// var fileTag = regexp.MustCompile(`(\S+) (\d+)\x00`)

	// Find the first space starting from the beginning of the file.
	// We fail if it's not found within 128 characters (tags aren't that long)
	e := len(buf)
	if e > 128 {
		e = 128
	}
	spacepos := bytes.IndexByte(buf[:e], byte(' '))
	if spacepos == -1 {
		return nil, "bad"
	}

	// Extract out the tag
	filetype = string(buf[0:spacepos])

	// The number runs from after the space to the zero byte
	zeropos := bytes.IndexByte(buf[spacepos+1:e], 0)
	if zeropos == -1 {
		return nil, "bad"
	}

	// Get the object size, which we don't actually need in the
	// Go code, because the Zlib reader will automatically make
	// its output object the right size. However, we can verify
	// with it
	objsize, err := strconv.Atoi(string(buf[spacepos+1 : zeropos]))
	if err != nil {
		return nil, "bad"
	} else if objsize != len(buf[zeropos+1:]) {
		fmt.Fprintf(os.Stderr, "Expected %d bytes, got %d bytes\n", objsize, len(buf[zeropos+1:]))
		return nil, "bad"
	}

	// Because this is a slice, this is actually efficient
	return buf[zeropos+1:], filetype
}

func write_sha1_file(buf []byte) error {

	// IMCOMPLETE
	var sha1hash Sha1

	if err := write_sha1_buffer(sha1hash, buf); err != nil {
		return err
	}
	fmt.Printf("%s\n", sha1_to_hex(sha1hash))
	return nil
}

func write_sha1_buffer(sha1hash Sha1, buf []byte) error {

	// Write the data to its content name in the object database
	path := sha1_file_name(sha1hash)
	f, err := os.Open(path)
	if err != nil {
		return err
	}

	// TBD we really should be checking the error on write
	f.Write(buf)
	f.Close()
	return nil
}

func show_error(msg string) error {
	fmt.Fprintf(os.Stderr, "error: %s\n", msg)
	return fmt.Errorf("fatal error")
}

// Validate the cache header (unpacked by caller)
func verify_hdr(h cache_header, buf []byte) error {

	if h.signature != CACHE_SIGNATURE {
		return show_error("bad signature")
	}
	if h.version != 1 {
		return show_error("bad version")
	}

	// SHA-1 hash everything except the signature
	// in the header, and compare against the signature
	sh := sha1.New()
	sh.Write(buf[:12])
	// Skip the sha1 hash, obviously
	sh.Write(buf[cache_header_bytes:])
	hash := sh.Sum(nil)
	var sha1hash Sha1
	copy(sha1hash[:], hash)
	if sha1hash != h.sha1hash {
		return show_error("bad header sha1")
	}
	return nil
}

func read_cache() error {
	if active_cache != nil {
		return show_error("more than one cachefile")
	}

	// Do init if needed
	if db_location == "" {
		db_location := os.Getenv("DB_ENVIRONMENT")
		if db_location == "" {
			db_location = DEFAULT_DB_ENVIRONMENT
		}
	}

	// Make sure we can access the cache directory
	var err error
	var info os.FileInfo
	if info, err = os.Stat(db_location); err != nil || (info.Mode().Perm()&0444) == 0 {
		return show_error("no access to SHA1 file directory")
	}

	// Get the index file and unpack it into memory
	var buf []byte
	if buf, err = load_raw_index(); err != nil {
		return err
	}

	// Decode the header and verify it
	h := unpack_cache_header(buf)

	if err = verify_hdr(h, buf); err != nil {
		return show_error("verify header failed")
	}

	// Decode the cache entries into our cache.
	// When we create a cache, give it room to grow
	var cacheCap int = (int(h.entries) + 16) * 3 / 2
	active_cache = make([]cache_entry, h.entries, cacheCap)

	offset := cache_header_bytes
	for i := 0; i < int(h.entries); i++ {
		entry, size := unpack_cache_entry(buf[offset:])
		active_cache = append(active_cache, entry)
		offset = offset + size
	}

	return nil
}

// unpack_cache_header unpacks a binary buffer into a cache_header
func unpack_cache_header(buf []byte) cache_header {
	var h cache_header
	h.signature = binary.LittleEndian.Uint32(buf[0:4])
	h.version = binary.LittleEndian.Uint32(buf[4:8])
	h.entries = binary.LittleEndian.Uint32(buf[8:12])
	copy(h.sha1hash[:], buf[12:32])
	return h
}

// pack_cache_header packs a cache_header into a binary buffer
func pack_cache_header(buf []byte, h cache_header) {
	binary.LittleEndian.PutUint32(buf[0:4], h.signature)
	binary.LittleEndian.PutUint32(buf[4:8], h.version)
	binary.LittleEndian.PutUint32(buf[8:12], h.entries)
	copy(buf[12:32], h.sha1hash[:])
}

// unpack_cache_entry unpacks a binary buffer into a cache_entry
// Because a packed cache_entry is a variable size, this returns
// the packed size, so the caller can iterate through the buffer
func unpack_cache_entry(buf []byte) (e cache_entry, size int) {
	e.ctime.sec = binary.LittleEndian.Uint32(buf[0:])
	e.ctime.nsec = binary.LittleEndian.Uint32(buf[4:])

	e.mtime.sec = binary.LittleEndian.Uint32(buf[8:])
	e.mtime.nsec = binary.LittleEndian.Uint32(buf[12:])

	e.st_dev = binary.LittleEndian.Uint32(buf[16:])
	e.st_ino = binary.LittleEndian.Uint32(buf[20:])
	e.st_mode = binary.LittleEndian.Uint32(buf[24:])
	e.st_uid = binary.LittleEndian.Uint32(buf[28:])
	e.st_gid = binary.LittleEndian.Uint32(buf[32:])
	e.st_size = binary.LittleEndian.Uint32(buf[36:])

	copy(e.sha1hash[:], buf[40:60])

	L := binary.LittleEndian.Uint16(buf[60:]) + 62
	e.name = string(buf[62:int(L)])

	// cache entries are rounded up to 8-byte boundaries
	// (we always need a zero, so this "rounds" 72 to 80, for example)
	size = (int(L) + 8) & ^7
	return e, size
}

// pack_cache_entry packs a cache_entry into a binary buffer
// As with unpacking, we return the packed size to the caller
func pack_cache_entry(buf []byte, e cache_entry) (size int) {
	binary.LittleEndian.PutUint32(buf[0:], e.ctime.sec)
	binary.LittleEndian.PutUint32(buf[4:], e.ctime.sec)

	binary.LittleEndian.PutUint32(buf[8:], e.ctime.sec)
	binary.LittleEndian.PutUint32(buf[12:], e.ctime.sec)

	binary.LittleEndian.PutUint32(buf[16:], e.st_dev)
	binary.LittleEndian.PutUint32(buf[20:], e.st_ino)
	binary.LittleEndian.PutUint32(buf[24:], e.st_mode)
	binary.LittleEndian.PutUint32(buf[28:], e.st_uid)
	binary.LittleEndian.PutUint32(buf[32:], e.st_gid)
	binary.LittleEndian.PutUint32(buf[36:], e.st_size)

	copy(buf[40:60], e.sha1hash[:])

	L := len(e.name)
	binary.LittleEndian.PutUint16(buf[60:], uint16(L))
	copy(buf[62:62+L], e.name)

	size = (62 + L + 8) & ^7
	return size
}

// TBD. Go strings are UTF-8. Git strings can have any encoding (although
// UTF-8 is the most likely). I need to resolve this.

// load_raw_index reads the contents of .dircache/index
// This needs to be unpacked and verified.
func load_raw_index() ([]byte, error) {
	var index *os.File
	var err error
	if index, err = os.Open(".dircache/index"); err != nil {
		if !os.IsNotExist(err) {
			return nil, show_error("open failed")
		}
		return nil, nil // it's not an error for the index to not exist
	}
	defer index.Close()

	// Read the entire index into memory
	var info os.FileInfo
	if info, err = index.Stat(); err != nil {
		return nil, show_error("access failed")
	}
	indexSize := info.Size()
	if indexSize < cache_header_bytes {
		return nil, show_error("header too short")
	}

	buf := make([]byte, indexSize)
	bytesread, err := index.Read(buf)
	if int64(bytesread) != indexSize {
		return nil, show_error("failed to read entire index")
	}

	return buf, nil
}
