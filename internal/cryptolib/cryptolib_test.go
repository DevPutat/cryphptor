package cryptolib

import (
	"bytes"
	"crypto/rand"
	"io"
	"os"
	"path/filepath"
	"testing"

	"slices"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestEncrypt(t *testing.T) {
	tempDir := t.TempDir()

	originalFile := filepath.Join(tempDir, "original.php")
	encryptedFile := filepath.Join(tempDir, "encrypted.php")
	decryptedFile := filepath.Join(tempDir, "decrypted.php")

	originalData := []byte("Test data for my test")
	err := os.WriteFile(originalFile, originalData, 0644)
	require.NoError(t, err)

	key := []byte("this-is-a-32-byte-secret-key-for-test")

	encryptor := NewSafeEncryptor(key)

	t.Run("encrypt file", func(t *testing.T) {
		err := encryptor.Encrypt(originalFile, encryptedFile)
		require.NoError(t, err)

		encryptedData, err := os.ReadFile(encryptedFile)
		assert.Greater(t, len(encryptedData), 12, "encrypted file should be larger than nonce")
	})

	t.Run("decrypt file", func(t *testing.T) {
		err := encryptor.Decrypt(encryptedFile, decryptedFile)
		require.NoError(t, err)

		decryptedData, err := os.ReadFile(decryptedFile)
		require.NoError(t, err)
		assert.Equal(t, originalData, decryptedData, "decrypted no equal original")
	})

	t.Run("tampered data should fail to decrypt", func(t *testing.T) {
		tamperedFile := filepath.Join(tempDir, "tampered.php")
		tamperedDecryptedFile := filepath.Join(tempDir, "tamperedDecrypted.php")

		encryptedData, err := os.ReadFile(encryptedFile)
		require.NoError(t, err)

		tamperedData := slices.Clone(encryptedData)
		tamperedData[12] ^= 0xFF

		err = os.WriteFile(tamperedFile, tamperedData, 0644)
		require.NoError(t, err)

		err = encryptor.Decrypt(tamperedFile, tamperedDecryptedFile)
		assert.Error(t, err, "decrypt tampered should fail")
	})

	t.Run("decrypt with wrong key should fail", func(t *testing.T) {
		wrongDecryptedFile := filepath.Join(tempDir, "wrongDecrypted.php")

		wrongKey := []byte("wrong-key-32-bytes-for-testing!")
		wrongEncryptor := NewSafeEncryptor(wrongKey)

		err := wrongEncryptor.Decrypt(encryptedFile, wrongDecryptedFile)
		assert.Error(t, err, "decrypt with wrong key should fail")
	})
}

func TestEncryptor_EncryptDecrypt_StreamEquivalence(t *testing.T) {
	// Проверка на больших данных (чтобы убедиться, что потоковая обработка работает)
	tempDir := t.TempDir()

	src := filepath.Join(tempDir, "large.php")
	dstEnc := filepath.Join(tempDir, "large_enc.php")
	dstDec := filepath.Join(tempDir, "large_dec.php")

	// Генерируем 100 КБ данных
	data := make([]byte, 100*1024)
	_, err := io.ReadFull(rand.Reader, data)
	require.NoError(t, err)

	err = os.WriteFile(src, data, 0644)
	require.NoError(t, err)

	key := []byte("test-key-for-streaming-encryption")
	encryptor := NewSafeEncryptor(key)

	// Шифруем
	err = encryptor.Encrypt(src, dstEnc)
	require.NoError(t, err)

	// Расшифровываем
	err = encryptor.Decrypt(dstEnc, dstDec)
	require.NoError(t, err)

	// Сравниваем
	decrypted, err := os.ReadFile(dstDec)
	require.NoError(t, err)
	assert.True(t, bytes.Equal(data, decrypted), "decrypted data must match original")
}
