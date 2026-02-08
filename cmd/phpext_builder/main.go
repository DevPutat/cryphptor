package main

import (
	"fmt"
	"os"
	"path/filepath"
	"io"
)

func main() {
	projectDir, err := os.Getwd()
	if err != nil {
		fmt.Printf("Error getting current directory: %v\n", err)
		os.Exit(1)
	}

	// Define source and destination directories
	phpExtDir := filepath.Join(projectDir, "internal", "phpext")
	buildDir := filepath.Join(projectDir, "dist", "phpext")

	// Check if source directory exists
	if _, err := os.Stat(phpExtDir); os.IsNotExist(err) {
		fmt.Printf("Extension directory not found: %s\n", phpExtDir)
		os.Exit(1)
	}

	// Create build directory
	if err := os.MkdirAll(buildDir, 0755); err != nil {
		fmt.Printf("Error creating build directory: %v\n", err)
		os.Exit(1)
	}

	// Очистка старых файлов в dist/phpext
	fmt.Println("Cleaning old extension files...")
	entries, err := os.ReadDir(buildDir)
	if err != nil {
		fmt.Printf("Error reading build directory: %v\n", err)
		os.Exit(1)
	}
	for _, entry := range entries {
		if !entry.IsDir() {
			oldFile := filepath.Join(buildDir, entry.Name())
			if err := os.Remove(oldFile); err != nil {
				fmt.Printf("Warning: could not remove %s: %v\n", entry.Name(), err)
			} else {
				fmt.Printf("  Removed: %s\n", entry.Name())
			}
		}
	}

	// List of files to copy from internal/phpext to dist/phpext
	files := []string{
		"cryphptor_memory.c", // Версия с дешифровкой в памяти (zend_compile_file hook)
		"config.m4",
		"php_cryphptor.h",
	}

	fmt.Printf("Building PHP extension from %s to %s\n", phpExtDir, buildDir)

	// Copy all files
	for _, filename := range files {
		src := filepath.Join(phpExtDir, filename)
		dst := filepath.Join(buildDir, filename)

		if err := copyFile(src, dst); err != nil {
			fmt.Printf("Error copying %s: %v\n", filename, err)
			os.Exit(1)
		}
		fmt.Printf("✓ Copied: %s\n", filename)
	}

	fmt.Printf("\n✅ PHP extension successfully prepared in: %s\n", buildDir)
	fmt.Println("\nTo build the extension, run in the build directory:")
	fmt.Println("  phpize")
	fmt.Println("  ./configure --enable-cryphptor")
	fmt.Println("  make")
	fmt.Println("  make install")
}

func copyFile(src, dst string) error {
	srcFile, err := os.Open(src)
	if err != nil {
		return fmt.Errorf("failed to open source file %s: %w", src, err)
	}
	defer srcFile.Close()

	dstFile, err := os.Create(dst)
	if err != nil {
		return fmt.Errorf("failed to create destination file %s: %w", dst, err)
	}
	defer dstFile.Close()

	_, err = io.Copy(dstFile, srcFile)
	if err != nil {
		return fmt.Errorf("failed to copy file: %w", err)
	}

	return nil
}