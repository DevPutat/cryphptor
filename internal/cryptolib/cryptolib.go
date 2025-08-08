package cryptolib

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/sha256"
	"io"
	"os"

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

	hkdfKey := hkdf.New(sha256.New, e.key, nonce, []byte("cryphptor"))
	encryptKey := make([]byte, 32)
	if _, err := io.ReadFull(hkdfKey, encryptKey); err != nil {
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

	buf := make([]byte, 4096)
	for {
		n, err := inFile.Read(buf)
		if err != nil && err != io.EOF {
			return err
		}
		if n == 0 {
			break
		}

		encrypted := aesGCM.Seal(nil, nonce, buf[:n], nil)
		if _, err := outFile.Write(encrypted); err != nil {
			return err
		}
	}

	return nil
}
