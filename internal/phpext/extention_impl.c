#include "php.h"
#include "ext/standard/info.h"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <string.h>

// Функция для открытия зашифрованного файла с помощью stream wrapper
PHP_FUNCTION(cryphptor_open_encrypted) {
  char *filename;
  size_t filename_len;

  // Парсинг аргументов: путь к зашифрованному файлу
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
    RETURN_FALSE;
  }

  // Создаем путь с префиксом cryphptor://
  char *cryphptor_path = emalloc(filename_len + 13); // 12 для "cryphptor://" + 1 для '\0'
  strcpy(cryphptor_path, "cryphptor://");
  strcat(cryphptor_path, filename);

  // Возвращаем путь, который может быть использован с file_get_contents и т.п.
  RETVAL_STRING(cryphptor_path);
  efree(cryphptor_path);
}