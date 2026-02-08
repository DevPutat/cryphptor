#!/bin/bash
# Скрипт полной сборки расширения CRYPHPTOR

set -e  # Прервать при любой ошибке

LOG_FILE="build.log"

echo "=== Сборка CRYPHPTOR ===" | tee "$LOG_FILE"
echo "Лог сохраняется в: $LOG_FILE"
echo ""

# Шаг 1: Сборка builder'а
echo "[1/3] Сборка builder'а (make build)..." | tee -a "$LOG_FILE"
make build >> "$LOG_FILE" 2>&1
echo "✓ Builder собран" | tee -a "$LOG_FILE"
echo ""

# Шаг 2: Формирование файлов расширения
echo "[2/3] Формирование файлов расширения (./dist/phpext_builder)..." | tee -a "$LOG_FILE"
./dist/phpext_builder >> "$LOG_FILE" 2>&1
echo "✓ Файлы расширения сформированы" | tee -a "$LOG_FILE"
echo ""

# Шаг 3: Сборка Docker образа
echo "[3/3] Сборка Docker образа (make docker-84)..." | tee -a "$LOG_FILE"
make docker-84 >> "$LOG_FILE" 2>&1
echo "✓ Docker образ собран" | tee -a "$LOG_FILE"
echo ""

echo "=== Сборка завершена успешно ===" | tee -a "$LOG_FILE"
echo ""
echo "Последние 50 строк из лога:"
echo "---"
tail -n 50 "$LOG_FILE"
