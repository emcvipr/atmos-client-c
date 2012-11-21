SRC=atmos.c atmos_util.c atmos_client.c atmos_common.c atmos_create.c \
	atmos_service_info.c atmos_read.c 
TESTSRC=seatest.c test.c test_atmos.c

OUTPUT_DIR=output
TEST_OUTPUT_DIR=testout

OBJ=$(addprefix $(OUTPUT_DIR)/,$(SRC:.c=.o))
TESTOBJ=$(addprefix $(TEST_OUTPUT_DIR)/,$(TESTSRC:.c=.o))

TESTEXE=$(TEST_OUTPUT_DIR)/atmostest

UNAME := $(shell uname)

VERSION = 2.1.0
SLIBNAME = libatmos.so
LIBNAME = libatmos.a
LIBDIR = -L/usr/local/lib 
XMLLIBS := $(shell xml2-config --libs)
XMLCFLAGS := $(shell xml2-config --cflags)
LIBS= -lssl -lcrypto -lcurl $(XMLLIBS)
CFLAGS = -Iinclude -Idep/rest-client-c/include -Wall -fPIC -g -c -D_FILE_OFFSET_BITS=64 -pthreads $(XMLCFLAGS)
SOFLAGS = -shared -Wl,-soname,$(SLIBNAME) -o $(SLIBNAME)
DEPLIB = $(OUTPUT_DIR)/librest.a
TESTLINK = -L$(OUTPUT_DIR) -latmos -lrest $(XMLLIBS)

ifeq ($(UNAME), Darwin)
	SLIBNAME = atmos.dylib
	SOFLAGS = -dynamiclib -undefined suppress -flat_namespace -o $(OUTPUT_DIR)/$(SLIBNAME)
endif 

all: dep prepare lib testharness

dep: rest-client-c

rest-client-c: dep/rest-client-c/output/librest.a

dep/rest-client-c/output/librest.a:
	cd dep/rest-client-c; make lib

prepare:
	mkdir -p $(OUTPUT_DIR)
	mkdir -p $(TEST_OUTPUT_DIR)

lib: prepare $(OBJ)
	gcc $(SOFLAGS) $(OBJ)
	cd $(OUTPUT_DIR); ar rcs $(LIBNAME) *.o
	
$(OUTPUT_DIR)/%.o : src/%.c
	gcc -c $(CFLAGS) $< -o $@ 
	
$(TEST_OUTPUT_DIR)/%.o : test/%.c
	gcc -c $(CFLAGS) $< -o $@
	
$(OUTPUT_DIR)/librest.a : dep/rest-client-c/output/librest.a
	cp dep/rest-client-c/output/librest.a $(OUTPUT_DIR)/librest.a
	
testharness: lib $(TESTOBJ) $(DEPLIB)
	gcc -pg -g -o $(TESTEXE) $(TESTLINK) $(LIBDIR) $(LIBS) $(TESTOBJ)

test: testharness
	$(TESTEXE)
	
doc: 
	doxygen Doxyfile

tar:
	mkdir -p tar
	mkdir -p tar/atmos-c
	cp -r src test Makefile Doxyfile include tar/atmos-c
	find tar -name .svn | xargs rm -rf
	cd tar; tar cvzf ../atmos-c_$(VERSION).tar.gz atmos-c
	rm -rf tar

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.dylib
	rm -rf $(OUTPUT_DIR)
	rm -rf $(TEST_OUTPUT_DIR)
	cd dep/rest-client-c; make clean
	
