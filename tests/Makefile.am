AM_CFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/dep/rest-client-c/lib
TESTS = check_atmos
bin_PROGRAMS = check_atmos
check_atmos_CFLAGS = $(XML_CFLAGS) -I$(top_srcdir)/dep/rest-client-c/lib $(AM_CFLAGS)
check_atmos_SOURCES = seatest.c seatest.h test.c test.h test_atmos.c test_atmos.h test_xmlbind.c test_xmlbind.h atmostest.ini.template dirlist.xml
check_atmos_LDADD = ../lib/libatmos.la \
	$(top_srcdir)/dep/rest-client-c/lib/librest.la $(CURLLIB) $(XML_LIBS) \
	$(SSL_LIBS)

