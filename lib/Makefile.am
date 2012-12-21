lib_LTLIBRARIES = libatmos.la
libatmos_la_SOURCES = atmos.c atmos_client.c atmos_common.c atmos_create.c \
 atmos_read.c atmos_service_info.c atmos_util.c atmos_access_tokens.c \
 atmos_update.c atmos_versions.c atmos_token_policy.c policy.xsd \
 uncrustify.cfg
libatmos_la_LDFLAGS = -version-info 0:0:0
libatmos_la_CFLAGS = $(XML_CFLAGS) -I$(top_srcdir)/dep/rest-client-c/lib
include_HEADERS = atmos.h atmos_client.h atmos_common.h atmos_constants.h \
 atmos_create.h atmos_read.h atmos_service_info.h atmos_util.h \
 atmos_access_tokens.h atmos_update.h atmos_versions.h atmos_token_policy.h
noinst_HEADERS = atmos_private.h
BUILT_SOURCES = atmos_token_policy.c atmos_token_policy.h
CLEANFILES = $(BUILT_SOURCES) atmos_token_policy.xb.c atmos_token_policy.xb.h
atmos_token_policy.c : atmos_token_policy.xb.c atmos_token_policy.h
	$(CC) -I.. -I../xmlbind -P -E atmos_token_policy.xb.c -o atmos_token_policy.c
	if [ "$(USE_UNCRUSTIFY)" == "yes" ]; then uncrustify -c uncrustify.cfg --replace -l C atmos_token_policy.c; fi
	
atmos_token_policy.h : atmos_token_policy.xb.c
	$(CC) -I.. -I../xmlbind -P -E atmos_token_policy.xb.h -o atmos_token_policy.h
	if [ "$(USE_UNCRUSTIFY)" == "yes" ]; then uncrustify -c uncrustify.cfg --replace -l C atmos_token_policy.h; fi

atmos_token_policy.xb.c : policy.xsd
	../xmlbind/xmlbind policy.xsd atmos_token_policy
