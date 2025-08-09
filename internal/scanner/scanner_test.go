package scanner

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestScanner(t *testing.T) {
	curTime := time.Now().Format("200601021504")
	tmpDir := filepath.Join(os.TempDir(), fmt.Sprintf("%s-project-test", curTime))

	err := setupTestDir(tmpDir)
	assert.NoError(t, err, "error create test files and dir")
	defer func() {
		err := os.RemoveAll(tmpDir)
		assert.NoError(t, err, "error remove test files and dir")
	}()
	cfg := Config{
		RootDir: tmpDir,
		Exclude: []string{"excluded"},
	}

	sc := NewScanner(cfg)
	files, err := sc.Scan()
	assert.NoError(t, err, "error on scan")
	assert.Equal(t, len(files), 2, "wrong count files")
}

func setupTestDir(tmpDir string) error {
	if err := os.Mkdir(tmpDir, 0755); err != nil {
		return err
	}
	if err := os.WriteFile(filepath.Join(tmpDir, "test.php"), []byte("<?php echo 'test';"), 0644); err != nil {
		return err
	}

	// Создаем поддиректорию included и test.php в ней
	includedDir := filepath.Join(tmpDir, "included")
	if err := os.MkdirAll(includedDir, 0755); err != nil {
		return err
	}
	if err := os.WriteFile(filepath.Join(includedDir, "test.php"), []byte("<?php echo 'included';"), 0644); err != nil {
		return err
	}

	// Создаем поддиректорию excluded и test.php в ней
	excludedDir := filepath.Join(tmpDir, "excluded")
	if err := os.MkdirAll(excludedDir, 0755); err != nil {
		return err
	}
	if err := os.WriteFile(filepath.Join(excludedDir, "test.php"), []byte("<?php echo 'excluded';"), 0644); err != nil {
		return err
	}
	return nil
}
