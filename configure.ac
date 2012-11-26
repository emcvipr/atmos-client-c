AC_INIT([atmos-c], [2.1.0], [jason_d0t_cwik@emc_d0t_com])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_LIB([curl], [curl_easy_init], [CURLLIB=-lcurl])
AC_SUBST([CURLLIB])
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)
AC_CHECK_HEADERS([libxml/parser.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile tests/Makefile])
AC_CONFIG_SUBDIRS([dep/rest-client-c])
AC_OUTPUT
