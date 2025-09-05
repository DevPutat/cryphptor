#include "php.h"
#include "ext/standard/info.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define CRYPTHPTOR_MIN_SIZE 28 // 12 (nonce) + 16 (tag)

static zend_op_array *(*orig_compile_file)(zend_file_handle *, int TSRMLS_DC);

static zend_op_array *cryphptor_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {
    // Проверяем тип файла
    if (file_handle->type != ZEND_HANDLE_FILENAME) {
        return orig_compile_file(file_handle, type TSRMLS_CC);
    }

    // Открываем файл
    FILE *fp = fopen(file_handle->filename, "rb");
    if (!fp) {
        return orig_compile_file(file_handle, type TSRMLS_CC);
    }

    // Получаем размер файла
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Проверяем минимальный размер
    if (file_size < CRYPTHPTOR_MIN_SIZE) {
        fclose(fp);
        return orig_compile_file(file_handle, type TSRMLS_CC);
    }

    // Читаем файл в память
    unsigned char *ciphertext = emalloc(file_size);
    size_t read_size = fread(ciphertext, 1, file_size, fp);
    fclose(fp);

    if (read_size != file_size) {
        efree(ciphertext);
        return orig_compile_file(file_handle, type TSRMLS_CC);
    }

    // Извлекаем компоненты
    unsigned char *nonce = ciphertext;                  // 12 байт
    unsigned char *ciphertext_and_tag = ciphertext + 12; // Остальное
    size_t ct_len = file_size - 12;
    unsigned char *tag = ciphertext_and_tag + (ct_len - 16);
    unsigned char *ct = ciphertext_and_tag;

    // Получаем ключ из окружения
    char *env_key = getenv("DECRYPTION_KEY");
    if (!env_key || strlen(env_key) != 32) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, 
            "Invalid DECRYPTION_KEY length. Must be exactly 32 bytes");
        efree(ciphertext);
        return NULL;
    }

    // Генерация ключа через HKDF
    unsigned char decrypt_key[32];
    {
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
        if (!pctx) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "HKDF context creation failed");
            efree(ciphertext);
            return NULL;
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
            efree(ciphertext);
            return NULL;
        }
        EVP_PKEY_CTX_free(pctx);
    }

    // Расшифровка
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cipher context creation failed");
        efree(ciphertext);
        return NULL;
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, decrypt_key, nonce)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption init failed");
        EVP_CIPHER_CTX_free(ctx);
        efree(ciphertext);
        return NULL;
    }

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);

    int plaintext_len = ct_len - 16;
    unsigned char *plaintext = emalloc(plaintext_len);
    int len;

    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ct, ct_len - 16)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption update failed");
        efree(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        efree(ciphertext);
        return NULL;
    }

    plaintext_len = len;
    if (!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Decryption failed (invalid tag)");
        efree(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        efree(ciphertext);
        return NULL;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    efree(ciphertext);

    // Создаем временный файл
    char tmp_path[] = "/tmp/cryphptor_XXXXXX";
    int fd = mkstemp(tmp_path);
    if (fd == -1) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Temp file creation failed");
        efree(plaintext);
        return NULL;
    }

    if (write(fd, plaintext, plaintext_len) != plaintext_len) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Temp file write failed");
        close(fd);
        unlink(tmp_path);
        efree(plaintext);
        return NULL;
    }

    close(fd);
    efree(plaintext);

    // Компилируем расшифрованный файл
    zend_file_handle tmp_file = {0};
    tmp_file.type = ZEND_HANDLE_FILENAME;
    tmp_file.filename = tmp_path;
    zend_op_array *op_array = orig_compile_file(&tmp_file, type TSRMLS_CC);

    // Удаляем временный файл
    unlink(tmp_path);

    return op_array;
}

PHP_MINIT_FUNCTION(cryphptor) {
    orig_compile_file = zend_compile_file;
    zend_compile_file = cryphptor_compile_file;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(cryphptor) {
    zend_compile_file = orig_compile_file;
    return SUCCESS;
}

PHP_MINFO_FUNCTION(cryphptor) {
    php_info_print_table_start();
    php_info_print_table_row(2, "cryphptor support", "enabled");
    php_info_print_table_row(2, "Author", "Your Name");
    php_info_print_table_end();
}

zend_module_entry cryphptor_module_entry = {
    STANDARD_MODULE_HEADER,
    "cryphptor",
    NULL,
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
