/* xor_decrypt extension for PHP - Прозрачная расшифровка PHP файлов */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_xor_decrypt.h"
#include "xor_decrypt_arginfo.h"

#include <stdio.h>
#include <string.h>

/* Глобальная переменная для ключа */
static char *xor_decrypt_key = NULL;
static size_t xor_decrypt_key_len = 0;

/* Оригинальная функция компиляции */
static zend_op_array *(*original_compile_file)(zend_file_handle *file_handle, int type) = NULL;

/* XOR расшифровка */
static void xor_decrypt_data(char *data, size_t len, const char *key, size_t key_len) {
    size_t i;
    for (i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

/* Наша функция компиляции - расшифровывает PHP файлы перед компиляцией */
static zend_op_array *xor_decrypt_compile_file(zend_file_handle *file_handle, int type) {
    /* Получаем ключ из environment */
    if (!xor_decrypt_key || xor_decrypt_key_len == 0) {
        char *env_key = getenv("XOR_DECRYPT_KEY");
        if (env_key && strlen(env_key) > 0) {
            xor_decrypt_key = env_key;
            xor_decrypt_key_len = strlen(env_key);
        }
    }
    
    /* Если нет ключа или это не компиляция файла - используем оригинальную функцию */
    if (!xor_decrypt_key || xor_decrypt_key_len == 0 || !file_handle || !file_handle->filename) {
        return original_compile_file(file_handle, type);
    }
    
    const char *filename = ZSTR_VAL(file_handle->filename);
    size_t filename_len = ZSTR_LEN(file_handle->filename);
    
    /* Проверяем, что это .php файл */
    if (filename_len < 4 || strcmp(filename + filename_len - 4, ".php") != 0) {
        return original_compile_file(file_handle, type);
    }
    
    /* Открываем и читаем файл */
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return original_compile_file(file_handle, type);
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fp);
        return original_compile_file(file_handle, type);
    }
    
    char *content = emalloc(file_size + 1);
    fread(content, 1, file_size, fp);
    fclose(fp);
    content[file_size] = '\0';
    
    /* Проверяем, зашифрован ли файл (не начинается с <?php) */
    if (file_size >= 5 && (memcmp(content, "<?php", 5) == 0 || memcmp(content, "#!", 2) == 0)) {
        efree(content);
        return original_compile_file(file_handle, type);
    }
    
    /* Расшифровываем */
    xor_decrypt_data(content, file_size, xor_decrypt_key, xor_decrypt_key_len);
    
    /* Проверяем, валидный ли PHP получился */
    if (file_size < 5 || memcmp(content, "<?php", 5) != 0) {
        efree(content);
        return original_compile_file(file_handle, type);
    }
    
    /* Проверяем и удаляем открывающий тег если есть */
    char *code_start = content;
    size_t code_len = file_size;
    
    if (file_size >= 5 && memcmp(content, "<?php", 5) == 0) {
        code_start = content + 5;
        code_len = file_size - 5;
    }
    
    /* Создаём zend_string с расшифрованным кодом */
    zend_string *code = zend_string_init(code_start, code_len, 0);
    efree(content);
    
    /* Компилируем строку как PHP код */
    zend_op_array *result = zend_compile_string(code, filename, ZEND_COMPILE_POSITION_AFTER_OPEN_TAG);
    
    zend_string_release(code);
    
    return result;
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(xor_decrypt)
{
    /* Сохраняем оригинальную функцию и заменяем на нашу */
    original_compile_file = zend_compile_file;
    zend_compile_file = xor_decrypt_compile_file;
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(xor_decrypt)
{
    /* Восстанавливаем оригинальную функцию */
    zend_compile_file = original_compile_file;
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(xor_decrypt)
{
#if defined(ZTS) && defined(COMPILE_DL_XOR_DECRYPT)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    
    /* Сбрасываем ключ для каждого запроса */
    xor_decrypt_key = NULL;
    xor_decrypt_key_len = 0;
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(xor_decrypt)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "xor_decrypt support", "enabled");
    php_info_print_table_row(2, "Transparent decryption", "enabled for .php files");
    php_info_print_table_end();
}
/* }}} */

/* {{{ xor_decrypt_functions[] */
static const zend_function_entry xor_decrypt_functions[] = {
    ZEND_FE(test1, arginfo_test1)
    ZEND_FE(test2, arginfo_test2)
    ZEND_FE(xor_decrypt, arginfo_xor_decrypt)
    ZEND_FE(xor_encrypt, arginfo_xor_encrypt)
    ZEND_FE_END
};
/* }}} */

/* {{{ xor_decrypt_module_entry */
zend_module_entry xor_decrypt_module_entry = {
    STANDARD_MODULE_HEADER,
    "xor_decrypt",
    xor_decrypt_functions,
    PHP_MINIT(xor_decrypt),
    PHP_MSHUTDOWN(xor_decrypt),
    PHP_RINIT(xor_decrypt),
    NULL,
    PHP_MINFO(xor_decrypt),
    PHP_XOR_DECRYPT_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XOR_DECRYPT
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(xor_decrypt)
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
    ZEND_PARSE_PARAMETERS_NONE();
    php_printf("XOR Decrypt extension loaded!\r\n");
}
/* }}} */

/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION(test2)
{
    char *var = "World";
    size_t var_len = sizeof("World") - 1;
    zend_string *retval;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STRING(var, var_len)
    ZEND_PARSE_PARAMETERS_END();

    retval = strpprintf(0, "Hello %s", var);
    RETURN_STR(retval);
}
/* }}} */

/* {{{ string xor_decrypt( string $data, string $key ) */
PHP_FUNCTION(xor_decrypt)
{
    char *data, *key;
    size_t data_len, key_len;
    char *result;
    size_t i;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(data, data_len)
        Z_PARAM_STRING(key, key_len)
    ZEND_PARSE_PARAMETERS_END();

    if (key_len == 0) {
        php_error_docref(NULL, E_WARNING, "Key cannot be empty");
        RETURN_FALSE;
    }

    result = emalloc(data_len + 1);
    for (i = 0; i < data_len; i++) {
        result[i] = data[i] ^ key[i % key_len];
    }
    result[data_len] = '\0';

    RETVAL_STRINGL(result, data_len);
    efree(result);
}
/* }}} */

/* {{{ string xor_encrypt( string $data, string $key ) */
PHP_FUNCTION(xor_encrypt)
{
    char *data, *key;
    size_t data_len, key_len;
    char *result;
    size_t i;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(data, data_len)
        Z_PARAM_STRING(key, key_len)
    ZEND_PARSE_PARAMETERS_END();

    if (key_len == 0) {
        php_error_docref(NULL, E_WARNING, "Key cannot be empty");
        RETURN_FALSE;
    }

    result = emalloc(data_len + 1);
    for (i = 0; i < data_len; i++) {
        result[i] = data[i] ^ key[i % key_len];
    }
    result[data_len] = '\0';

    RETVAL_STRINGL(result, data_len);
    efree(result);
}
/* }}} */
