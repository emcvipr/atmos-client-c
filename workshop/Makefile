LIBS=-latmos -L/usr/local/lib/ -I/usr/local/include -lssl -lcurl 

atmos_create01: create01.c
	gcc -g -o create create01.c ${INC} ${LIBS}
read01: read01.c
	gcc -g -o read read01.c ${INC} ${LIBS}
update01: update01.c
	gcc -g -o update update01.c ${INC} ${LIBS}
delete01: delete01.c
	gcc -g -o delete delete01.c ${INC} ${LIBS}
meta: meta_data01.c
	gcc -g -o meta meta_data01.c ${INC} ${LIBS}	
sysmeta: sys_meta_data01.c
	gcc -g -o sys_meta sys_meta_data01.c ${INC} ${LIBS}	
upmeta: updatemeta_data01.c
	gcc -g -o upmeta updatemeta_data01.c ${INC} ${LIBS}	
meta_helpers: meta_helpers_test.c
	gcc -g -o meta_helpers  meta_helpers_test.c ${INC} ${LIBS}	
getmeta: getmeta_data01.c
	gcc -g -o getmeta getmeta_data01.c ${INC} ${LIBS}	
acl: acl_data01.c
	gcc -g -o al acl_data01.c ${INC} ${LIBS}	