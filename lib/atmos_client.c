/*
 * atmos_client.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#include <string.h>
#include <stdlib.h>

#include "atmos.h"
#include "atmos_private.h"
#include "atmos_util.h"

static const char *methods[] = { "POST", "GET", "PUT", "DELETE", "HEAD",
        "OPTIONS" };

RestFilter *AtmosClient_add_default_filters(AtmosClient *self,
        RestFilter *chain) {

    chain = RestFilter_add(chain, RestFilter_execute_curl_request);
    chain = RestFilter_add(chain, AtmosFilter_sign_request);
    chain = RestFilter_add(chain, RestFilter_set_content_headers);
    chain = RestFilter_add(chain, AtmosFilter_add_uid);
    chain = RestFilter_add(chain, AtmosFilter_add_date);
    chain = RestFilter_add(chain, AtmosFilter_parse_atmos_error);

    return chain;
}

//Needs to be free*d
char *AtmosClient_sign(AtmosClient *self, const char *hash_string) {
    if (self->signature_debug) {
        fprintf(stderr, "String to Sign: %s\n", hash_string);
        fprintf(stderr, "With key: %s\n", self->secret);
    }

    char *sig = AtmosUtil_HMACSHA1(hash_string, self->secret,
            strlen(self->secret));

    if (self->signature_debug) {
        fprintf(stderr, "Signature: %s\n", sig);
    }

    return sig;
}

char *AtmosClient_sign_request(AtmosClient *self, RestRequest *request) {
    char *string_to_sign = AtmosClient_canonicalize_request(self, request);
    return AtmosClient_sign(self, string_to_sign);
}

char *AtmosClient_canonicalize_request(AtmosClient *self, RestRequest *request) {
    char *hash_string = NULL;
    size_t hash_size = 0;
    int firstheader;
    int i;
    char *loweruri;
    char **emc_sorted_headers;
    const char *header;

    // Lowercase, sort, and normalize headers.
    emc_sorted_headers = malloc(sizeof(char*) * (request->header_count));
    for (i = 0; i < request->header_count; i++) {
        emc_sorted_headers[i] = strdup(request->headers[i]);
        AtmosUtil_lowercaseheader(emc_sorted_headers[i]);
        AtmosUtil_normalize_whitespace(emc_sorted_headers[i]);
    }
    qsort(emc_sorted_headers, request->header_count, sizeof(char*),
            AtmosUtil_cstring_cmp);

    // Verb
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size,
            methods[request->method]);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, "\n");

    // Content-Type
    header = RestRequest_get_header_value(request, HTTP_HEADER_CONTENT_TYPE);
    if (header) {
        hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, header);
    }
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, "\n");

    // Range
    header = RestRequest_get_header_value(request, HTTP_HEADER_RANGE);
    if (header) {
        hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, header);
    }
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, "\n");

    // Date
    header = RestRequest_get_header_value(request, HTTP_HEADER_DATE);
    if (header) {
        hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, header);
    }
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, "\n");

    // Resource
    loweruri = strdup(request->uri);
    AtmosUtil_lowercase(loweruri);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, loweruri);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, "\n");
    free(loweruri);

    // Headers
    firstheader = 1;
    for (i = 0; i < request->header_count; i++) {
        // Only include x-emc headers
        if (strcasestr(emc_sorted_headers[i], "x-emc") == emc_sorted_headers[i]) {
            if (!firstheader) {
                hash_string = AtmosUtil_cstring_append(hash_string, &hash_size,
                        "\n");
            } else {
                firstheader = 0;
            }
            hash_string = AtmosUtil_cstring_append(hash_string, &hash_size,
                    emc_sorted_headers[i]);
        }
    }

    // Cleanup
    for (i = 0; i < request->header_count; i++) {
        free(emc_sorted_headers[i]);
    }

    return hash_string;
}

/********************
 * Public Functions *
 ********************/

AtmosClient*
AtmosClient_init(AtmosClient *self, const char *host, int port,
        const char *uid, const char *secret) {
    RestClient_init((RestClient*) self, host, port);
    memset(((void*)self)+sizeof(RestClient), 0, sizeof(AtmosClient) - sizeof(RestClient));

    ((Object*) self)->class_name = CLASS_ATMOS_CLIENT;

    strncpy(self->uid, uid, 255);
    strncpy(self->secret, secret, 64);

    return self;
}

void AtmosClient_destroy(AtmosClient *self) {
    memset(self->uid, 0, 255);
    memset(self->secret, 0, 64);

    RestClient_destroy((RestClient*) self);
}

void AtmosClient_get_service_information(AtmosClient *self,
        AtmosServiceInfoResponse *response) {
    RestRequest req;
    RestFilter *chain = NULL;

    RestRequest_init(&req, "/rest/service", HTTP_GET);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_service_info_response);

    RestClient_execute_request((RestClient*) self, chain, &req,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&req);
}

void AtmosClient_create_object(AtmosClient *self,
        AtmosCreateObjectRequest *request, AtmosCreateObjectResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_create_response);
    chain = RestFilter_add(chain, AtmosFilter_set_create_headers);

    // Special case -- if there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    if (!((RestRequest*)request)->request_body) {
        RestRequest_add_header((RestRequest*)request,
                HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    }

    RestClient_execute_request((RestClient*) self, chain,
            (RestRequest*) request, (RestResponse*) response);

    RestFilter_free(chain);
}

void AtmosClient_create_object_simple(AtmosClient *self, const char *data,
        size_t data_size, const char *content_type,
        AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init(&request);
    if (data) {
        RestRequest_set_array_body((RestRequest*) &request, data, data_size,
                content_type);
    }

    AtmosClient_create_object(self, &request, response);

    AtmosCreateObjectRequest_destroy(&request);
}

void AtmosClient_create_object_simple_ns(AtmosClient *self, const char *path,
        const char *data, size_t data_size, const char *content_type,
        AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init_ns(&request, path);
    if (data) {
        RestRequest_set_array_body((RestRequest*) &request, data, data_size,
                content_type);
    }

    AtmosClient_create_object(self, &request, response);

    AtmosCreateObjectRequest_destroy(&request);
}

void AtmosClient_create_object_file(AtmosClient *self, FILE *f,
        off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init(&request);
    if (f) {
        RestRequest_set_file_body((RestRequest*) &request, f, content_length,
                content_type);
    }

    AtmosClient_create_object(self, &request, response);

    AtmosCreateObjectRequest_destroy(&request);
}

void AtmosClient_create_object_file_ns(AtmosClient *self, const char *path,
        FILE *f, off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init_ns(&request, path);
    if (f) {
        RestRequest_set_file_body((RestRequest*) &request, f, content_length,
                content_type);
    }

    AtmosClient_create_object(self, &request, response);

    AtmosCreateObjectRequest_destroy(&request);
}

void AtmosClient_delete_object(AtmosClient *self, const char *object_id,
        RestResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s", object_id);

    RestRequest_init(&request, uri, HTTP_DELETE);

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void AtmosClient_delete_object_ns(AtmosClient *self, const char *path,
        RestResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/namespace%s", path);

    RestRequest_init(&request, uri, HTTP_DELETE);

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_read_object(AtmosClient *self, AtmosReadObjectRequest *request,
        AtmosReadObjectResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_read_object_headers);
    chain = RestFilter_add(chain, AtmosFilter_parse_read_object_response);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_read_object_simple(AtmosClient *self, const char *object_id,
        AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init(&request, object_id);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_read_object_simple_ns(AtmosClient *self, const char *path,
        AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init_ns(&request, path);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_delete_user_meta(AtmosClient *self, const char *object_id,
        const char const **meta_names, int meta_name_count, RestResponse *response) {
    char uri[ATMOS_OID_LENGTH+64];
    RestFilter *chain = NULL;
    RestRequest request;

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/user", object_id);
    RestRequest_init(&request, uri, HTTP_DELETE);

    // Set the headers
    if(meta_names && meta_name_count > 0) {
        AtmosUtil_set_tags_header2(&request, meta_names, meta_name_count);
    }

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}

void
AtmosClient_delete_user_meta_ns(AtmosClient *self, const char *path,
        const char const **meta_names, int meta_name_count,
        RestResponse *response) {
    char uri[ATMOS_PATH_MAX+64];
    RestFilter *chain = NULL;
    RestRequest request;

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", path);
    RestRequest_init(&request, uri, HTTP_DELETE);

    // Set the headers
    if(meta_names && meta_name_count > 0) {
        AtmosUtil_set_tags_header2(&request, meta_names, meta_name_count);
    }

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);

}

void
AtmosClient_get_user_meta(AtmosClient *self, AtmosGetUserMetaRequest *request,
        AtmosGetUserMetaResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_get_user_meta_headers);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_user_meta_response);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_get_user_meta_simple(AtmosClient *self, const char *object_id,
        AtmosGetUserMetaResponse *response) {
    AtmosGetUserMetaRequest request;

    AtmosGetUserMetaRequest_init(&request, object_id);

    AtmosClient_get_user_meta(self, &request, response);

    AtmosGetUserMetaRequest_destroy(&request);
}

void
AtmosClient_get_user_meta_simple_ns(AtmosClient *self, const char *path,
        AtmosGetUserMetaResponse *response) {
    AtmosGetUserMetaRequest request;

    AtmosGetUserMetaRequest_init_ns(&request, path);

    AtmosClient_get_user_meta(self, &request, response);

    AtmosGetUserMetaRequest_destroy(&request);

}
