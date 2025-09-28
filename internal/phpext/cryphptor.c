#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define CRYPTHPTOR_MIN_SIZE 28 // 12 (nonce) + 16 (tag)

// Структура для данных потока
typedef struct {
    unsigned char *decrypted_data;
    size_t decrypted_size;
    size_t pos;
} cryphptor_stream_data;

// Функция для дешифровки
static int cryphptor_decrypt_data(const char *ciphertext, size_t ciphertext_len, unsigned char **plaintext, size_t *plaintext_len TSRMLS_DC) {
    if (ciphertext_len < CRYPTHPTOR_MIN_SIZE) {
        return 0; // Слишком маленький размер для дешифровки
    }

    // Извлекаем компоненты
    const unsigned char *nonce = (const unsigned char *)ciphertext;                  // 12 байт
    const unsigned char *ciphertext_and_tag = (const unsigned char *)(ciphertext + 12); // Остальное
    size_t ct_len = ciphertext_len - 12;
    const unsigned char *tag = ciphertext_and_tag + (ct_len - 16);
    const unsigned char *ct = ciphertext_and_tag;

    // Получаем ключ из окружения
    char *env_key = getenv("DECRYPTION_KEY");
    if (!env_key || strlen(env_key) != 32) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, 
            "Invalid DECRYPTION_KEY length. Must be exactly 32 bytes");
        return 0;
    }

    // Генерация ключа через HKDF
    unsigned char decrypt_key[32];
    {
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
        if (!pctx) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "HKDF context creation failed");
            return 0;
        }

        EVP_PKEY_derive_init(pctx);
        EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256());
        EVP_PKEY_CTX_set1_hkdf_key(pctx, (const unsigned char *)env_key, 32);
        EVP_PKEY_CTX_set1_hkdf_salt(pctx, nonce, 12);
        EVP_PKEY_CTX_add1_hkdf_info(pctx, (const unsigned char *)"cryphptor", 9);

        size_t outlen = 32;
        if (EVP_PKEY_derive(pctx, decrypt_key, &outlen) <= 0) {
            EVP_PKEY_CTX_free(pctx);
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "HKDF derivation failed");
            return 0;
        }
        EVP_PKEY_CTX_free(pctx);
    }

    // Расшифровка
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cipher context creation failed");
        return 0;
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, decrypt_key, nonce)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption init failed");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);

    *plaintext_len = ct_len - 16;
    *plaintext = emalloc(*plaintext_len + 1); // +1 для завершающего нуля
    int len;

    if (!EVP_DecryptUpdate(ctx, *plaintext, &len, ct, ct_len - 16)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption update failed");
        efree(*plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    *plaintext_len = len;
    if (!EVP_DecryptFinal_ex(ctx, (*plaintext) + len, &len)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption failed (invalid tag)");
        efree(*plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    *plaintext_len += len;
    (*plaintext)[*plaintext_len] = '\0'; // Добавляем завершающий ноль

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

// Stream wrapper operations
static php_stream *php_cryphptor_stream_opener(php_stream_wrapper *wrapper, 
                                              const char *filename, 
                                              const char *mode, 
                                              int options, 
                                              zend_string **opened_path, 
                                              php_stream_context *context STREAMS_DC TSRMLS_DC) {
    php_stream *stream = NULL;
    FILE *fp = NULL;
    char *actual_filename = (char *)filename;
    
    // Если имя файла начинается с cryphptor://, убираем префикс
    if (strncmp(filename, "cryphptor://", 12) == 0) {
        actual_filename += 12;
    }
    
    // Открываем зашифрованный файл
    fp = fopen(actual_filename, "rb");
    if (!fp) {
        return NULL;
    }

    // Получаем размер файла
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Проверяем минимальный размер
    if (file_size < CRYPTHPTOR_MIN_SIZE) {
        fclose(fp);
        return NULL;
    }

    // Читаем файл в память
    char *ciphertext = emalloc(file_size);
    size_t read_size = fread(ciphertext, 1, file_size, fp);
    fclose(fp);

    if (read_size != file_size) {
        efree(ciphertext);
        return NULL;
    }

    // Дешифруем данные
    unsigned char *plaintext = NULL;
    size_t plaintext_len = 0;
    if (!cryphptor_decrypt_data(ciphertext, file_size, &plaintext, &plaintext_len TSRMLS_CC)) {
        efree(ciphertext);
        return NULL;
    }
    
    efree(ciphertext);

    // Создаем структуру данных потока
    cryphptor_stream_data *stream_data = emalloc(sizeof(cryphptor_stream_data));
    stream_data->decrypted_data = plaintext;
    stream_data->decrypted_size = plaintext_len;
    stream_data->pos = 0;

    // Создаем поток в памяти
    stream = php_stream_alloc(&php_stream_memory_ops, stream_data, 0, "rb");
    if (!stream) {
        efree(plaintext);
        efree(stream_data);
        return NULL;
    }

    if (opened_path) {
        *opened_path = zend_string_init(actual_filename, strlen(actual_filename), 0);
    }
    
    return stream;
}

// Stream wrapper для cryphptor
static php_stream_wrapper_ops php_cryphptor_wrapper_ops = {
    php_cryphptor_stream_opener,
    NULL,                                    /* close */
    NULL,                                    /* fstat */
    NULL,                                    /* stat */
    NULL,                                    /* opendir */
    "cryphptor",
    NULL,                                    /* unlink */
    NULL,                                    /* rename */
    NULL,                                    /* mkdir */
    NULL                                     /* rmdir */
};

static php_stream_wrapper php_cryphptor_wrapper = {
    &php_cryphptor_wrapper_ops,
    NULL,
    0
};

// Определяем версию модуля
#define PHP_CRYPTHPTOR_VERSION "1.0.0"

PHP_MINIT_FUNCTION(cryphptor) {
    // Регистрируем протокол cryphptor://
    php_register_url_stream_wrapper("cryphptor", &php_cryphptor_wrapper TSRMLS_CC);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(cryphptor) {
    return SUCCESS;
}

// Завершающий блок
#ifdef COMPILE_DL_CRYPTHPTOR
ZEND_GET_MODULE(cryphptor)
#endif

PHP_MINFO_FUNCTION(cryphptor) {
    php_info_print_table_start();
    php_info_print_table_row(2, "cryphptor support", "enabled");
    php_info_print_table_row(2, "Author", "Your Name");
    php_info_print_table_end();
}

// Объявление функций
PHP_FUNCTION(cryphptor_open_encrypted);

// Таблица функций
static const zend_function_entry cryphptor_functions[] = {
    PHP_FE(cryphptor_open_encrypted, NULL)
    PHP_FE_END
};

zend_module_entry cryphptor_module_entry = {
    STANDARD_MODULE_HEADER,
    "cryphptor",
    cryphptor_functions, // Обновляем для включения функций
    PHP_MINIT(cryphptor),
    PHP_MSHUTDOWN(cryphptor),
    NULL,
    NULL,
    PHP_MINFO(cryphptor),
    PHP_CRYPTHPTOR_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYPTHPTOR
ZEND_GET_MODULE(cryphptor)
#endif
