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
        ATMOS_DEBUG("String to Sign: %s\n", hash_string);
        ATMOS_DEBUG("With key: %s\n", self->secret);
    }

    char *sig = AtmosUtil_HMACSHA1(hash_string, self->secret,
            strlen(self->secret));

    if (self->signature_debug) {
        ATMOS_DEBUG("Signature: %s\n", sig);
    }

    return sig;
}

char *AtmosClient_sign_request(AtmosClient *self, RestRequest *request) {
    char *string_to_sign = AtmosClient_canonicalize_request(self, request);
    char *sig = AtmosClient_sign(self, string_to_sign);
    free(string_to_sign);
    return sig;
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
    } else {
        // another version
        header = RestRequest_get_header_value(request, HTTP_HEADER_CONTENT_RANGE);
        if(header) {
            hash_string = AtmosUtil_cstring_append(hash_string, &hash_size, header);
        }
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
        if (strstr(emc_sorted_headers[i], "x-emc") == emc_sorted_headers[i]) {
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
    free(emc_sorted_headers);

    return hash_string;
}

/********************
 * Public Functions *
 ********************/

AtmosClient*
AtmosClient_init(AtmosClient *self, const char *endpoint, int port,
        const char *uid, const char *secret) {
    RestClient_init((RestClient*) self, endpoint, port);
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
    chain = RestFilter_add(chain, AtmosFilter_set_write_headers);

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

void AtmosClient_create_object_simple_keypool(AtmosClient *self,
        const char *pool, const char *key, const char *data, size_t data_size,
        const char *content_type, AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init_keypool(&request, pool, key);
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

void AtmosClient_create_object_file_keypool(AtmosClient *self, const char *pool,
        const char *key, FILE *f, off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response) {
    AtmosCreateObjectRequest request;

    AtmosCreateObjectRequest_init_keypool(&request, pool, key);
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
    char uri[ATMOS_PATH_MAX+16];

    snprintf(uri, ATMOS_PATH_MAX+16, "/rest/namespace%s", path);

    RestRequest_init(&request, uri, HTTP_DELETE);

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void AtmosClient_delete_object_keypool(AtmosClient *self, const char *pool,
        const char *key, RestResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+16];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+16, "/rest/namespace/%s", key);

    RestRequest_init(&request, uri, HTTP_DELETE);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header(&request, poolheader);

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
AtmosClient_read_object_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init_keypool(&request, pool, key);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_head_object(AtmosClient *self, const char *object_id,
        AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init_head(&request, object_id);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_head_object_ns(AtmosClient *self, const char *path,
        AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init_ns_head(&request, path);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_head_object_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosReadObjectResponse *response) {
    AtmosReadObjectRequest request;

    AtmosReadObjectRequest_init_keypool_head(&request, pool, key);

    AtmosClient_read_object(self, &request, response);

    AtmosReadObjectRequest_destroy(&request);
}

void
AtmosClient_delete_user_meta(AtmosClient *self, const char *object_id,
        const char **meta_names, int meta_name_count, RestResponse *response) {
    char uri[ATMOS_OID_LENGTH+64];
    RestFilter *chain = NULL;
    RestRequest request;
    int utf8;

    utf8 = self->enable_utf8_metadata;

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/user", object_id);
    RestRequest_init(&request, uri, HTTP_DELETE);

    if(utf8) {
        RestRequest_add_header(&request, ATMOS_HEADER_UTF8 ": true");
    }

    // Set the headers
    if(meta_names && meta_name_count > 0) {
        AtmosUtil_set_tags_header2(&request, meta_names, meta_name_count, utf8);
    }

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}

void
AtmosClient_delete_user_meta_ns(AtmosClient *self, const char *path,
        const char **meta_names, int meta_name_count,
        RestResponse *response) {
    char uri[ATMOS_PATH_MAX+64];
    RestFilter *chain = NULL;
    RestRequest request;
    int utf8;

    utf8 = self->enable_utf8_metadata;

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", path);
    RestRequest_init(&request, uri, HTTP_DELETE);

    if(utf8) {
        RestRequest_add_header(&request, ATMOS_HEADER_UTF8 ": true");
    }

    // Set the headers
    if(meta_names && meta_name_count > 0) {
        AtmosUtil_set_tags_header2(&request, meta_names, meta_name_count, utf8);
    }

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);

}

void
AtmosClient_delete_user_meta_keypool(AtmosClient *self, const char *pool,
        const char *key, const char **meta_names, int meta_name_count,
        RestResponse *response) {
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];
    RestFilter *chain = NULL;
    RestRequest request;
    int utf8;

    utf8 = self->enable_utf8_metadata;

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", key);
    RestRequest_init(&request, uri, HTTP_DELETE);

    if(utf8) {
        RestRequest_add_header(&request, ATMOS_HEADER_UTF8 ": true");
    }

    // Set the headers
    if(meta_names && meta_name_count > 0) {
        AtmosUtil_set_tags_header2(&request, meta_names, meta_name_count, utf8);
    }

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header(&request, poolheader);

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

void
AtmosClient_get_user_meta_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetUserMetaResponse *response) {
    AtmosGetUserMetaRequest request;

    AtmosGetUserMetaRequest_init_keypool(&request, pool, key);

    AtmosClient_get_user_meta(self, &request, response);

    AtmosGetUserMetaRequest_destroy(&request);

}

void
AtmosClient_set_user_meta(AtmosClient *self, AtmosSetUserMetaRequest *request,
        AtmosResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_write_headers);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);

}

void
AtmosClient_get_system_meta(AtmosClient *self,
        AtmosGetSystemMetaRequest *request,
        AtmosGetSystemMetaResponse *response) {

    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_get_system_metadata);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);

}

void
AtmosClient_get_system_meta_simple(AtmosClient *self, const char *object_id,
        AtmosGetSystemMetaResponse *response) {
    AtmosGetSystemMetaRequest request;

    AtmosGetSystemMetaRequest_init(&request, object_id);

    AtmosClient_get_system_meta(self, &request, response);

    AtmosGetSystemMetaRequest_destroy(&request);
}

void
AtmosClient_get_system_meta_simple_ns(AtmosClient *self, const char *path,
        AtmosGetSystemMetaResponse *response) {
    AtmosGetSystemMetaRequest request;

    AtmosGetSystemMetaRequest_init_ns(&request, path);

    AtmosClient_get_system_meta(self, &request, response);

    AtmosGetSystemMetaRequest_destroy(&request);

}

void
AtmosClient_get_system_meta_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetSystemMetaResponse *response) {
    AtmosGetSystemMetaRequest request;

    AtmosGetSystemMetaRequest_init_keypool(&request, pool, key);

    AtmosClient_get_system_meta(self, &request, response);

    AtmosGetSystemMetaRequest_destroy(&request);

}

void
AtmosClient_get_acl(AtmosClient *self, const char *object_id,
        AtmosGetAclResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?acl", object_id);

    RestRequest_init(&request, uri, HTTP_GET);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_get_acl_ns(AtmosClient *self, const char *path,
        AtmosGetAclResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?acl", path);

    RestRequest_init(&request, uri, HTTP_GET);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_get_acl_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetAclResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?acl", key);

    RestRequest_init(&request, uri, HTTP_GET);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header(&request, poolheader);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_set_acl(AtmosClient *self, const char *object_id,
        AtmosAclEntry *acl, int acl_count, AtmosResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?acl", object_id);

    RestRequest_init(&request, uri, HTTP_POST);
    // since there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    RestRequest_add_header((RestRequest*)&request,
            HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    AtmosUtil_set_acl_header(acl, acl_count, &request);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_set_acl_ns(AtmosClient *self, const char *path,
        AtmosAclEntry *acl, int acl_count, AtmosResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?acl", path);

    RestRequest_init(&request, uri, HTTP_POST);
    // since there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    RestRequest_add_header((RestRequest*)&request,
            HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    AtmosUtil_set_acl_header(acl, acl_count, &request);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_set_acl_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosAclEntry *acl, int acl_count,
        AtmosResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?acl", key);

    RestRequest_init(&request, uri, HTTP_POST);
    // since there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    RestRequest_add_header((RestRequest*)&request,
            HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    AtmosUtil_set_acl_header(acl, acl_count, &request);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header(&request, poolheader);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_acl_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_update_object(AtmosClient *self, AtmosUpdateObjectRequest *request,
        RestResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_write_headers);
    chain = RestFilter_add(chain, AtmosFilter_update_object);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_update_object_simple(AtmosClient *self, const char *object_id,
        const char *data, size_t data_size, const char *content_type,
        RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init(&request, object_id);

    if(data && data_size > 0) {
        RestRequest_set_array_body((RestRequest*)&request, data, data_size,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);
}

void
AtmosClient_update_object_simple_ns(AtmosClient *self, const char *path,
        const char *data, size_t data_size, const char *content_type,
        RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init_ns(&request, path);

    if(data && data_size > 0) {
        RestRequest_set_array_body((RestRequest*)&request, data, data_size,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);

}

void
AtmosClient_update_object_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, const char *data, size_t data_size,
        const char *content_type, RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init_keypool(&request, pool, key);

    if(data && data_size > 0) {
        RestRequest_set_array_body((RestRequest*)&request, data, data_size,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);
}

void
AtmosClient_update_object_file(AtmosClient *self, const char *object_id,
        FILE *f, off_t content_length, const char *content_type,
        RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init(&request, object_id);

    if(f && content_length > 0) {
        RestRequest_set_file_body((RestRequest*)&request, f, content_length,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);
}

void
AtmosClient_update_object_file_ns(AtmosClient *self, const char *path,
        FILE *f, off_t content_length, const char *content_type,
        RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init_ns(&request, path);

    if(f && content_length > 0) {
        RestRequest_set_file_body((RestRequest*)&request, f, content_length,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);
}

void
AtmosClient_update_object_file_keypool(AtmosClient *self, const char *pool,
        const char *key, FILE *f, off_t content_length,
        const char *content_type, RestResponse *response) {
    AtmosUpdateObjectRequest request;

    AtmosUpdateObjectRequest_init_keypool(&request, pool, key);

    if(f && content_length > 0) {
        RestRequest_set_file_body((RestRequest*)&request, f, content_length,
                content_type);
    }

    AtmosClient_update_object(self, &request, response);

    AtmosUpdateObjectRequest_destroy(&request);
}

void
AtmosClient_get_object_info(AtmosClient *self, const char *object_id,
        AtmosGetObjectInfoResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?info", object_id);

    RestRequest_init(&request, uri, HTTP_GET);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_info_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_get_object_info_ns(AtmosClient *self, const char *path,
        AtmosGetObjectInfoResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?info", path);

    RestRequest_init(&request, uri, HTTP_GET);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_info_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_get_object_info_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetObjectInfoResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?info", key);

    RestRequest_init(&request, uri, HTTP_GET);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header(&request, poolheader);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_info_response);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}

void
AtmosClient_rename_object(AtmosClient *self, const char *source,
        const char *destination, int force, AtmosResponse *response) {
    RestRequest request;
    RestFilter *chain = NULL;
    char uri[ATMOS_PATH_MAX+32];
    char rename_header[ATMOS_PATH_MAX+32];

    snprintf(uri, ATMOS_PATH_MAX+32, "/rest/namespace%s?rename", source);

    RestRequest_init(&request, uri, HTTP_POST);

    // Add the destination
    snprintf(rename_header, ATMOS_PATH_MAX+32, "%s: %s", ATMOS_HEADER_PATH,
            destination);
    RestRequest_add_header(&request, rename_header);

    if(force) {
        RestRequest_add_header(&request, ATMOS_HEADER_FORCE ": true");
    }

    // Since there's no body, force a content type
    RestRequest_add_header(&request, HTTP_HEADER_CONTENT_TYPE
            ": application/octet-stream");

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*) self, chain, &request,
            (RestResponse*) response);

    RestFilter_free(chain);

    RestRequest_destroy(&request);
}


void
AtmosClient_list_directory(AtmosClient *self,
        AtmosListDirectoryRequest *request,
        AtmosListDirectoryResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    // Reuse the read_object_respnose to parse the directory-level metadata,
    // ACL, etc.
    chain = RestFilter_add(chain, AtmosFilter_parse_read_object_response);
    chain = RestFilter_add(chain, AtmosFilter_list_directory);
    chain = RestFilter_add(chain, AtmosFilter_set_pagination_headers);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_get_listable_tags(AtmosClient *self,
        AtmosGetListableTagsRequest *request,
        AtmosGetListableTagsResponse *response) {

    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_pagination_headers);
    chain = RestFilter_add(chain, AtmosFilter_get_listable_tags);


    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_list_objects(AtmosClient *self, AtmosListObjectsRequest *request,
        AtmosListObjectsResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_list_objects);
    chain = RestFilter_add(chain, AtmosFilter_set_pagination_headers);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}


void
AtmosClient_create_version(AtmosClient *self, const char *object_id,
        AtmosCreateVersionResponse *response) {
    AtmosCreateVersionRequest request;
    RestFilter *chain = NULL;

    AtmosCreateVersionRequest_init(&request, object_id);

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_create_version_response);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)&request, (RestResponse*)response);

    AtmosCreateVersionRequest_destroy(&request);

    RestFilter_free(chain);
}

void
AtmosClient_list_versions(AtmosClient *self, AtmosListVersionsRequest *request,
        AtmosListVersionsResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_list_versions);
    chain = RestFilter_add(chain, AtmosFilter_set_pagination_headers);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);
}

void
AtmosClient_delete_version(AtmosClient *self, const char *version_id,
        RestResponse *response) {
    char uri[ATMOS_PATH_MAX];
    RestRequest request;
    RestFilter *chain = NULL;

    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s?versions", version_id);
    RestRequest_init(&request, uri, HTTP_DELETE);

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}

void
AtmosClient_restore_version(AtmosClient *self, const char *object_id,
        const char *version_id, RestResponse *response) {
    char uri[ATMOS_PATH_MAX];
    char vid_header[ATMOS_SIMPLE_HEADER_MAX];

    RestRequest request;
    RestFilter *chain = NULL;

    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s?versions", object_id);
    RestRequest_init(&request, uri, HTTP_PUT);

    snprintf(vid_header, ATMOS_SIMPLE_HEADER_MAX,
            ATMOS_HEADER_VERSION_OID ": %s", version_id);
    RestRequest_add_header(&request, vid_header);

    // Since this is a PUT with no body, curl gets a bit confused if we don't
    // give it something to read.
    RestRequest_set_array_body(&request, "", 0, "application/octet-stream");

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}


void
AtmosClient_create_access_token(AtmosClient *self,
        AtmosCreateAccessTokenRequest *request,
        AtmosCreateAccessTokenResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_create_access_token);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);

}


void
AtmosClient_delete_access_token(AtmosClient *self, const char *token_id,
        RestResponse *response) {
    char uri[ATMOS_PATH_MAX];
    RestRequest request;
    RestFilter *chain = NULL;

    snprintf(uri, ATMOS_PATH_MAX, "/rest/accesstokens/%s", token_id);
    RestRequest_init(&request, uri, HTTP_DELETE);
    request.uri_encoded = 1;

    chain = AtmosClient_add_default_filters(self, chain);

    RestClient_execute_request((RestClient*)self, chain,
            &request, response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}


void
AtmosClient_list_access_tokens(AtmosClient *self,
        AtmosListAccessTokensRequest *request,
        AtmosListAccessTokensResponse *response) {
    RestFilter *chain = NULL;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_set_pagination_headers);
    chain = RestFilter_add(chain, AtmosFilter_parse_list_access_tokens_response);

    RestClient_execute_request((RestClient*)self, chain,
            (RestRequest*)request, (RestResponse*)response);

    RestFilter_free(chain);

}


void
AtmosClient_get_access_token_info(AtmosClient *self, const char *token_id,
        AtmosGetAccessTokenInfoResponse *response) {
    char uri[ATMOS_PATH_MAX];
    RestRequest request;
    RestFilter *chain = NULL;

    snprintf(uri, ATMOS_PATH_MAX, "/rest/accesstokens/%s?info", token_id);
    RestRequest_init(&request, uri, HTTP_GET);
    request.uri_encoded = 1;

    chain = AtmosClient_add_default_filters(self, chain);
    chain = RestFilter_add(chain, AtmosFilter_parse_get_token_info_response);

    RestClient_execute_request((RestClient*)self, chain,
            &request, (RestResponse*)response);

    RestFilter_free(chain);
    RestRequest_destroy(&request);
}

char *
AtmosClient_get_shareable_url_internal(AtmosClient *self, const char *uri,
        time_t expires, const char *disposition) {
    CURL *curl;
    char *hash_string = NULL;
    char *signature;
    size_t hash_string_sz = 0;
    char expires_str[ATMOS_SIMPLE_HEADER_MAX];
    char *url = NULL;
    size_t url_sz = 0;
    char *uri_lower;

    curl = curl_easy_init();
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            "GET\n");

    uri_lower = strdup(uri);
    AtmosUtil_lowercase(uri_lower);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            uri_lower);
    free(uri_lower);

    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            "\n");

    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            self->uid);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            "\n");

    snprintf(expires_str, ATMOS_SIMPLE_HEADER_MAX, "%lld",
            (long long)expires);
    hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
            expires_str);

    if(disposition) {
        hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
                "\n");
        hash_string = AtmosUtil_cstring_append(hash_string, &hash_string_sz,
                disposition);
    }

    signature = AtmosClient_sign(self, hash_string);

    // Got the signature, now build the URL.
    url = AtmosUtil_cstring_append(url, &url_sz, self->parent.host);
    if(self->parent.port != -1) {
        char portbuf[ATMOS_SIMPLE_HEADER_MAX];
        snprintf(portbuf, ATMOS_SIMPLE_HEADER_MAX, ":%d", self->parent.port);
        url = AtmosUtil_cstring_append(url, &url_sz, portbuf);
    }
    url = AtmosUtil_cstring_append(url, &url_sz, uri);
    url = AtmosUtil_cstring_append(url, &url_sz, "?uid=");
    url = AtmosUtil_cstring_append_utf8(url, &url_sz,
            self->uid, curl);
    url = AtmosUtil_cstring_append(url, &url_sz, "&expires=");
    url = AtmosUtil_cstring_append(url, &url_sz, expires_str);
    url = AtmosUtil_cstring_append(url, &url_sz, "&signature=");
    url = AtmosUtil_cstring_append_utf8(url, &url_sz, signature, curl);

    if(disposition) {
        url = AtmosUtil_cstring_append(url, &url_sz, "&disposition=");
        url = AtmosUtil_cstring_append_utf8(url, &url_sz, disposition, curl);
    }

    free(signature);
    free(hash_string);
    curl_easy_cleanup(curl);

    return url;
}

char *
AtmosClient_get_shareable_url(AtmosClient *self, const char *object_id,
        time_t expires, const char *disposition) {
    char uri[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s", object_id);

    return AtmosClient_get_shareable_url_internal(self, uri, expires,
            disposition);
}

char *
AtmosClient_get_shareable_url_ns(AtmosClient *self, const char *path,
        time_t expires, const char *disposition) {
    char uri[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/namespace%s", path);

    return AtmosClient_get_shareable_url_internal(self, uri, expires,
            disposition);
}

