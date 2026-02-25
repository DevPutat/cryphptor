<?php
require "/usr/local/share/xor-decrypt/xor_autoloader.php";

/* Create a test PHP file content */
$testContent = "<?php\n\$message = \"Hello from encrypted file!\";\necho \$message . PHP_EOL;\n";

/* Encrypt it */
$key = "test_key_123";
$encrypted = xor_encrypt_string($testContent, $key);

/* Save to temp file */
file_put_contents("/tmp/test_enc.php", $encrypted);
echo "Created encrypted file\n";

/* Now try to include it using the stream wrapper */
register_xor_wrapper();
echo "Including encrypted file:\n";
include "xor:///tmp/test_enc.php";
