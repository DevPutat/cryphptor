dnl $Id$
dnl config.m4 for cryphptor

PHP_ARG_ENABLE(cryphptor, whether to enable cryphptor support,
[  --enable-cryphptor           Enable cryphptor support])

if test "$PHP_CRYPTHPTOR" != "no"; then
  PHP_REQUIRE_CXX()
  AC_DEFINE(HAVE_CRYPTHPTOR, 1, [ ])
  PHP_NEW_EXTENSION(cryphptor, cryphptor.c, $ext_shared)
  PHP_ADD_LIBRARY_WITH_PATH(crypto, [], CRYPTHPTOR_SHARED_LIBADD)
fi
