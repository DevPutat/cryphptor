#!/usr/bin/env php
<?php
/**
 * XOR Encryption Script for PHP Files
 * 
 * Usage: php encrypt.php <key> <directory>
 * 
 * This script encrypts all PHP files in the given directory using XOR encryption.
 * The encrypted files can be decrypted at runtime by the xor_decrypt PHP extension
 * or the xor_autoloader.php wrapper.
 */

if ($argc < 3) {
    echo "XOR PHP File Encryption\n";
    echo "========================\n\n";
    echo "Usage: php encrypt.php <key> <directory>\n\n";
    echo "Arguments:\n";
    echo "  key       - The XOR encryption key (use a strong, unique key)\n";
    echo "  directory - The directory containing PHP files to encrypt\n\n";
    echo "Example:\n";
    echo "  php encrypt.php \"my_super_secret_key_123\" /var/www/laravel\n\n";
    echo "Warning: This will modify all PHP files in place. Make sure to backup first!\n";
    exit(1);
}

$key = $argv[1];
$directory = rtrim($argv[2], '/');

if (!is_dir($directory)) {
    echo "Error: Directory '$directory' does not exist.\n";
    exit(1);
}

if (empty($key)) {
    echo "Error: Encryption key cannot be empty.\n";
    exit(1);
}

if (strlen($key) < 8) {
    echo "Warning: Key length is less than 8 characters. This is not secure.\n";
    echo "Consider using a longer key.\n\n";
}

/**
 * XOR encrypt/decrypt data (symmetric operation)
 */
function xorCrypt(string $data, string $key): string
{
    $result = '';
    $keyLength = strlen($key);
    $dataLength = strlen($data);
    
    for ($i = 0; $i < $dataLength; $i++) {
        $result .= $data[$i] ^ $key[$i % $keyLength];
    }
    
    return $result;
}

/**
 * Find all PHP files in directory recursively
 */
function findPhpFiles(string $directory): array
{
    $files = [];
    $excludePatterns = [
        '/vendor/',
        '/node_modules/',
        '/.git/',
        '/encrypt.php',
        '/xor_autoloader.php',
    ];
    
    $iterator = new RecursiveDirectoryIterator($directory);
    $recursiveIterator = new RecursiveIteratorIterator($iterator);
    
    foreach ($recursiveIterator as $file) {
        if ($file->isFile() && $file->getExtension() === 'php') {
            $filePath = $file->getPathname();
            
            /* Skip excluded patterns */
            $skip = false;
            foreach ($excludePatterns as $pattern) {
                if (strpos($filePath, $pattern) !== false) {
                    $skip = true;
                    break;
                }
            }
            
            if (!$skip) {
                $files[] = $filePath;
            }
        }
    }
    
    return $files;
}

/**
 * Encrypt a single PHP file
 */
function encryptFile(string $filePath, string $key): bool
{
    $content = file_get_contents($filePath);
    
    if ($content === false) {
        echo "  Failed to read: $filePath\n";
        return false;
    }
    
    /* Check if already encrypted (simple heuristic) */
    if (strpos($content, '<?php') !== 0 && strpos($content, '#!/') !== 0) {
        echo "  Skipping (not a valid PHP file): $filePath\n";
        return false;
    }
    
    $encrypted = xorCrypt($content, $key);
    
    $result = file_put_contents($filePath, $encrypted);
    
    if ($result === false) {
        echo "  Failed to write: $filePath\n";
        return false;
    }
    
    return true;
}

/* Main execution */
echo "XOR PHP File Encryption\n";
echo "========================\n";
echo "Key length: " . strlen($key) . " characters\n";
echo "Directory: $directory\n\n";

/* Confirm before proceeding */
echo "This will encrypt all PHP files in the directory.\n";
echo "Make sure you have a backup!\n\n";
echo "Type 'YES' to confirm: ";
$handle = fopen("php://stdin", "r");
$line = trim(fgets($handle));
fclose($handle);

if ($line !== 'YES') {
    echo "Aborted.\n";
    exit(0);
}

echo "\n";

$phpFiles = findPhpFiles($directory);

if (empty($phpFiles)) {
    echo "No PHP files found in '$directory'.\n";
    exit(0);
}

echo "Found " . count($phpFiles) . " PHP file(s) to encrypt\n\n";

$successCount = 0;
$failCount = 0;
$skipCount = 0;

foreach ($phpFiles as $file) {
    echo "Processing: $file\n";
    
    if (encryptFile($file, $key)) {
        echo "  ✓ Encrypted\n";
        $successCount++;
    } else {
        $failCount++;
    }
}

echo "\n========================\n";
echo "Summary:\n";
echo "  Encrypted: $successCount\n";
echo "  Failed/Skipped: $failCount\n";
echo "\nDone! All PHP files have been encrypted.\n\n";
echo "To run the application:\n";
echo "1. Set XOR_DECRYPT_KEY environment variable to your key\n";
echo "2. Include xor_autoloader.php before loading encrypted files\n";
echo "3. Use xor_require() or include 'xor://' stream wrapper\n";
echo "\nExample:\n";
echo "  XOR_DECRYPT_KEY=\"$key\" php -r \"require 'xor_autoloader.php'; xor_require('index.php');\"\n";
