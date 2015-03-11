lib_LTLIBRARIES = libatmos.la
libatmos_la_SOURCES = atmos.c atmos_client.c atmos_common.c atmos_create.c \
 atmos_read.c atmos_service_info.c atmos_util.c atmos_access_tokens.c \
 atmos_update.c atmos_versions.c atmos_token_policy.c policy_no_ns.xsd
libatmos_la_LDFLAGS = -version-info 0:0:0 $(REST_LIBS) $(CURL_LIBS) $(XML_LIBS)
libatmos_la_CFLAGS = $(XML_CFLAGS) -I$(top_srcdir)/dep/rest-client-c/lib
include_HEADERS = atmos.h atmos_client.h atmos_common.h atmos_constants.h \
 atmos_create.h atmos_read.h atmos_service_info.h atmos_util.h \
 atmos_access_tokens.h atmos_update.h atmos_versions.h atmos_token_policy.h
noinst_HEADERS = atmos_private.h
BUILT_SOURCES = atmos_token_policy.c atmos_token_policy.h
CLEANFILES = $(BUILT_SOURCES) 

atmos_token_policy.c atmos_token_policy.h : policy_no_ns.xsd
	xsltproc --stringparam prefix Atmos \
		--stringparam module-name atmos_token_policy  \
		../xmlbind/xsd_to_c.xsl policy_no_ns.xsd 

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = atmos-c.pc

