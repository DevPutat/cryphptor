<?php
// test_decrypt.php - тестовый скрипт для проверки дешифрования через PHP расширение

// Проверяем, что расширение загружено
if (!extension_loaded('cryphptor')) {
    die("Расширение cryphptor не загружено\n");
}

echo "Расширение cryphptor загружено успешно\n";

// Пример использования функции дешифрования
// В реальной реализации эта функция будет вызываться автоматически
// при загрузке PHP файлов, но для теста мы вызовем её напрямую

$encrypted_data = file_get_contents('/var/www/html/test_encrypted.php');
$key = getenv('ENCRYPTION_KEY');

if (empty($key)) {
    die("Ключ шифрования не установлен в переменной окружения ENCRYPTION_KEY\n");
}

echo "Попытка дешифрования данных...\n";

// В реальной реализации здесь будет вызов функции из расширения
// decrypted_data = cryphptor_decrypt($encrypted_data, $key);
// Но пока используем заглушку
$decrypted_data = "Дешифрованные данные (заглушка)";

echo "Дешифрование завершено\n";
echo "Результат: $decrypted_data\n";
?>