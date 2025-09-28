<?php
// test_decrypt.php - тестовый скрипт для проверки дешифрования через PHP расширение

// Проверяем, что расширение загружено
if (!extension_loaded('cryphptor')) {
    die("Расширение cryphptor не загружено\n");
}

echo "Расширение cryphptor загружено успешно\n";

// Пример использования расширения для чтения зашифрованного файла
$encrypted_file = $argv[1] ?? '/tmp/encrypted_test.php'; // Путь к зашифрованному файлу как аргумент

if (!file_exists($encrypted_file)) {
    die("Файл зашифрованного файла не существует: $encrypted_file\n");
}

// Используем функцию расширения для получения пути с префиксом cryphptor://
$cryphptor_path = cryphptor_open_encrypted($encrypted_file);
echo "Cryphptor путь: $cryphptor_path\n";

// Читаем содержимое зашифрованного файла с автоматической дешифровкой
try {
    $content = file_get_contents($cryphptor_path);
    if ($content !== false) {
        echo "Содержимое зашифрованного файла успешно прочитано:\n";
        echo $content;
    } else {
        echo "Не удалось прочитать зашифрованный файл\n";
    }
} catch (Exception $e) {
    echo "Ошибка при чтении зашифрованного файла: " . $e->getMessage() . "\n";
}
?>