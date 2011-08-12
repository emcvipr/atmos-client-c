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

enum {
    CREATE =0,
    READ,
    UPDATE,
    DELETE,
    SHARE} atmos_cmd;


void create_usage() {
    fprintf(stderr, "Usage:\n--help Displays this message\n--token Atmos userid\n--secret Atmos shared secret\n--host Atmos accesspoint\n--create Creates an object and prints the resulting object id\n");
}

int main(int argc, char *argv[])
{
    int c;
    int digit_optind = 0;
    char *key=NULL, *user_id=NULL, *endpoint=NULL;

    //atmos_cmd action=NULL;
    while (1) {
	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"token",  1, 0, 't'},
	    {"secret", 1, 0, 's'},
	    {"host",   1, 0, 'o'},
	    {"help",   1, 0, 'h'},
	    {"create", 0, 0, 'c'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "t:s:h:o:c",long_options, &option_index);
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
	    endpoint =(char*)strdup(optarg);
	    break;
	case 's':
	    key =(char*)strdup(optarg);
	    break;
	case 't':
	    user_id=(char*)strdup(optarg);
	    break;
	case 'h':
	    printf("help!\n");
	    create_usage();
	    break;
	case 'c':
	    //action=CREATE;
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
    
    char objid[44];
    ws_result wsr;
    create_obj(creds,objid, NULL, NULL, NULL, &wsr);
    printf("return %d\tcreated object %s\n", wsr.return_code, objid);
    
    result_deinit(&wsr);
    return 0;
}
