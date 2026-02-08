/* xor_decrypt extension for PHP */
#include <php.h>
#include <Zend/zend_extensions.h>
#include <ext/standard/info.h>

// Настройки конфигурации
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Минимум необходимой функциональности
PHP_MINIT_FUNCTION(xor_decrypt) {
    return SUCCESS;
}

// Информация о модуле
PHP_MINFO_FUNCTION(xor_decrypt) {
    php_info_print_table_start();
    php_info_print_table_row(2, "XOR Decryption Support", "Enabled");
    php_info_print_table_end();
}

// Экспортированные функции (пока пусто)
const zend_function_entry xor_decrypt_functions[] = {
    PHP_FE_END
};

// Главная структура расширения
zend_module_entry xor_decrypt_module_entry = {
    STANDARD_MODULE_HEADER,
    "xor_decrypt",
    xor_decrypt_functions,
    PHP_MINIT(xor_decrypt),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(xor_decrypt),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

// Макрос для получения модуля
#ifdef COMPILE_DL_XOR_DECRYPT
ZEND_GET_MODULE(xor_decrypt)
#endif
