#include "php.h"
#include "ext/standard/info.h"

#define CRYPTHPTOR_VERSION "1.0.0"

// Глобальная переменная для хранения корневой директории
static char *cryphptor_root_dir = NULL;

// PHP функции
ZEND_BEGIN_ARG_INFO_EX(arginfo_cryphptor_config, 0, 0, 1)
    ZEND_ARG_INFO(0, setting)
    ZEND_ARG_INFO(0, value)
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

PHP_FUNCTION(cryphptor_test) {
    RETURN_STRING("CRYPHPTOR extension is working!");
}

// Таблица функций
static const zend_function_entry cryphptor_functions[] = {
    PHP_FE(cryphptor_config, arginfo_cryphptor_config)
    PHP_FE(cryphptor_test, NULL)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(cryphptor) {
    cryphptor_root_dir = estrdup("/var/www/laravel");
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
    php_info_print_table_row(2, "Version", CRYPTHPTOR_VERSION);
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
    CRYPTHPTOR_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYPTHPTOR
ZEND_GET_MODULE(cryphptor)
#endif