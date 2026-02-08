#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

static const zend_function_entry cryphptor_functions[] = {
    PHP_FE_END
};

zend_module_entry cryphptor_module_entry = {
    STANDARD_MODULE_HEADER,
    "cryphptor",
    cryphptor_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "1.0.0",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYPTHPTOR
ZEND_GET_MODULE(cryphptor)
#endif