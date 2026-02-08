PHP_ARG_WITH([cryphptor],
  [for cryphptor support],
  [AS_HELP_STRING([--with-cryphptor],
    [Include cryphptor support])])

if test "$PHP_CRYPHPTOR" != "no"; then
  dnl Проверяем наличие pkg-config
  PKG_CHECK_MODULES([OPENSSL], [openssl],
  [
    PHP_EVAL_INCLINE($OPENSSL_CFLAGS)
    PHP_EVAL_LIBLINE($OPENSSL_LIBS, CRYPHPTOR_SHARED_LIBADD)
    AC_DEFINE(HAVE_CRYPHPTOR, 1, [Whether you have cryphptor])
  ],[
    dnl Fallback: проверяем библиотеки напрямую через pkg-config
    if test -n "$PKG_CONFIG" && $PKG_CONFIG --exists openssl; then
      OPENSSL_CFLAGS=`$PKG_CONFIG --cflags openssl`
      OPENSSL_LIBS=`$PKG_CONFIG --libs openssl`
      PHP_EVAL_INCLINE($OPENSSL_CFLAGS)
      PHP_EVAL_LIBLINE($OPENSSL_LIBS, CRYPHPTOR_SHARED_LIBADD)
      AC_DEFINE(HAVE_CRYPHPTOR, 1, [Whether you have cryphptor])
    else
      dnl Fallback: проверяем стандартные пути
      PHP_CHECK_LIBRARY(crypto, EVP_EncryptInit_ex,
      [
        PHP_ADD_LIBRARY_WITH_PATH(crypto, /usr/lib, CRYPHPTOR_SHARED_LIBADD)
        PHP_ADD_LIBRARY_WITH_PATH(ssl, /usr/lib, CRYPHPTOR_SHARED_LIBADD)
        AC_DEFINE(HAVE_CRYPHPTOR, 1, [Whether you have cryphptor])
      ],[
        AC_MSG_ERROR([cryphptor requires OpenSSL libraries])
      ],[
        -lcrypto -lssl
      ])
    fi
  ])

  PHP_NEW_EXTENSION(cryphptor, cryphptor_memory.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi