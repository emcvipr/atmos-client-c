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

void usage() 
{
    fprintf(stderr, "Usage:\n"
	    "--help Displays this message\n"
	    "--updatemeta key=value addes the specified key value pair to the object.  For more than 1 pair specify the option multiple times"    

	    ""	    
	    );
}

int main(int argc, char *argv[])
{
    int c;
    int digit_optind = 0;
    char *key=NULL, *user_id=NULL, *endpoint=NULL;
    acl *acl = NULL;
    user_meta *umd=NULL;// = malloc(sizeof(user_meta));
    //memset(umd, 0, sizeof(user_meta));
    user_meta *current_umd = NULL;
    int limit = 0;
    char *bytes = NULL;
    char object_id[OBJECTIDSIZE];

    while (1) {
	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"updatemeta",   1, 0, 'm'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "m:",long_options, &option_index);
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
	case 'm':
	    printf("");
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
	case '?':
	    break;
	default:
	    printf("Invalid entry.\n");
	    usage();
	}
    }

    if (optind < argc) {
	printf("non-option ARGV-elements: ");
	while (optind < argc)
	    printf("%s ", argv[optind++]);
	printf("\n");
    }

    char *meta_listable_header=NULL;
    char *meta_header=NULL;
    add_meta_headers(&meta_listable_header, &meta_header, umd);
    printf("listable meta header is %s\n", meta_listable_header);
    printf("non listable meta header is %s\n", meta_header);

    //free_user_meta(umd);
}

