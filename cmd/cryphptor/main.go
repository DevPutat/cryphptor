package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
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
		go func() {
			defer wg.Done()
			if err := sem.Acquire(ctx, 1); err != nil {
				fmt.Printf("ERROR: failed to take gorutine %v\n", err)
				os.Exit(1)
			}
			defer sem.Release(1)
			if err := enc.Encrypt(src, dst); err != nil {
				fmt.Printf("ERROR: failed to encrypt %s: %v\n", src, err)
			}
		}()
	}
	wg.Wait()
	fmt.Println("Deployment completed")
}
