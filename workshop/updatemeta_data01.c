/*

 */
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "atmos_rest.h"
#include "transport.h"
#define _XOPEN_SOURCE /* glibc2 needs this */
#include <time.h>

enum atmos_cmd{
    CREATE =0,
    READ,
    UPDATE,
    DELETE,
    SHARE,
    SYSMETA,
    UPDATEMETA,
    NONE
};

void do_atmos_action(enum atmos_cmd cmd, credentials *creds,char *object_id, char* bytes, acl *acl, user_meta* umd, int limit, ws_result *wsr);
void create_action_cmdline(credentials *creds, char *data, char *content_type, acl *acl, user_meta *meta,ws_result *wsr);
void read_action_cmdline(credentials *creds, char *object_id, int limit,ws_result *wsr);
void sysmeta_action_cmdline(credentials *creds, char *object_id,ws_result *wsr);
void update_action_cmdline(credentials *creds, char *object_id, char *data, char *content_type, acl *acl, user_meta *meta,ws_result *wsr);
void delete_action_cmdline(credentials *creds, char *object_id,ws_result *wsr);


void print_system_meta(system_meta sm) {

    printf("atime\t%s\n", sm.atime);
    printf("mtime\t%s\n", sm.mtime);
    printf("ctime\t%s\n", sm.ctime);
    printf("itime\t%s\n", sm.itime);
    printf("type\t%s\n", sm.type );
    printf("uid\t%s\n", sm.uid );
    printf("gid\t%s\n", sm.gid);
    printf("objectid\t%s\n", sm.objectid );
    printf("objname\t%s\n", sm.objname);
    printf("size\t%lld\n", sm.size);
    printf("nlink\t%d\n", sm.nlink);
    printf("policyname\t%s\n", sm.policyname);
  
}


void create_usage() 
{
    fprintf(stderr, "Usage:\n"
"--help Displays this message\n"
"--token Atmos userid\n"
"--secret Atmos shared secret\n"
"--host Atmos accesspoint\n"
"--create Creates an object and prints the resulting object id\n"
"--objectid specifies an objectid to use\n"
"--read prints the contents of specified object id\n"
"--update updates the contents of the specified object id with --data\n"
"--data the data for an update or create operation\n"
"--delete deletes the specified object from the Atmos storage system\n"	
"--systemmeta prints the system meta data associated with this object\n"
"--time/-T print timing information\n"
"--updatemeta key=value addes the specified key value pair to the object.  For more than 1 pair specify the option multiple times\n"    

""	    
);
    }

int main(int argc, char *argv[])
{
    int c;
    int digit_optind = 0;
    char *key=NULL, *user_id=NULL, *endpoint=NULL;
    acl *acl = NULL;
    user_meta *umd = NULL;
    int limit = 0;
    char *bytes = NULL;
    char object_id[OBJECTIDSIZE];
    enum atmos_cmd cmd = NONE;
    ws_result wsr;
    int capture_time = 0;
    memset(&umd, 0, sizeof(user_meta));

    while (1) {
	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"token",        1, 0, 't'},
	    {"secret",       1, 0, 's'},
	    {"host",         1, 0, 'H'},
	    {"help",         1, 0, 'h'},
	    {"create",       0, 0, 'c'},
	    {"objectid",     1, 0, 'o'},
	    {"read",         0, 0, 'r'},
	    {"update",       0, 0, 'u'},
	    {"data",         1, 0, 'i'},
	    {"delete",       0, 0, 'd'},
	    {"systemmeta",   0, 0, 'y'},
	    {"updatemeta",   1, 0, 'm'},
	    {"time",         0, 0, 'T'},
	    {0, 0, 0, 0}

	};

	c = getopt_long(argc, argv, "t:s:h:H:o:rcui:m:",long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 0:
	    printf("option %s", long_options[option_index].name);
	    if (optarg)
		printf(" with arg %s", optarg);
	    printf("\n");
	    break;

	case '0':
	case '1':
	case '2':
	    if (digit_optind != 0 && digit_optind != this_option_optind)
		printf("digits occur in two different argv-elements.\n");
	digit_optind = this_option_optind;
	printf("option %c\n", c);
	break;
	case 'o':
	    strcpy(object_id, optarg);
	    break;
	case 'H':
	    endpoint =optarg;
	    break;
	case 's':
	    key =optarg;
	    break;
	case 't':
	    user_id=optarg;
	    break;
	case 'h':
	    printf("help!\n");
	    create_usage();
	    break;
	case 'c':
	    cmd=CREATE;
	    break;
	case 'r':
	    cmd = READ;
	    break;
	case 'u':
	    cmd = UPDATE;
	    break;
	case 'i':
	    bytes=optarg;
	    break;
	case 'd':
	    cmd=DELETE;
	    break;
	case 'y':
	    cmd=SYSMETA;
	    break;
	case 'm':
	    cmd=UPDATEMETA;
	    int pos = 0;
	    int found = 0;
		
	    for(pos; pos < strlen(optarg); pos++) {
		if(optarg[pos] == '=') {
		    char key[1024];
		    char value[1024];
		    memset(key, 0,1024);
		    memset(value, 0,1024);
		    int listable = 0;
		    strncpy(key, optarg, pos);
		    strncpy(value, optarg+pos+1, strlen(optarg)-pos);		    
		    add_user_meta(&umd, key, value, listable);		    
		    found = 1;
		} 
	    }
	    if(found != 1) {
		printf("malformed key=value pair\n") ;
		exit(EXIT_FAILURE);
	   } 
	    break;
	case 'T':
	    capture_time = 1;
	    break;
	case '?':
	    break;
	default:
	    printf("Invalid entry.\n");
	    create_usage();
	}
    }

    if (optind < argc) {
	printf("non-option ARGV-elements: ");
	while (optind < argc)
	    printf("%s ", argv[optind++]);
	printf("\n");
    }

    if(user_id == NULL || key == NULL || endpoint == NULL) { 
	create_usage();
	exit(EXIT_SUCCESS);
    }
  

    credentials *creds = init_ws(user_id, key, endpoint);
    do_atmos_action(cmd, creds, object_id, bytes, acl, umd, limit, &wsr);
    if(capture_time)  {
	printf("%ds:%dms", wsr.duration_sec, wsr.duration_ms);
    }
    
    free(creds);
    free_user_meta(umd);
    return 0;
}


void do_atmos_action(enum atmos_cmd cmd, credentials *creds,char *object_id, char* bytes, acl *acl, user_meta* umd, int limit, ws_result *wsr) 
{


    switch(cmd) {
    case CREATE:
	create_action_cmdline(creds, NULL, NULL, NULL, NULL, wsr);
	break;
    case READ:
	read_action_cmdline(creds, object_id, limit, wsr);
	break;
    case UPDATE:
	update_action_cmdline(creds, object_id, bytes, NULL, acl, umd,wsr);
	break;
    case DELETE:
	delete_action_cmdline(creds, object_id, wsr);
	break;
    case SYSMETA:
	sysmeta_action_cmdline(creds, object_id, wsr);
	break;
    case UPDATEMETA:
	update_action_cmdline(creds, object_id, bytes, NULL, acl, umd, wsr);
	break;
    default:
	printf("how did you get here?");
    }
}

void create_action_cmdline(credentials *creds, char *data, char *content_type, acl *acl, user_meta *meta, ws_result *wsr) 
{
    char object_id[OBJECTIDSIZE];
    create_obj(creds,object_id, NULL, NULL, NULL, wsr);
    printf("created object %s\n", object_id);
}

void read_action_cmdline(credentials *creds, char *object_id, int limit, ws_result *wsr) 
{
    postdata read_data;
    memset(&read_data, 0, sizeof(postdata));
    read_obj(creds, object_id, &read_data, 0, wsr);
    char *bytes_read= malloc(wsr->body_size+1);
    if(wsr->body_size > 0)  {
	memcpy(bytes_read, wsr->response_body,wsr->body_size);
	bytes_read[wsr->body_size] = '\0';
	printf("%d\n", wsr->body_size);
	printf("%s\n", bytes_read);
    } 
    free(bytes_read);
} 
void sysmeta_action_cmdline(credentials *creds, char *object_id, ws_result *wsr) 
{

    user_meta *um = NULL;
    system_meta sm ;
    memset(&sm, 0, sizeof(system_meta));
    read_obj(creds, object_id, NULL, 0, wsr);
    parse_headers(wsr, &sm, &um);    
    print_system_meta(sm);
    if(um) free(um);
} 

void update_action_cmdline(credentials *creds, char *object_id, char *data, char *content_type, acl *acl, user_meta *meta, ws_result *wsr) 
{
    postdata update_data;
    memset(&update_data, 0, sizeof(postdata));
    
    if(data) {
	update_data.data = data;
	update_data.body_size = strlen(data);
    } 
    
    update_obj(creds, object_id, content_type,acl, &update_data,meta,  wsr);
    printf("%d\n", wsr->return_code);
}

void delete_action_cmdline(credentials *creds, char *object_id, ws_result *wsr) 
{

    delete_obj(creds, object_id, wsr);
    if(wsr->return_code == 204) 
	printf("Delete of %s succeeded\n", object_id);
    else
	printf("Failed %d\n", wsr->return_code);
}
