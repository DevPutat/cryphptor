<?php
// test_runtime_decryption.php - тест runtime расшифровки

if (!extension_loaded('cryphptor')) {
    die("Расширение cryphptor не загружено\n");
}

echo "Расширение cryphptor загружено\n";

// Создаем зашифрованные файлы с правильными именами
$encryptedMain = __DIR__ . '/test_runtime/encrypted/main.php';
$encryptedUtils = __DIR__ . '/test_runtime/encrypted/utils.php';

// Проверяем, что файлы существуют
if (!file_exists($encryptedMain) || !file_exists($encryptedUtils)) {
    die("Зашифрованные файлы не найдены\n");
}

echo "Файлы существуют, пытаемся выполнить их с помощью stream wrapper\n";

// Используем stream wrapper для чтения зашифрованного файла
$cryphptorMain = 'cryphptor://' . $encryptedMain;

// Читаем содержимое основного файла и выполняем его
$content = file_get_contents($cryphptorMain);
if ($content === false) {
    die("Не удалось прочитать зашифрованный файл\n");
}

echo "Содержимое основного файла прочитано, длина: " . strlen($content) . "\n";

// Создаем временный файл с расшифрованным содержимым для выполнения
$tempFile = tempnam(sys_get_temp_dir(), 'cryphptor_test_');
file_put_contents($tempFile, $content);

// Загружаем и выполняем временный файл
include $tempFile;

// Удаляем временный файл
unlink($tempFile);

echo "Тест выполнения зашифрованных файлов завершен\n";
?>