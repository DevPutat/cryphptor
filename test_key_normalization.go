package main

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/DevPutat/cryphptor/internal/cryptolib"
)

func main() {
	// Create temporary directory for test
	tempDir := os.TempDir()
	originalFile := filepath.Join(tempDir, "test_original.php")
	encryptedFile := filepath.Join(tempDir, "test_encrypted.php")
	decryptedFile := filepath.Join(tempDir, "test_decrypted.php")

	// Original PHP content
	originalContent := `<?php
// test.php - тестовый файл для проверки шифрования/дешифрования

echo "Hello, World! This is a test PHP file.\n";

// Простая функция для тестирования
function testFunction() {
    return "This is a test function output.";
}

echo testFunction() . "\n";
?>`

	// Write original file
	err := os.WriteFile(originalFile, []byte(originalContent), 0644)
	if err != nil {
		fmt.Printf("Error writing original file: %v\n", err)
		return
	}

	// Key - exactly as used in test_pipeline.sh
	key := "testencryptionkey12345" // 25 bytes
	
	// Normalize key to 32 bytes (same as CLI does now)
	if len(key) > 32 {
		key = key[:32]
	} else if len(key) < 32 {
		// Pad with zeros
		paddedKey := make([]byte, 32)
		copy(paddedKey, []byte(key))
		key = string(paddedKey)
	}

	fmt.Printf("Using key of length: %d\n", len(key))

	encryptor := cryptolib.NewSafeEncryptor([]byte(key))

	// Encrypt
	fmt.Println("Encrypting...")
	err = encryptor.Encrypt(originalFile, encryptedFile)
	if err != nil {
		fmt.Printf("Error encrypting: %v\n", err)
		return
	}

	// Decrypt
	fmt.Println("Decrypting...")
	err = encryptor.Decrypt(encryptedFile, decryptedFile)
	if err != nil {
		fmt.Printf("Error decrypting: %v\n", err)
		return
	}

	// Read decrypted content
	decryptedContent, err := os.ReadFile(decryptedFile)
	if err != nil {
		fmt.Printf("Error reading decrypted file: %v\n", err)
		return
	}

	// Compare contents
	if string(decryptedContent) == originalContent {
		fmt.Println("SUCCESS: Original and decrypted content match!")
	} else {
		fmt.Println("FAILURE: Original and decrypted content do NOT match!")
		fmt.Printf("Original length: %d\n", len(originalContent))
		fmt.Printf("Decrypted length: %d\n", len(string(decryptedContent)))
	}

	// Cleanup
	os.Remove(originalFile)
	os.Remove(encryptedFile)
	os.Remove(decryptedFile)
}