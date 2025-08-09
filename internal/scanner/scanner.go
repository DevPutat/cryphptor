package scanner

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
)

type Scanner struct {
	rootDir  string
	exclude  []string
	fileList []string
	mutex    sync.Mutex
}

type Config struct {
	RootDir string
	Exclude []string
}

func NewScanner(cfg Config) *Scanner {
	return &Scanner{
		rootDir:  cfg.RootDir,
		exclude:  cfg.Exclude,
		fileList: make([]string, 0),
	}
}

func (s *Scanner) Scan() ([]string, error) {
	if _, err := os.Stat(s.rootDir); os.IsNotExist(err) {
		return nil, err
	}

	var wg sync.WaitGroup

	err := filepath.Walk(s.rootDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if s.isExclude(path) {
			if info.IsDir() {
				return filepath.SkipDir
			}
			return nil
		}

		if !info.IsDir() && strings.HasSuffix(strings.ToLower(info.Name()), ".php") {
			wg.Add(1)
			go func(p string) {
				defer wg.Done()
				s.mutex.Lock()
				s.fileList = append(s.fileList, p)
				s.mutex.Unlock()
			}(path)
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	wg.Wait()
	return s.fileList, nil
}

func (s *Scanner) isExclude(path string) bool {
	relPath, err := filepath.Rel(s.rootDir, path)
	if err != nil {
		return false
	}
	for _, exc := range s.exclude {
		if strings.HasPrefix(relPath, exc) || path == exc {
			return true
		}
	}
	return false
}
