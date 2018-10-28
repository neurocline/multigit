// cat-file.go

package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/user"
	"strings"
	"time"
)

const ORIG_OFFSET = 40

// -----------------------------------------------------------------------------------------------

func main() {
	if len(os.Args) < 2 {
		usage("commit-tree <sha1> [-p <sha1>]* < changelog")
	}

	// Turn the hex string into a Sha1 value
	tree_sha1, err := get_sha1_hex(os.Args[1])
	if err != nil {
		usage("commit-tree <sha1> [-p <sha1>]* < changelog")
	}

	// Get any extra parents
	var parents [][20]byte
	for i := 2; i < len(os.Args); i = i + 2 {
		if os.Args[i] == "-p" {
			sha1, err := get_sha1_hex(os.Args[i+1])
			if err == nil {
				parents = append(parents, sha1)
				continue
			}
		}
		usage("commit-tree <sha1> [-p <sha1>]* < changelog")
	}

	if len(parents) == 0 {
		fmt.Fprintf(os.Stderr, "Committing initial tree %s\n", os.Args[1])
	}

	// For committer, we use machine name+email and current time
	// For author, we use env vars if they exist, otherwise author info
	// The idea is you might have a patch from someone else, but you always
	// commit as your local self.

	// Get login name and email to use for committer info
	var committer_name, committer_email string
	if committer_name, committer_email = getUserEmail(); committer_name == "" {
		usage("You don't exist. Go away!")
	}

	// Get current datetime in Git-desired format
	committer_date := time.Now().Format("Mon Jan _2 15:04:05 2006")

	// Override machine values with environment variables if they exist
	// Note that these environment variables are named backwards. I didn't do it.
	var author_name, author_email, author_date string
	if author_name = os.Getenv("COMITTER_NAME"); author_name == "" {
		author_name = committer_name
	}
	if author_email = os.Getenv("COMMITTER_EMAIL"); author_email == "" {
		author_email = committer_email
	}
	if author_date = os.Getenv("COMMITTER_DATE"); author_date == "" {
		author_date = committer_date
	}

	// Fix strings, removing garbage from OS
	committer_name = strings.Replace(committer_name, "[]\n", "", -1)
	committer_email = strings.Replace(committer_email, "[]\n", "", -1)
	committer_date = strings.Replace(committer_date, "[]\n", "", -1)

	author_name = strings.Replace(author_name, "[]\n", "", -1)
	author_email = strings.Replace(author_email, "[]\n", "", -1)
	author_date = strings.Replace(author_date, "[]\n", "", -1)

	// Create a new buffer to hold a tree, and create a blank
	// spacer that we will fill in with the object header once we
	// know the size of the tree object.
	var b bytes.Buffer
	fmt.Fprintf(&b, "%*s", ORIG_OFFSET, "")

	// Add first parent
	fmt.Fprintf(&b, "tree %s\n", sha1_to_hex(tree_sha1))

	// Add any remaining parents
	for _, parent := range parents {
		fmt.Fprintf(&b, "parent %s\n", sha1_to_hex(parent))
	}

	// Add author and committer info
	fmt.Fprintf(&b, "author %s <%s> %s\n", author_name, author_email, author_date)
	fmt.Fprintf(&b, "committer %s <%s> %s\n", committer_name, committer_email, committer_date)

	// Add the comment from stdin
	io.Copy(&b, os.Stdin)

	// Finalize by adding a header. Because the header is not fixed-length,
	// we need to skip past any pad bytes when we write it out.
	buf := b.Bytes()
	offset := finish_buffer("commit", buf)
	write_sha1_file(buf[offset:])
}

// finish_buffer finalizes the buffer by adding an object header.
// It returns the offset needed to skip past unneeded starting pad bytes
func finish_buffer(tag string, buf []byte) int {
	tagline := fmt.Sprintf("%s %d\x00", tag, len(buf[40:]))

	// copy it into the buffer so it aligns with the start of object data
	offset := ORIG_OFFSET - len(tagline)
	copy(buf[offset:], tagline)

	// This is the number of unused bytes at the beginning of the buffer
	return offset
}

// -----------------------------------------------------------------------------------------------

// Go doesn't really have this in the format that getpwuid
// provides, so create it from what we have available
func getUserEmail() (name, email string) {

	// Do our best to get a real username; if we can't, use
	// the login name
	var username, login string
	if me, err := user.Current(); err == nil {
		login = me.Username
		if username = me.Name; me.Name == "" {
			username = login
		}
	}

	// Get hostname. If we can't get a hostname, we are out of luck
	var err error
	var hostname string
	if hostname, err = os.Hostname(); err != nil {
		return "", ""
	}

	// On Windows, the username usually is prefixed by the hostname,
	// so strip it
	if len(username) > len(hostname) && username[0:len(hostname)] == hostname {
		username = username[len(hostname):]
		if username[0] == '\\' {
			username = username[1:]
		}
	}

	// Note that "user@host" is unlikely to be a valid email address
	// on Windows, but it's the best we can do
	return username, fmt.Sprintf("%s@%s", login, hostname)
}
