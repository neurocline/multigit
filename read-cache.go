// read-cache.go

package main

import (
    "bytes"
    "compress/zlib"
    _ "crypto/sha1"
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
type cache_header struct {
    signature uint32
    version uint32
    entries uint32
    sha1 Sha1
}

type cache_time struct {
    sec uint32
    nsec uint32
}

type cache_entry struct {
    ctime cache_time
    mtime cache_time
    st_dev uint32
    st_ino uint32
    st_mode uint32
    st_uid uint32
    st_gid uint32
    st_size uint32
    sha1 Sha1
    name string
}

var active_cache []cache_entry

var DEFAULT_DB_ENVIRONMENT string = ".dircache/objects"

// -----------------------------------------------------------------------------------------------

func usage(msg string) {
    fmt.Fprintf(os.Stderr, "read-tree: %s\n", msg)
    os.Exit(1)
}

func sha1_to_hex(sha1 Sha1) string {
    return hex.EncodeToString(sha1[:])
}

func get_sha1_hex(hexvalue string) (sha1 Sha1, err error) {
    var hashvalue []byte
    hashvalue, err = hex.DecodeString(hexvalue)
    if err != nil {
        return sha1, fmt.Errorf("malformed hex")
    }

    copy(sha1[:], hashvalue)
    return sha1, nil
}

// Turn a Sha1 into a path in the object database. This assumes
// that objects are binned into sub-directories 00/, 01/, ..FF/
// and the name inside the sub-directory is missing the first octet.
func sha1_file_name(sha1 Sha1) string {
    db := os.Getenv("DB_ENVIRONMENT")
    if db == "" {
        db = DEFAULT_DB_ENVIRONMENT
    }

    hv := sha1_to_hex(sha1)
    return filepath.Join(db, hv[0:2], hv[2:])
}

func read_sha1_file(sha1 Sha1) (buf []byte, filetype string) {

    // Open the object
    path := sha1_file_name(sha1)

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
    objsize, err := strconv.Atoi(string(buf[spacepos+1:zeropos]))
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
    return fmt.Errorf("Not implemented yet")
}

func write_sha1_buffer(sha1 Sha1, buf []byte) error {
    return fmt.Errorf("Not implemented yet")
}

func show_error(err string) error {
    fmt.Fprintf(os.Stderr, "error: %s\n", err)
    return fmt.Errorf("fatal error")
}

func verify_hdr(h *cache_header, size int) error {
    return fmt.Errorf("Not implemented yet")
}

func read_cache() error {
    return fmt.Errorf("Not implemented yet")
}
