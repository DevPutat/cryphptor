#include "php.h"
#include "ext/standard/info.h"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <string.h>

// Функция дешифровки для PHP
PHP_FUNCTION(cryphptor_decrypt) {
  char *encrypted_data;
  size_t encrypted_data_len;
  char *key;
  size_t key_len;

  // Парсинг аргументов: зашифрованные данные и ключ
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &encrypted_data,
                            &encrypted_data_len, &key, &key_len) == FAILURE) {
    return;
  }

  // Проверка минимальной длины данных (nonce + tag)
  if (encrypted_data_len < 12 + 16) {
    php_error_docref(NULL, E_WARNING, "Encrypted data is too short");
    RETURN_FALSE;
  }

  // Извлечение nonce из первых 12 байт
  unsigned char nonce[12];
  memcpy(nonce, encrypted_data, 12);

  // Генерация ключа с использованием HKDF
  unsigned char decrypt_key[32];
  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
  if (!pctx) {
    php_error_docref(NULL, E_WARNING, "Failed to create HKDF context");
    RETURN_FALSE;
  }
  
  if (EVP_PKEY_derive_init(pctx) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to initialize HKDF");
    RETURN_FALSE;
  }
  
  if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to set HKDF hash");
    RETURN_FALSE;
  }
  
  if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, nonce, 12) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to set HKDF salt");
    RETURN_FALSE;
  }
  
  if (EVP_PKEY_CTX_set1_hkdf_key(pctx, (unsigned char *)key, key_len) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to set HKDF key");
    RETURN_FALSE;
  }
  
  const char *info = "cryphptor";
  if (EVP_PKEY_CTX_add1_hkdf_info(pctx, (unsigned char *)info, strlen(info)) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to set HKDF info");
    RETURN_FALSE;
  }
  
  size_t outlen = 32;
  if (EVP_PKEY_derive(pctx, decrypt_key, &outlen) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    php_error_docref(NULL, E_WARNING, "Failed to derive key");
    RETURN_FALSE;
  }
  
  EVP_PKEY_CTX_free(pctx);

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
  // Учитываем, что в зашифрованных данных первые 12 байт - это nonce, 
  // а последние 16 байт - это тег аутентификации
  unsigned char *plaintext = malloc(encrypted_data_len - 12 - 16);
  if (!plaintext) {
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to allocate memory");
    RETURN_FALSE;
  }
  
  int len;
  int ret = EVP_DecryptUpdate(ctx, plaintext, &len,
                        (unsigned char *)encrypted_data + 12,
                        encrypted_data_len - 12 - 16);
                        
  if (ret != 1) {
    free(plaintext);
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to decrypt data");
    RETURN_FALSE;
  }

  int plaintext_len = len;
  
  // Установка тега аутентификации
  if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (unsigned char *)encrypted_data + encrypted_data_len - 16) != 1) {
    free(plaintext);
    EVP_CIPHER_CTX_free(ctx);
    php_error_docref(NULL, E_WARNING, "Failed to set authentication tag");
    RETURN_FALSE;
  }
  
  ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
  if (ret != 1) {
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