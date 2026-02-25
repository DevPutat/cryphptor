<?php
echo "Тест прозрачной расшифровки\n";
echo "=========================\n\n";

/* Создаём тестовый PHP файл */
$testCode = '<?php
function hello() {
    return "Привет из зашифрованного файла!";
}
echo hello() . PHP_EOL;
';

echo "1. Создаём тестовый PHP файл:\n";
file_put_contents('/tmp/test_transparent.php', $testCode);
echo "Файл создан: /tmp/test_transparent.php\n\n";

/* Шифруем вручную (XOR) */
$key = "test_key_123";
$encrypted = '';
$keyLen = strlen($key);
for ($i = 0; $i < strlen($testCode); $i++) {
    $encrypted .= $testCode[$i] ^ $key[$i % $keyLen];
}

file_put_contents('/tmp/test_encrypted.php', $encrypted);
echo "2. Зашифровали файл ключом: $key\n";
echo "Содержимое (hex): " . bin2hex($encrypted) . "\n\n";

/* Теперь пробуем включить зашифрованный файл */
echo "3. Включаем зашифрованный файл через require:\n";
require '/tmp/test_encrypted.php';

echo "\nГотово!\n";
