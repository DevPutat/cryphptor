#ifndef PHP_CRYPHPTOR_H
#define PHP_CRYPHPTOR_H

extern zend_module_entry cryphptor_module_entry;
#define phpext_cryphptor_ptr &cryphptor_module_entry

#define PHP_CRYPHPTOR_VERSION "1.0.0"

#ifdef PHP_WIN32
#	define PHP_CRYPHPTOR_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_CRYPHPTOR_API __attribute__ ((visibility("default")))
#else
#	define PHP_CRYPHPTOR_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(ZTS) && defined(COMPILE_DL_CRYPHPTOR)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

/* Forward declarations for the stream open functions */
#if PHP_VERSION_ID >= 80400
zend_result cryphptor_stream_open_php84(zend_file_handle *handle);
#elif PHP_VERSION_ID >= 80311
zend_result cryphptor_stream_open_php84(zend_file_handle *handle);
#else
int cryphptor_stream_open(const char *filename, zend_file_handle *handle);
#endif

/* Function to initialize original function pointers */
void cryphptor_init_original_functions(void *original_fn_ptr);

#endif /* PHP_CRYPHPTOR_H */