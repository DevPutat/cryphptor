#include "php.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

// Функция дешифровки для PHP
PHP_FUNCTION(laradecrypt) {
  char *encrypted_data;
  size_t encrypted_data_len;
  char *key;
  size_t key_len;

  // Парсинг аргументов: зашифрованные данные и ключ
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &encrypted_data,
                            &encrypted_data_len, &key, &key_len) == FAILURE) {
    return;
  }

  // Извлечение nonce из первых 12 байт
  unsigned char nonce[12];
  memcpy(nonce, encrypted_data, 12);

  // Генерация ключа с использованием HKDF
  unsigned char decrypt_key[32];
  HKDF(decrypt_key, 32, (unsigned char *)key, key_len, nonce, 12,
       (unsigned char *)"larashield", 10);

  // Инициализация контекста OpenSSL
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    php_error_docref(NULL, E_WARNING, "Failed to create EVP context");
    RETURN_FALSE;
  }

  // Настройка AES-256-GCM для дешифровки
  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to initialize decryption");
    RETURN_FALSE;
  }

  if (EVP_DecryptInit_ex(ctx, NULL, NULL, decrypt_key, nonce) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to set key and nonce");
    RETURN_FALSE;
  }

  // Выделение памяти под расшифрованные данные
  unsigned char *plaintext = malloc(encrypted_data_len - 12);
  int len;
  if (EVP_DecryptUpdate(ctx, plaintext, &len,
                        (unsigned char *)encrypted_data + 12,
                        encrypted_data_len - 12) != 1) {
    free(plaintext);
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to decrypt data");
    RETURN_FALSE;
  }

  int plaintext_len = len;
  if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
    free(plaintext);
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to finalize decryption");
    RETURN_FALSE;
  }
  plaintext_len += len;

  // Очистка и возврат результата
  EVP_CIPHER_CTX_free(ctx);
  RETVAL_STRINGL((char *)plaintext, plaintext_len);
  free(plaintext);
}
