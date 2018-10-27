// +build mage

package main

import (
	"github.com/magefile/mage/sh"
)

func Build() error {
	// Build cat-file: cat-file.go read-cache.go
	err := sh.Run("go", "build", "cat-file.go", "read-cache.go")
	if err != nil {
		return err
	}

	// Build init-db: init-db.go read-cache.go
	err = sh.Run("go", "build", "init-db.go", "read-cache.go")
	if err != nil {
		return err
	}

	return nil
}
