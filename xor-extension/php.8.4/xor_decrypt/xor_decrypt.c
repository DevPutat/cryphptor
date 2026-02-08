/* xor_decrypt extension for PHP */

#include <stddef.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ext/standard/info.h"
#include "php.h"
#include "php_xor_decrypt.h"
#include "xor_decrypt_arginfo.h"

#include <Zend/zend.h>
#include <stdio.h>
#include <string.h>
#include <zend_API.h>

static char *xor_decrypt(const char *input, size_t len, const char *key) {
  char *output = emalloc(len + 1);
  for (size_t i = 0; i < len; ++i) {
    output[i] = input[i] ^ key[i % strlen(key)];
  }
  output[len] = '\0';
  return output;
}

PHP_MINIT_FUNCTION(xor_decrypt) {zend_set_compile_hook(
  zend_set_compile_hook(ZEND_COMPILE_HOOK_PHP_FILE_OPEN, xor_decrypt_file_open_handler);
  return SUCCESS;
}

static int xor_decrypt_file_open_handler(zend_file_handle *handle,
                                         char **filename,
                                         int options TSRMLS_DC) {
  if (!strstr(*filename, ".enc")) {
    return ZEND_COMPILE_CONTINUE;
  }
  FILE *fp = fopen(*filename, "rb");
  if (!fp) {
    return FAILURE;
  }
  fseek(fp, 0L, SEEK_END);
  long file_size = ftell(fp);
  rewind(fp);

  char *buffer = emalloc(file_size + 1);
  fread(buffer, sizeof(char), file_size, fp);
  buffer[file_size] = '\0';
  fclose(fp);
  char *decrypted_data = xor_decrypt(buffer, file_size, "my_secret_key");
  handle->opened_path = estrdup(*filename);
  handle->type = ZEND_HANDLE_FILENAME;
  handle->free_filename = 1;
  handle->filename = estrndup(decrypted_data, strlen(decrypted_data));
  efree(buffer);
  efree(decrypted_data);
  return SUCCESS;
}
PHP_FE_END
/* }}} */
const zend_function_entry xor_decrypt_functions[] = {PHP_FE_END};
zend_module_entry xor_decrypt_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "xor_decrypt",
    xor_decrypt_functions,
    PHP_MINIT(xor_decrypt),
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    NO_VERSION_YET,
#endif
    STANDARD_MODULE_PROPERTIES};
#ifdef COMPILE_DL_XOR_DECRYPT
ZEND_GET_MODULE(xor_decrypt)
#endif
/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()                                           \
  ZEND_PARSE_PARAMETERS_START(0, 0)                                            \
  ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_printf("The extension %s is loaded and working!\r\n", "xor_decrypt");
}
/* }}} */

/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION(test2) {
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
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(xor_decrypt) {
#if defined(ZTS) && defined(COMPILE_DL_XOR_DECRYPT)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(xor_decrypt) {
  php_info_print_table_start();
  php_info_print_table_row(2, "xor_decrypt support", "enabled");
  php_info_print_table_end();
}
/* }}} */

/* {{{ xor_decrypt_module_entry */
zend_module_entry xor_decrypt_module_entry = {
    STANDARD_MODULE_HEADER,
    "xor_decrypt",           /* Extension name */
    ext_functions,           /* zend_function_entry */
    NULL,                    /* PHP_MINIT - Module initialization */
    NULL,                    /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(xor_decrypt),  /* PHP_RINIT - Request initialization */
    NULL,                    /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(xor_decrypt),  /* PHP_MINFO - Module info */
    PHP_XOR_DECRYPT_VERSION, /* Version */
    STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_XOR_DECRYPT
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(xor_decrypt)
#endif
