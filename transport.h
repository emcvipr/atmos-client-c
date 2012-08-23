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
    char *proxy_host; // NULL = disabled
    int proxy_port; // -1 = default
    char *proxy_user; // NULL = no auth
    char *proxy_pass;
    void *curlconfig; // Pointer to curl_config_callback function.
    void *retryhandler; // Pointer to curl_retry_callback function.
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

/**
 * Handler callback to determine whether a failed request should be retried.
 * The handler should return nonzero if a retry should be performed.  Note that
 * the handler should also see if a FILE* was used in the postdata structure
 * and reset that to an appropriate offset (e.g. seek back to zero).  The other
 * fields like bytes_remaining and bytes_written will be reset automatically.
 * @param c The Atmos credentials object.
 * @param data the Data (if any) that was POSTed to the server.
 */
typedef int (*curl_retry_callback)(credentials *c, postdata *data,
		ws_result *result);


/**
 * Handler callback to perform some sort of configuration on a request before
 * it's executed (e.g. set custom headers, authenticate, etc).  Note that a
 * handler may be called by multiple threads concurrently so it should be
 * thread-safe.
 * @param c the pointer to the credentials object.
 * @param handle the handle to the current cURL connection object.
 * @return zero to continue processing. Return nonzero to abort processing
 * the request.
 */
typedef int (*curl_config_callback)(credentials *c, CURL *handle);


#define true 1
#define false 0

#define HTTP_HDR_SIZE 4096

void http_request(credentials *c,http_method method, const char *uri,
		const char * content_type, char **headers, int header_count, postdata* a,
		ws_result* ws_result);

void http_request_ns(credentials *c,http_method method, const char *uri,
		const char *content_type, char **headers, int header_count, postdata* a,
		ws_result *ws_result);

void result_deinit(ws_result *result);
void result_init(ws_result *result);
void print_ws_result(ws_result *result);

/**
 * Sets a configuration callback to invoke just before we send a request using
 * cURL.  This can be used to change any settings on the cURL handle like
 * debugging, certificate validation, etc.
 * @param c the Atmos credentials object
 * @param func the configuration callback function
 */
void set_config_callback(credentials *c, curl_config_callback func);

/**
 * Sets a callback to invoke in case there is an error invoking an HTTP
 * request.  This handler will get called whenever the cURL request fails
 * (e.g. could not connect) or if the HTTP response code is greater than 299.
 * If it returns nonzero, the request will be retried.  The handler function
 * is also responsible for resetting the state of any postdata such that the
 * request can be sent again.
 */
void set_retry_callback(credentials *c, curl_retry_callback func);

#endif

