# Инструкция по шифрованию Laravel-проекта с помощью Docker

В этом документе описывается, как зашифровать Laravel-проект с помощью инструмента cryphptor, используя многоэтапную сборку (multi-stage) в Docker. В результате получится зашифрованный образ, способный выполнять расшифровку кода в runtime.

## Подготовка

1. Убедитесь, что у вас установлены Docker и Docker Compose
2. Склонируйте репозиторий cryphptor:
   ```
   git clone https://github.com/DevPutat/cryphptor.git
   cd cryphptor
   ```

## Создание Dockerfile для шифрования

Создайте Dockerfile с использованием многоэтапной сборки:

```Dockerfile
# Этап 1: Шифрование файлов
FROM cryphptor-cryphptor as encryptor

# Копируем скрипт шифрования и исходники
COPY . /src
WORKDIR /src

# Создаем директорию для зашифрованного кода
RUN mkdir -p /encrypted

# Шифруем исходный код (все PHP файлы, включая vendor)
RUN /cryphptor -key="$ENCRYPTION_KEY" -root="/src" -dist="/encrypted"

# Этап 2: Копирование зашифрованного кода в рабочий образ
FROM cryphptor-phpfpm

# Устанавливаем переменную окружения для ключа расшифровки
ARG CRYPHPTOR_KEY
ENV DECRYPTION_KEY=$CRYPHPTOR_KEY

# Копируем зашифрованные файлы из этапа шифрования
COPY --from=encryptor /encrypted/ /var/www/html/

# Устанавливаем права доступа
RUN chown -R www-data:www-data /var/www/html/
```

## Динамическая версия с ARG параметрами

```Dockerfile
# Используем базовый образ cryphptor для шифрования
FROM cryphptor-cryphptor as encryptor

# Принимаем аргументы
ARG CRYPHPTOR_KEY
ENV ENCRYPTION_KEY=$CRYPHPTOR_KEY

# Копируем исходный код Laravel-приложения
COPY . /app/src
WORKDIR /app/src

# Создаем директорию для зашифрованного кода
RUN mkdir -p /app/encrypted

# Устанавливаем зависимости Laravel, если они еще не установлены
RUN composer install --no-dev --optimize-autoloader

# Шифруем все PHP файлы, включая vendor
RUN /cryphptor -key="$ENCRYPTION_KEY" -root="/app/src" -dist="/app/encrypted"

# Используем образ с PHP расширением cryphptor для runtime
FROM cryphptor-phpfpm

# Устанавливаем ключ расшифровки
ARG CRYPHPTOR_KEY
ENV DECRYPTION_KEY=$CRYPHPTOR_KEY

# Копируем зашифрованные файлы из этапа шифрования
COPY --from=encryptor /app/encrypted/ /var/www/html/

# Устанавливаем права доступа
RUN chown -R www-data:www-data /var/www/html/

# Убеждаемся, что включены необходимые модули (если нужно)
RUN docker-php-ext-enable cryphptor

# Устанавливаем рабочую директорию
WORKDIR /var/www/html/

EXPOSE 80
CMD ["php-fpm"]
```

## Альтернативный вариант с Docker Compose

Создайте файл `docker-compose.yml`:

```yaml
version: '3.8'

services:
  # Этап шифрования
  encryptor:
    build:
      context: .
      dockerfile: Dockerfile.encrypt
      args:
        - CRYPHPTOR_KEY=${CRYPHPTOR_KEY}
    volumes:
      - encrypted_code:/encrypted

  # Основной сервис приложения
  app:
    build:
      context: .
      dockerfile: Dockerfile.runtime
      args:
        - CRYPHPTOR_KEY=${CRYPHPTOR_KEY}
    depends_on:
      - encryptor
    volumes:
      - encrypted_code:/var/www/html:ro
    environment:
      - DECRYPTION_KEY=${CRYPHPTOR_KEY}
    ports:
      - "80:80"

volumes:
  encrypted_code:
```

Где `Dockerfile.encrypt`:

```Dockerfile
FROM cryphptor-cryphptor

ARG CRYPHPTOR_KEY
ENV ENCRYPTION_KEY=$CRYPHPTOR_KEY

WORKDIR /src
COPY . /src

RUN mkdir -p /encrypted
RUN /cryphptor -key="$ENCRYPTION_KEY" -root="/src" -dist="/encrypted"
```

А `Dockerfile.runtime`:

```Dockerfile
FROM cryphptor-phpfpm

ARG CRYPHPTOR_KEY
ENV DECRYPTION_KEY=$CRYPHPTOR_KEY

WORKDIR /var/www/html

# Остальные настройки
EXPOSE 80
CMD ["php-fpm"]
```

## Сборка и запуск

1. Установите переменную окружения:
   ```
   export CRYPHPTOR_KEY="ваш_ключ_шифрования"
   ```

2. Соберите образ:
   ```
   docker build -t laravel-encrypted-app --build-arg CRYPHPTOR_KEY="$CRYPHPTOR_KEY" .
   ```

3. Запустите контейнер:
   ```
   docker run -d --name laravel-app -e DECRYPTION_KEY="$CRYPHPTOR_KEY" -p 80:80 laravel-encrypted-app
   ```

## Запуск в продакшене

Для использования в продакшене:

1. Используйте безопасное хранение ключа (например, через Docker secrets или Kubernetes secrets)
2. Убедитесь, что переменная `DECRYPTION_KEY` доступна в контейнере
3. Проверьте совместимость версии PHP в образе с вашим Laravel-приложением
4. Протестируйте все компоненты: PHP-FPM, queue workers, schedule, и т.д.

## Важные замечания

- Обязательно используйте одинаковые ключи для шифрования и расшифровки
- Ключ должен быть длиной от 16 до 32 байт (он будет нормализован до 32 байт)
- Зашифрованные файлы останутся зашифрованными на диске
- PHP-расширение автоматически расшифрует файлы в памяти при их загрузке
- Все PHP-файлы, включая те, что в директории vendor, будут зашифрованы