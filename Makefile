SRC=atmos_rest.c atmos_util.c crypto.c seatest.c test.c transport.c
OBJ=$(SRC:.c=.o)

UNAME := $(shell uname)

VERSION = 1
LIBNAME = libatmos.so
LIBDIR = -L/usr/local/lib
LIBS= -lssl -lcrypto -lcurl
FLAGS = -Wall -fPIC -g -c -D_FILE_OFFSET_BITS=64
SOFLAGS = -shared -Wl,-soname,$(LIBNAME) -o $(LIBNAME)

ifeq ($(UNAME), Darwin)
	LIBNAME = atmos.dylib
	SOFLAGS = -dynamiclib -undefined suppress -flat_namespace -o $(LIBNAME)
endif 

all: objects lib testharness

objects: $(SRC)
	gcc $(FLAGS) $(SRC)
lib: objects
	gcc $(SOFLAGS) $(OBJ) 
testharness: objects
	gcc -pg -g -o atmostest ${OBJ} $(LIBS) $(LIBDIR)
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