#ifndef _AOL_TRANSPORT_H
#define _AOL_TRANSPORT_H

#include "atmos_util.h"

#define CREDS_SIZE 255
typedef struct credentialsval {
    char tokenid[CREDS_SIZE];
    char secret[CREDS_SIZE];
    char accesspoint[CREDS_SIZE];
} credentials;


typedef struct PD {
    char *data;
    unsigned long long body_size;
    unsigned long long bytes_remaining;
    unsigned long long offset;
    unsigned long long bytes_written;
}postdata;

typedef struct hdrval {
    void *header_data;
    size_t header_size;
    void *next_header;
} hdr;

#define MAX_HEADERS 1024

typedef struct ws_result {
    int return_code;
    char *response_body;
    size_t body_size;
    char *headers[MAX_HEADERS];
    int header_count;
    int duration_ms;
    int duration_sec;
    char *content_type;
} ws_result;


#define true 1
#define false 0

#define HTTP_HDR_SIZE 4096
const char *http_request(credentials *c,http_method method, const char *uri,char * content_type, char **headers, int header_count, postdata* a, ws_result* ws_result) ;
const char *http_request_ns(credentials *c,http_method method, const char *uri, char *content_type, char **headers, int header_count, postdata* a, ws_result *ws_result) ;
void ws_init(ws_result*);
void ws_deinit(ws_result*);
void result_deinit(ws_result *result);
void result_init(ws_result *result);
void print_ws_result(ws_result *result);
#endif

