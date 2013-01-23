AC_INIT([atmos-c], [2.1.0], [jason_d0t_cwik@emc_d0t_com])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT
AC_PROG_CC
AM_PROG_CC_C_O
AX_CFLAGS_WARN_ALL
AX_CHECK_COMPILE_FLAG([-Werror], [CFLAGS="$CFLAGS -Werror"], [], [])
AC_CHECK_PROG(USE_XSLTPROC, xsltproc, yes, no)
if test $USE_XSLTPROC = "no" ; then
    AC_MSG_ERROR([The xsltproc command from libxml2 is required.])
fi
AC_CHECK_LIB([curl], [curl_easy_init], [CURLLIB=-lcurl])
AC_SUBST([CURLLIB])
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)
PKG_CHECK_MODULES(SSL, openssl >= 0.9.8)
AC_CHECK_HEADERS([libxml/parser.h])
AC_DEFINE([_XOPEN_SOURCE], [600], [Some required extensions])
AC_CHECK_MEMBERS([struct tm.tm_gmtoff, struct tm.__tm_gmtoff], [], [], [#include <time.h>])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile xmlbind/Makefile lib/Makefile tests/Makefile])
AC_CONFIG_SUBDIRS([dep/rest-client-c])
AC_OUTPUT
