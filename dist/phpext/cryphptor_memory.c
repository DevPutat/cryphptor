#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "SAPI.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>

#ifndef STREAMS_CC
#define STREAMS_CC TSRMLS_CC
#endif

#define CRYPTHPTOR_MIN_SIZE 28

// Глобальная переменная для хранения корневой директории
static char *cryphptor_root_dir = NULL;

// Сохраняем указатель на оригинальную функцию компиляции
static zend_op_array* (*original_compile_file)(zend_file_handle *file_handle, int type) = NULL;

// Функция для дешифровки данных в памяти
static int cryphptor_decrypt_data(const char *ciphertext, size_t ciphertext_len, 
                                   unsigned char **plaintext, size_t *plaintext_len) {
    if (ciphertext_len < CRYPTHPTOR_MIN_SIZE) {
        return 0;
    }

    const unsigned char *nonce = (const unsigned char *)ciphertext;
    const unsigned char *ciphertext_and_tag = (const unsigned char *)(ciphertext + 12);
    size_t ct_len = ciphertext_len - 12;
    const unsigned char *tag = ciphertext_and_tag + (ct_len - 16);
    const unsigned char *ct = ciphertext_and_tag;

    char *env_key = getenv("DECRYPTION_KEY");
    if (!env_key || strlen(env_key) != 32) {
        return 0;
    }

    // Генерация ключа через HKDF
    unsigned char decrypt_key[32];
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    if (!pctx) {
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
        return 0;
    }
    EVP_PKEY_CTX_free(pctx);

    // Расшифровка AES-256-GCM
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0;
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, decrypt_key, nonce)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag);

    *plaintext_len = ct_len - 16;
    *plaintext = emalloc(*plaintext_len + 1);
    int len;

    if (!EVP_DecryptUpdate(ctx, *plaintext, &len, ct, ct_len - 16)) {
        efree(*plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    *plaintext_len = len;
    if (!EVP_DecryptFinal_ex(ctx, (*plaintext) + len, &len)) {
        efree(*plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    *plaintext_len += len;
    (*plaintext)[*plaintext_len] = '\0';

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

// Проверяет, является ли файл зашифрованным
static int is_encrypted_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size < CRYPTHPTOR_MIN_SIZE) {
        fclose(fp);
        return 0;
    }
    
    unsigned char header[12];
    size_t read_size = fread(header, 1, 12, fp);
    fclose(fp);
    
    return read_size == 12;
}

// Проверяет, находится ли файл в целевой директории
static int is_target_file(const char *filename) {
    if (!cryphptor_root_dir || !filename) return 0;
    
    size_t root_len = strlen(cryphptor_root_dir);
    if (strncmp(filename, cryphptor_root_dir, root_len) == 0) {
        if (filename[root_len] == '\0' || filename[root_len] == '/') {
            const char *ext = strrchr(filename, '.');
            if (ext && (strcmp(ext, ".php") == 0 || strcmp(ext, ".inc") == 0 || 
                       strcmp(ext, ".phtml") == 0)) {
                return 1;
            }
        }
    }
    return 0;
}

// Хук для перехвата компиляции файлов
static zend_op_array* cryphptor_compile_file(zend_file_handle *file_handle, int type) {
    const char *filename = ZSTR_VAL(file_handle->filename);
    
    // Проверяем, нужно ли обрабатывать этот файл
    if (!is_target_file(filename) || !is_encrypted_file(filename)) {
        // Если не целевой файл, вызываем оригинальную функцию
        return original_compile_file(file_handle, type);
    }

    // Читаем зашифрованный файл в память
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return original_compile_file(file_handle, type);
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < CRYPTHPTOR_MIN_SIZE) {
        fclose(fp);
        return original_compile_file(file_handle, type);
    }

    char *ciphertext = emalloc(file_size);
    size_t read_size = fread(ciphertext, 1, file_size, fp);
    fclose(fp);

    if (read_size != file_size) {
        efree(ciphertext);
        return original_compile_file(file_handle, type);
    }

    // Дешифруем в память
    unsigned char *plaintext = NULL;
    size_t plaintext_len = 0;
    if (!cryphptor_decrypt_data(ciphertext, file_size, &plaintext, &plaintext_len)) {
        efree(ciphertext);
        return original_compile_file(file_handle, type);
    }

    efree(ciphertext);

    // Создаем zend_string с дешифрованным содержимым
    zend_string *content = zend_string_init((char *)plaintext, plaintext_len, 0);
    efree(plaintext);

    // Создаем PHP stream из строки в памяти (PHP 8.4: mode=0 для чтения)
    php_stream *stream = _php_stream_memory_open(0, content STREAMS_CC);
    if (!stream) {
        zend_string_release(content);
        return original_compile_file(file_handle, type);
    }

    // Создаем временный file_handle с stream (PHP 8.4)
    zend_file_handle new_handle;
    memset(&new_handle, 0, sizeof(new_handle));
    
    new_handle.type = ZEND_HANDLE_STREAM;
    new_handle.filename = zend_string_copy(file_handle->filename);
    new_handle.opened_path = file_handle->opened_path ? zend_string_copy(file_handle->opened_path) : NULL;
    
    // Для PHP 8.4 используем правильную структуру
    new_handle.handle.stream.handle = stream;
    new_handle.handle.stream.isatty = 0;
    new_handle.handle.stream.closer = (zend_stream_closer_t)_php_stream_free;

    // Вызываем оригинальную функцию компиляции с дешифрованным содержимым
    zend_op_array *op_array = original_compile_file(&new_handle, type);

    // Очистка ресурсов
    if (new_handle.filename) {
        zend_string_release(new_handle.filename);
    }
    if (new_handle.opened_path) {
        zend_string_release(new_handle.opened_path);
    }
    // content освобождается stream автоматически через _php_stream_free

    return op_array;
}

// PHP функции
ZEND_BEGIN_ARG_INFO_EX(arginfo_cryphptor_config, 0, 0, 1)
    ZEND_ARG_INFO(0, setting)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cryphptor_decrypt_file, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cryphptor_test, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(cryphptor_config) {
    char *setting, *value;
    size_t setting_len, value_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &setting, &setting_len, &value, &value_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (strcmp(setting, "root") == 0) {
        if (cryphptor_root_dir) {
            efree(cryphptor_root_dir);
        }
        cryphptor_root_dir = estrndup(value, value_len);
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

PHP_FUNCTION(cryphptor_decrypt_file) {
    char *filename;
    size_t filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        RETURN_FALSE;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < CRYPTHPTOR_MIN_SIZE) {
        fclose(fp);
        RETURN_FALSE;
    }

    char *ciphertext = emalloc(file_size);
    size_t read_size = fread(ciphertext, 1, file_size, fp);
    fclose(fp);

    if (read_size != file_size) {
        efree(ciphertext);
        RETURN_FALSE;
    }

    unsigned char *plaintext = NULL;
    size_t plaintext_len = 0;
    if (!cryphptor_decrypt_data(ciphertext, file_size, &plaintext, &plaintext_len)) {
        efree(ciphertext);
        RETURN_FALSE;
    }

    efree(ciphertext);

    RETVAL_STRINGL((char *)plaintext, plaintext_len);
    efree(plaintext);
}

PHP_FUNCTION(cryphptor_test) {
    RETURN_STRING("CRYPHPTOR Memory Decryption is working!");
}

// Таблица функций
static const zend_function_entry cryphptor_functions[] = {
    PHP_FE(cryphptor_config, arginfo_cryphptor_config)
    PHP_FE(cryphptor_decrypt_file, arginfo_cryphptor_decrypt_file)
    PHP_FE(cryphptor_test, arginfo_cryphptor_test)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(cryphptor) {
    cryphptor_root_dir = estrdup("/var/www/laravel");
    
    // Сохраняем оригинальную функцию компиляции
    original_compile_file = zend_compile_file;
    
    // Заменяем функцию компиляции на наш хук
    zend_compile_file = cryphptor_compile_file;
    
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(cryphptor) {
    if (cryphptor_root_dir) {
        efree(cryphptor_root_dir);
        cryphptor_root_dir = NULL;
    }
    return SUCCESS;
}

PHP_MINFO_FUNCTION(cryphptor) {
    php_info_print_table_start();
    php_info_print_table_row(2, "cryphptor support", "enabled");
    php_info_print_table_row(2, "Version", "2.0.0 (Memory Decryption)");
    php_info_print_table_row(2, "Mode", "In-Memory Decryption");
    php_info_print_table_row(2, "Root Directory", cryphptor_root_dir ? cryphptor_root_dir : "Not set");
    php_info_print_table_end();
}

zend_module_entry cryphptor_module_entry = {
    STANDARD_MODULE_HEADER,
    "cryphptor",
    cryphptor_functions,
    PHP_MINIT(cryphptor),
    PHP_MSHUTDOWN(cryphptor),
    NULL,
    NULL,
    PHP_MINFO(cryphptor),
    "2.0.0",
    STANDARD_MODULE_PROPERTIES
};

// Always export the module entry for PHP 8.4
zend_module_entry* get_module(void) {
    return &cryphptor_module_entry;
}
