/* xor_decrypt extension for PHP */

#ifndef PHP_XOR_DECRYPT_H
# define PHP_XOR_DECRYPT_H

extern zend_module_entry xor_decrypt_module_entry;
# define phpext_xor_decrypt_ptr &xor_decrypt_module_entry

# define PHP_XOR_DECRYPT_VERSION "1.0.0"

# if defined(ZTS) && defined(COMPILE_DL_XOR_DECRYPT)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

/* Function declarations */
PHP_FUNCTION(test1);
PHP_FUNCTION(test2);

/* Module function macros */
PHP_MINIT_FUNCTION(xor_decrypt);
PHP_MSHUTDOWN_FUNCTION(xor_decrypt);
PHP_RINIT_FUNCTION(xor_decrypt);
PHP_MINFO_FUNCTION(xor_decrypt);

#endif	/* PHP_XOR_DECRYPT_H */
