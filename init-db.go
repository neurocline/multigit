// cat-file.go

package main

import (
	"fmt"
	"os"
	"path/filepath"
)

// -----------------------------------------------------------------------------------------------

func main() {
	// Create the .dircache
	// This returns an error if the .dircache already exists, because the user
	// shouldnt' do that.
	err := os.Mkdir(".dircache", 0700)
	if err != nil {
		fmt.Fprintf(os.Stderr, "unable to create .dircache: %s", err)
		os.Exit(1)
	}

	// If you want to, you can share the DB area with any number of branches.
	// That has advantages: you can save space by sharing all the SHA1 objects.
	// On the other hand, it might just make lookup slower and messier. You
	// be the judge.
	sha1_dir := os.Getenv("DB_ENVIRONMENT")
	if sha1_dir != "" {
		s, err := os.Stat(sha1_dir)
		if err == nil && s.IsDir() {
			return
		}
		fmt.Fprintf(os.Stderr, "DB_ENVIRONMENT set to bad directory %s: ", sha1_dir)
	}

	// The default case is to have a DB per managed directory.
	sha1_dir = DEFAULT_DB_ENVIRONMENT
	fmt.Fprintf(os.Stderr, "defaulting to private storage area\n")
	if err = os.Mkdir(sha1_dir, 0700); err != nil && !os.IsExist(err) {
		fmt.Fprintf(os.Stderr, "Can't create %s\n", sha1_dir)
		os.Exit(1)
	}

	// Create all the object subdirectories 00/, 01/, ... FF/
	for i := 0; i < 256; i++ {
		path := filepath.Join(sha1_dir, fmt.Sprintf("%02X", i))
		if err = os.Mkdir(path, 0700); err != nil && !os.IsExist(err) {
			fmt.Fprintf(os.Stderr, "Can't create %s\n", path)
			os.Exit(1)
		}
	}
}
