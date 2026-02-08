package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/DevPutat/cryphptor/internal/cryptolib"
	"github.com/DevPutat/cryphptor/internal/scanner"
	"golang.org/x/sync/semaphore"
)

const COUNT_GORUTINE = 100

type arrayFlags []string

func (i *arrayFlags) String() string {
	return fmt.Sprintf("%v", *i)
}

func (i *arrayFlags) Set(value string) error {
	*i = append(*i, value)
	return nil
}

var exclude arrayFlags

// Проверяет, является ли файл PHP файлом для шифрования
func isPHPFile(path string) bool {
	lower := strings.ToLower(path)
	return strings.HasSuffix(lower, ".php") ||
		strings.HasSuffix(lower, ".inc") ||
		strings.HasSuffix(lower, ".phtml")
}

// Копирует файл без шифрования
func copyFile(src, dst string) error {
	srcFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer srcFile.Close()

	// Создаем директорию назначения если нужно
	dstDir := filepath.Dir(dst)
	if err := os.MkdirAll(dstDir, 0755); err != nil {
		return err
	}

	dstFile, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer dstFile.Close()

	_, err = io.Copy(dstFile, srcFile)
	return err
}

func main() {
	/*
	* ========FLAGS======== -
	* -key=encrypt-key-min-16-max-32
	* -root=./path/to/project/abs/or/rel
	* -dist=./path/to/dist/abs/or/rel
	* -exclude=dir-or-file-name
	* -exclude=/path/to/dir/or/file
	* -exclude=its-is-slice
	 */
	key := flag.String("key", "", "Encryption key")
	root := flag.String("root", "./", "Path to root of project")
	dist := flag.String("dist", "./dist", "Output directory for encrypted files")
	flag.Var(&exclude, "exclude", "Exclude paths: -exclude=dir1 -exclude=dir2")
	flag.Parse()

	conf := scanner.Config{RootDir: *root, Exclude: exclude}
	fmt.Println(*key, *dist)

	s := scanner.NewScanner(conf)
	list, err := s.Scan()
	if err != nil {
		fmt.Printf("ERROR: %v\n", err)
		os.Exit(1)
	}
	if len(*key) < 16 {
		fmt.Printf("ERROR: key is too short or empty: %s\n", *key)
		os.Exit(1)
	}
	var keyBytes []byte
	if len(*key) > 32 {
		keyBytes = []byte((*key)[:32]) // trim to 32 bytes if longer

	} else if len(*key) < 32 {
		keyBytes = make([]byte, 32)
		copy(keyBytes, []byte(*key))
	} else {
		keyBytes = []byte(*key)
	}

	*key = string(keyBytes)
	var wg sync.WaitGroup
	sem := semaphore.NewWeighted(COUNT_GORUTINE)
	ctx := context.Background()

	enc := cryptolib.NewSafeEncryptor([]byte(*key))
	for _, src := range list {
		relPath, err := filepath.Rel(*root, src)
		if err != nil {
			fmt.Printf("ERROR: failed to compute relative path for %s: %v\n", src, err)
			os.Exit(1)
		}
		dst := filepath.Join(*dist, relPath)

		wg.Add(1)
		go func(srcPath string) {
			defer wg.Done()
			if err := sem.Acquire(ctx, 1); err != nil {
				fmt.Printf("ERROR: failed to take gorutine %v\n", err)
				os.Exit(1)
			}
			defer sem.Release(1)

			// PHP файлы шифруем, остальные просто копируем
			if isPHPFile(srcPath) {
				if err := enc.Encrypt(srcPath, dst); err != nil {
					fmt.Printf("ERROR: failed to encrypt %s: %v\n", srcPath, err)
					os.Exit(1)
				}
			} else {
				if err := copyFile(srcPath, dst); err != nil {
					fmt.Printf("ERROR: failed to copy %s: %v\n", srcPath, err)
					os.Exit(1)
				}
			}
		}(src)
	}
	wg.Wait()
	fmt.Println("Deployment completed")
}

