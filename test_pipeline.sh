#!/bin/bash
# test_pipeline.sh - скрипт для тестирования полного конвейера шифрования/дешифрования

set -e  # Остановить выполнение при ошибке

echo "=== Тестирование полного конвейера cryphptor ==="

# Создаем директорию для теста
TEST_DIR="/tmp/cryphptor_test"
mkdir -p "$TEST_DIR"
cp test.php "$TEST_DIR/"

echo "1. Шифрование тестового файла..."
mkdir -p "$TEST_DIR/encrypted"
./dist/cryphptor -key="testencryptionkey12345" -root="$TEST_DIR" -dist="$TEST_DIR/encrypted"

echo "2. Проверка наличия зашифрованного файла..."
if [ -f "$TEST_DIR/encrypted/test.php" ]; then
    echo "   Зашифрованный файл создан успешно"
else
    echo "   ОШИБКА: Зашифрованный файл не найден"
    exit 1
fi

echo "3. Дешифрование файла с помощью внутренней библиотеки..."
# Используем внутреннюю библиотеку для дешифрования
mkdir -p "$TEST_DIR/decrypted"
./dist/cryphptor -key="testencryptionkey12345" -root="$TEST_DIR/encrypted" -dist="$TEST_DIR/decrypted"

echo "4. Сравнение исходного и дешифрованного файлов..."
# Сравниваем файлы
if cmp -s "$TEST_DIR/test.php" "$TEST_DIR/decrypted/test.php"; then
    echo "   Файлы совпадают - шифрование/дешифрование работает корректно"
else
    echo "   ОШИБКА: Файлы не совпадают"
    echo "   Исходный файл:"
    cat "$TEST_DIR/test.php"
    echo "   Дешифрованный файл:"
    cat "$TEST_DIR/decrypted/test.php"
    exit 1
fi

echo ""
echo "=== Тестирование завершено успешно ==="