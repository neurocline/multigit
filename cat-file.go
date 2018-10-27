// cat-file.go

package main

import (
    "fmt"
    "io/ioutil"
    "os"
)

// -----------------------------------------------------------------------------------------------

func main() {
    // cat-file requires one argument, the SHA1 hash of the object to print
    if len(os.Args) != 2 {
        usage("cat-file: cat-file <sha1>")
    }

    // Turn the hex string into a Sha1 value
    sha1, err := get_sha1_hex(os.Args[1])
    if err != nil {
        usage("cat-file: cat-file <sha1>")
    }

    // test
    s := sha1_to_hex(sha1)
    fmt.Printf("Saw %s, got %s\n", os.Args[1], s)

    // Read the content-named object from the object database
    buf, filetype := read_sha1_file(sha1)
    if buf == nil {
        os.Exit(1)
    }

    // Write the object contents to a tempfile
    tempfile, err := ioutil.TempFile(".", "temp_git_file_")
    if err != nil {
        usage("unable to create tempfile");
    }
    defer tempfile.Close()
    tempname := tempfile.Name()

    written, err := tempfile.Write(buf)
    if err != nil || written != len(buf) {
        filetype = "bad"
    }

    // Output tempfile name and type of object
    fmt.Printf("%s: %s\n", tempname, filetype)
}
