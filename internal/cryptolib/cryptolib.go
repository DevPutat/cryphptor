package cryptolib

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/sha256"
	"io"
	"os"
	"path/filepath"

	"golang.org/x/crypto/hkdf"
)

type Encryptor struct {
	key []byte
}

func NewSafeEncryptor(key []byte) *Encryptor {
	return &Encryptor{key}
}

func (e *Encryptor) Encrypt(src string, dst string) error {
	inFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer inFile.Close()

	outFile, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer outFile.Close()

	nonce := make([]byte, 12)
	if _, err := io.ReadFull(rand.Reader, nonce); err != nil {
		return err
	}

	hkdfReader := hkdf.New(sha256.New, e.key, nonce, []byte("cryphptor"))
	encryptKey := make([]byte, 32)
	if _, err := io.ReadFull(hkdfReader, encryptKey); err != nil {
		return err
	}

	block, err := aes.NewCipher(encryptKey)
	if err != nil {
		return err
	}

	aesGCM, err := cipher.NewGCM(block)
	if err != nil {
		return err
	}

	if _, err := outFile.Write(nonce); err != nil {
		return err
	}

	// Читаем весь файл в память
	fileData, err := io.ReadAll(inFile)
	if err != nil {
		return err
	}

	// Шифруем все данные целиком с использованием одного nonce
	encrypted := aesGCM.Seal(nil, nonce, fileData, nil)
	
	if _, err := outFile.Write(encrypted); err != nil {
		return err
	}

	return nil
}

func (e *Encryptor) Decrypt(src string, dst string) error {
	inFile, err := os.Open(src)
	if err != nil {
		return err
	}

	defer inFile.Close()

	if err := os.MkdirAll(filepath.Dir(dst), 0755); err != nil {
		return err
	}

	outFile, err := os.Create(dst)
	if err != nil {
		return err
	}

	defer outFile.Close()

	nonce := make([]byte, 12)
	if _, err := io.ReadFull(inFile, nonce); err != nil {
		return err
	}

	hkdfReader := hkdf.New(sha256.New, e.key, nonce, []byte("cryphptor"))
	decryptKey := make([]byte, 32)
	if _, err := io.ReadFull(hkdfReader, decryptKey); err != nil {
		return err
	}

	block, err := aes.NewCipher(decryptKey)
	if err != nil {
		return err
	}
	aesGCM, err := cipher.NewGCM(block)
	if err != nil {
		return err
	}
	
	// Читаем все зашифрованные данные
	encryptedData, err := io.ReadAll(inFile)
	if err != nil {
		return err
	}

	// Расшифровываем все данные целиком
	decrypted, err := aesGCM.Open(nil, nonce, encryptedData, nil)
	if err != nil {
		return err
	}
	
	if _, err := outFile.Write(decrypted); err != nil {
		return err
	}
	
	return nil
}
