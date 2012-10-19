SRC=atmos_rest.c atmos_util.c crypto.c transport.c
TESTSRC=seatest.c test.c
OBJ=$(SRC:.c=.o)
TESTOBJ=$(TESTSRC:.c=.o)

UNAME := $(shell uname)

VERSION = 1
SLIBNAME = libatmos.so
LIBNAME = libatmos.a
LIBDIR = -L/usr/local/lib
LIBS= -lssl -lcrypto -lcurl
FLAGS = -Wall -fPIC -g -c -D_FILE_OFFSET_BITS=64
SOFLAGS = -shared -Wl,-soname,$(SLIBNAME) -o $(SLIBNAME)

ifeq ($(UNAME), Darwin)
	SLIBNAME = atmos.dylib
	SOFLAGS = -dynamiclib -undefined suppress -flat_namespace -o $(SLIBNAME)
endif 

all: dep objects lib testharness

dep: rest-client-c

rest-client-c: dep/rest-client-c/output/librest.a

dep/rest-client-c/output/librest.a:
	cd dep/rest-client-c; make lib


objects: $(SRC) $(TESTSRC)
	gcc $(FLAGS) $(SRC) $(TESTSRC)
lib: objects
	gcc $(SOFLAGS) $(OBJ) 
	ar rcs $(LIBNAME) $(OBJ)
testharness: objects lib
	gcc -pg -g -o atmostest $(TESTOBJ) -L . -l atmos $(LIBS) $(LIBDIR)
test: testharness
	./atmostest

terawriter: terawriter.c
	gcc -pg -g -o terawriter ${OBJ} terawriter.c $(LIBS) $(LIBDIR)

lister: lister.c
	gcc -pg -g -o lister ${OBJ} lister.c $(LIBS) $(LIBDIR)

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.dylib
