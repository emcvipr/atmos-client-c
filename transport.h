#ifndef _AOL_TRANSPORT_H
#define _AOL_TRANSPORT_H

/**
 * @file transport.h
 */

#include <stdio.h> /*FILE*/
#include <unistd.h> /*off_t*/
#include "curl/curl.h"
#include "atmos_util.h"

#define CREDS_SIZE 255
typedef struct credentialsval {
    char tokenid[CREDS_SIZE];
    char secret[CREDS_SIZE];
    char accesspoint[CREDS_SIZE];
    int curl_verbose;
} credentials;

/**
 * Contains request data to be sent to the server.  Note that data and
 * data_stream are mutually exclusive.
 */
typedef struct {
	/** @{ */
    char *data; /** Character data to post */
    FILE *data_stream; /** Stream data to post */
    /** @} */
    off_t body_size; /** Size of data or data_stream.  Required. */
    off_t bytes_remaining; /** Bytes remaining to send. */
    off_t offset; /** Offset inside the Atmos object.  Set to -1 to truncate */
    off_t bytes_written; /** ? */
}postdata;

typedef struct {
    void *header_data;
    size_t header_size;
    void *next_header;
} hdr;

#define MAX_HEADERS 1024

typedef struct {
    long return_code;
    char *response_body;
    size_t body_size;
    char *headers[MAX_HEADERS];
    int header_count;
    int duration_ms;
    int duration_sec;
    char *content_type;
    int curl_error_code; /** If return_code is zero, you can check this to get the underlying error from cURL **/
    char curl_error_message[CURL_ERROR_SIZE];
} ws_result;


#define true 1
#define false 0

#define HTTP_HDR_SIZE 4096

const char *http_request(credentials *c,http_method method, const char *uri,
		const char * content_type, char **headers, int header_count, postdata* a,
		ws_result* ws_result);

const char *http_request_ns(credentials *c,http_method method, const char *uri,
		const char *content_type, char **headers, int header_count, postdata* a,
		ws_result *ws_result);

void result_deinit(ws_result *result);
void result_init(ws_result *result);
void print_ws_result(ws_result *result);
#endif

