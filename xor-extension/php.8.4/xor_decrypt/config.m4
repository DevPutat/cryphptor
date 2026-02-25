dnl Autotools config.m4 for PHP extension xor_decrypt

dnl Comments in this file start with the string 'dnl' (discard to next line).
dnl Remove where necessary.

PHP_ARG_ENABLE([xor_decrypt],
  [whether to enable xor_decrypt support],
  [AS_HELP_STRING([--enable-xor_decrypt],
    [Enable xor_decrypt support])],
  [no])

AS_VAR_IF([PHP_XOR_DECRYPT], [no], [
  AC_MSG_RESULT([xor_decrypt extension is disabled])
], [
  dnl Define a preprocessor macro to indicate that this PHP extension can
  dnl be dynamically loaded as a shared module or is statically built into PHP.
  AC_DEFINE([HAVE_XOR_DECRYPT], [1],
    [Define to 1 if the PHP extension 'xor_decrypt' is available.])

  dnl Configure extension sources and compilation flags.
  PHP_NEW_EXTENSION([xor_decrypt],
    [xor_decrypt.c],
    [$ext_shared],,
    [-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1])
])
