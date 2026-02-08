dnl config.m4 for extension xor_decrypt

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([xor_decrypt],
dnl   [for xor_decrypt support],
dnl   [AS_HELP_STRING([--with-xor_decrypt],
dnl     [Include xor_decrypt support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([xor_decrypt],
  [whether to enable xor_decrypt support],
  [AS_HELP_STRING([--enable-xor_decrypt],
    [Enable xor_decrypt support])],
  [no])

if test "$PHP_XOR_DECRYPT" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, XOR_DECRYPT_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-xor_decrypt -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/xor_decrypt.h"  # you most likely want to change this
  dnl if test -r $PHP_XOR_DECRYPT/$SEARCH_FOR; then # path given as parameter
  dnl   XOR_DECRYPT_DIR=$PHP_XOR_DECRYPT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for xor_decrypt files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       XOR_DECRYPT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$XOR_DECRYPT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the xor_decrypt distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-xor_decrypt -> add include path
  dnl PHP_ADD_INCLUDE($XOR_DECRYPT_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-xor_decrypt -> check for lib and symbol presence
  dnl LIBNAME=XOR_DECRYPT # you may want to change this
  dnl LIBSYMBOL=XOR_DECRYPT # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_XOR_DECRYPT_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your xor_decrypt library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $XOR_DECRYPT_DIR/$PHP_LIBDIR, XOR_DECRYPT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_XOR_DECRYPT_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your xor_decrypt library.])
  dnl ],[
  dnl   -L$XOR_DECRYPT_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(XOR_DECRYPT_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_XOR_DECRYPT, 1, [ Have xor_decrypt support ])

  PHP_NEW_EXTENSION(xor_decrypt, xor_decrypt.c, $ext_shared)
fi
