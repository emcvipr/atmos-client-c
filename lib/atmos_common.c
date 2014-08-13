/*
 * atmos_common.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */
#include "atmos.h"
#include "atmos_util.h"

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <inttypes.h>

const char *ATMOS_SYSTEM_META_NAMES[] = {ATMOS_SYSTEM_META_ATIME,
ATMOS_SYSTEM_META_CTIME, ATMOS_SYSTEM_META_GID, ATMOS_SYSTEM_META_ITIME,
ATMOS_SYSTEM_META_MTIME, ATMOS_SYSTEM_META_NLINK, ATMOS_SYSTEM_META_OBJECTID,
ATMOS_SYSTEM_META_OBJNAME, ATMOS_SYSTEM_META_POLICYNAME, ATMOS_SYSTEM_META_SIZE,
ATMOS_SYSTEM_META_TYPE, ATMOS_SYSTEM_META_UID, ATMOS_SYSTEM_META_WSCHECKSUM};
const int ATMOS_SYSTEM_META_NAME_COUNT = 13;

AtmosResponse*
AtmosResponse_init(AtmosResponse *self) {
    RestResponse_init((RestResponse*)self);
    self->atmos_error = 0;
    self->atmos_error_message[0] = 0;

    return self;
}

void
AtmosResponse_destroy(AtmosResponse *self) {
    self->atmos_error = 0;
    self->atmos_error_message[0] = 0;
    RestResponse_destroy((RestResponse*)self);
}

void
AtmosResponse_set_api_error(AtmosResponse *self, int code, const char *message) {
    self->parent.http_code = 400;
    strcpy(self->parent.http_status, "Bad Request");
    self->atmos_error = code;
    strcpy(self->atmos_error_message, message);
}


void
AtmosWriteObjectRequest_add_metadata(AtmosWriteObjectRequest *self,
        const char *name, const char *value,
        int listable) {
    if(listable) {
        int i = self->listable_meta_count++;
        strncpy(self->listable_meta[i].name, name, ATMOS_META_NAME_MAX);
        strncpy(self->listable_meta[i].value, value, ATMOS_META_VALUE_MAX);
    } else {
        int i = self->meta_count++;
        strncpy(self->meta[i].name, name, ATMOS_META_NAME_MAX);
        strncpy(self->meta[i].value, value, ATMOS_META_VALUE_MAX);
    }
}

void
AtmosWriteObjectRequest_add_acl(AtmosWriteObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission) {
    int i = self->acl_count++;
    self->acl[i].permission = permission;
    self->acl[i].type = type;
    strcpy(self->acl[i].principal, principal);
}

int AtmosWriteObjectRequest_checksum_filter(RestRequest *request, char *data,
        size_t data_size) {
    AtmosWriteObjectRequest *req = (AtmosWriteObjectRequest*)request;
    if(req->checksum) {
        AtmosChecksum_update(req->checksum, data, data_size);
    }

    return 1;
}


void AtmosFilter_set_write_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {

    AtmosWriteObjectRequest *req = (AtmosWriteObjectRequest*)request;
    AtmosClient *atmos = (AtmosClient*)rest;

    // Special case -- if there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    if (!((RestRequest*)request)->request_body) {
        RestRequest_add_header((RestRequest*)request,
                HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    }

    if(req->meta_count > 0) {
        AtmosUtil_set_metadata_header(req->meta, req->meta_count, 0,
                atmos->enable_utf8_metadata, request);
    }

    if(req->listable_meta_count > 0) {
        AtmosUtil_set_metadata_header(req->listable_meta,
                req->listable_meta_count, 1,
                atmos->enable_utf8_metadata, request);
    }

    if(atmos->enable_utf8_metadata) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    if(req->acl_count > 0) {
        AtmosUtil_set_acl_header(req->acl, req->acl_count, request);
    }

    /**
     * See if wschecksum was requested.
     */
    if(req->checksum && req->checksum->mode == ATMOS_WSCHECKSUM &&
            RestRequest_get_header_value(request, ATMOS_HEADER_WSCHECKSUM) == NULL) {
        char *value;
        char buffer[ATMOS_SIMPLE_HEADER_MAX];

        // We need to checksum the data before sending.
        if(request->request_body) {
            if(request->request_body->file_body) {
                FILE *f = request->request_body->file_body;
                off_t cur = ftello(f);
                if(cur == -1) {
                    // Error getting file offset, abort.
                    ATMOS_ERROR(
                            "Error calling ftello for checksumming file: %s\n",
                            strerror(errno));
                    if(strcmp(response->parent.class_name, CLASS_REST_RESPONSE) != 0) {
                        // It's not a basic RestResponse, it must be an AtmosResponse.
                        // Put in some more details.
                        AtmosResponse_set_api_error((AtmosResponse*)response,
                                ATMOS_ERROR_API_FTELL_NOT_SUPPORTED,
                                ATMOS_ERROR_API_FTELL_NOT_SUPPORTED_STR);
                    } else {
                        response->http_code = 400;
                        strcpy(response->http_status, "Bad Request");
                    }
                    return;
                }
                AtmosChecksum_update_file(req->checksum, f, request->request_body->data_size);
                if(fseeko(f, cur, SEEK_SET) == -1) {
                    ATMOS_ERROR(
                            "Error calling fseeko for checksumming file: %s\n",
                            strerror(errno));
                    if(strcmp(response->parent.class_name, CLASS_REST_RESPONSE) != 0) {
                        // It's not a basic RestResponse, it must be an AtmosResponse.
                        // Put in some more details.
                        AtmosResponse_set_api_error((AtmosResponse*)response,
                                ATMOS_ERROR_API_FSEEK_NOT_SUPPORTED,
                                ATMOS_ERROR_API_FSEEK_NOT_SUPPORTED_STR);
                    } else {
                        response->http_code = 400;
                        strcpy(response->http_status, "Bad Request");
                    }
                    return;
                }
            } else {
                // Checksum the buffer
                AtmosChecksum_update(req->checksum, request->request_body->body,
                        (size_t)request->request_body->data_size);
            }

            // Set the x-emc-wschecksum header.
            value = AtmosChecksum_get_state(req->checksum);
            if(!value) {
                if(strcmp(response->parent.class_name, CLASS_REST_RESPONSE) != 0) {
                    // It's not a basic RestResponse, it must be an AtmosResponse.
                    // Put in some more details.
                    AtmosResponse_set_api_error((AtmosResponse*)response,
                            ATMOS_ERROR_API_CHECKSUM_FAILED,
                            ATMOS_ERROR_API_CHECKSUM_FAILED_STR);
                } else {
                    response->http_code = 400;
                    strcpy(response->http_status, "Bad Request");
                }
                return;
            }
            snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX,
                    ATMOS_HEADER_WSCHECKSUM ": %s", value);
            RestRequest_add_header(request, buffer);
            free(value);
        }
    } else if(req->checksum && req->checksum->mode == ATMOS_GENERATE_CHECKSUM &&
            RestRequest_get_header(request, ATMOS_HEADER_GENERATE_CHECKSUM) == NULL) {
        char buffer[ATMOS_SIMPLE_HEADER_MAX];
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX,
                ATMOS_HEADER_GENERATE_CHECKSUM ": %s",
                req->checksum->algorithm);

        RestRequest_add_header(request, buffer);
        if(request->request_body) {
            if(request->request_body->file_body) {
                RestRequest_set_file_filter(request,
                        AtmosWriteObjectRequest_checksum_filter);
            } else {
                // Memory buffer -- do it now.
                AtmosChecksum_update(req->checksum, request->request_body->body,
                        (size_t)request->request_body->data_size);
            }
        }
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

}

AtmosWriteObjectRequest*
AtmosWriteObjectRequest_init(AtmosWriteObjectRequest *self, const char *uri,
        enum http_method method) {
    RestRequest_init((RestRequest*)self, uri, method);
    OBJECT_ZERO(self, AtmosWriteObjectRequest, RestRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_WRITE_OBJECT_REQUEST;

    return self;
}


void
AtmosWriteObjectRequest_destroy(AtmosWriteObjectRequest *self) {
    OBJECT_ZERO(self, AtmosWriteObjectRequest, RestRequest);
    RestRequest_destroy((RestRequest*)self);
}

AtmosChecksum*
AtmosChecksum_init(AtmosChecksum *self, enum atmos_checksum_mode mode,
        enum atmos_checksum_alg algorithm) {
    unsigned long e;
    Object_init_with_class_name((Object*)self, CLASS_ATMOS_CHECKSUM);
    OBJECT_ZERO(self, AtmosChecksum, Object);

    switch(algorithm) {
    case ATMOS_ALG_MD5:
        self->internal_state = malloc(sizeof(MD5_CTX));
        if(!MD5_Init((MD5_CTX*)self->internal_state)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error initializing MD5: %s\n", ERR_error_string(e, NULL));
            free(self->internal_state);
            self->internal_state = NULL;
        }
        self->algorithm = ATMOS_CHECKSUM_ALG_MD5;
        break;
    case ATMOS_ALG_SHA0:
        self->internal_state = malloc(sizeof(SHA_CTX));
        if(!SHA_Init((SHA_CTX*)self->internal_state)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error initializing SHA0: %s\n", ERR_error_string(e, NULL));
            free(self->internal_state);
            self->internal_state = NULL;
        }
        self->algorithm = ATMOS_CHECKSUM_ALG_SHA0;
        break;
    case ATMOS_ALG_SHA1:
        self->internal_state = malloc(sizeof(SHA_CTX));
        if(!SHA1_Init((SHA_CTX*)self->internal_state)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error initializing SHA1: %s\n", ERR_error_string(e, NULL));
            free(self->internal_state);
            self->internal_state = NULL;
        }
        self->algorithm = ATMOS_CHECKSUM_ALG_SHA1;
        break;
    default:
        ATMOS_ERROR("Unknown checksum algorithm: %d\n", algorithm);
        break;
    }

    self->alg = algorithm;
    self->mode = mode;

    return self;
}

void
AtmosChecksum_destroy(AtmosChecksum *self) {
    if(self->internal_state) {
        free(self->internal_state);
    }
    OBJECT_ZERO(self, AtmosChecksum, Object);
    Object_destroy((Object*)self);
}

void
AtmosChecksum_update(AtmosChecksum *self, const char *data, size_t count) {
    unsigned long e;

    if(!self->internal_state) {
        ATMOS_ERROR("Algorithm not initialized: %d\n", self->alg);
        return;
    }

    switch(self->alg) {
    case ATMOS_ALG_MD5:
        if(!MD5_Update((MD5_CTX*)self->internal_state, data, count)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error updating MD5: %s\n", ERR_error_string(e, NULL));
        }
        break;
    case ATMOS_ALG_SHA0:
        if(!SHA_Update((SHA_CTX*)self->internal_state, data, count)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error updating SHA0: %s\n", ERR_error_string(e, NULL));
        }
        break;
    case ATMOS_ALG_SHA1:
        if(!SHA1_Update((SHA_CTX*)self->internal_state, data, count)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error updating SHA1: %s\n", ERR_error_string(e, NULL));
        }
        break;
    default:
        ATMOS_ERROR("Unknown checksum algorithm: %d\n", self->alg);
        break;
    }
    self->offset += count;

}

void
AtmosChecksum_update_file(AtmosChecksum *self, FILE *data, int64_t count) {
    char buffer[65536];
    size_t c = 0;
    int64_t read = 0;
    do {
        if(read + sizeof(buffer) > count) {
            c = fread(buffer, 1, count - read, data);
        } else {
            c = fread(buffer, 1, sizeof(buffer), data);
        }
        if(c == 0) {
            // EOF
            break;
        }
        AtmosChecksum_update(self, buffer, c);
        read += c;
    } while(read < count);

    if(read < count) {
        ATMOS_ERROR("Expected to read %" PRId64
                " bytes but only read %" PRId64 "\n", count, read);
    }
}


char *
AtmosChecksum_get_state(AtmosChecksum *self) {
    char *value;
    char *state;
    char offsetbuf[ATMOS_SIMPLE_HEADER_MAX];
    size_t outsz;

    value = AtmosChecksum_get_value(self);
    if(!value) {
        return NULL;
    }
    sprintf(offsetbuf, "%" PRId64, self->offset);
    outsz = strlen(value) + strlen(offsetbuf) + 7;
    state = malloc(outsz);
    snprintf(state, outsz, "%s/%s/%s", self->algorithm, offsetbuf, value);
    free(value);
    return state;
}

char
AtmosChecksum_nibble_to_hex_char(unsigned int nibble) {
    if(nibble < 10) {
        return '0' + nibble;
    } else {
        return 'a' + (nibble-10);
    }
}

static void
AtmosChecksum_to_hex_string(unsigned char *bytes, char *buffer, size_t bytecount) {
    size_t i;

    for(i=0; i<bytecount; i++) {
        unsigned int high = bytes[i]>>4;
        unsigned int low = bytes[i] & 0xf;
        buffer[i*2] = AtmosChecksum_nibble_to_hex_char(high);
        buffer[i*2+1] = AtmosChecksum_nibble_to_hex_char(low);
    }
    buffer[i*2] = 0;
}

char *
AtmosChecksum_get_value(AtmosChecksum *self) {
    unsigned long e;

    switch(self->alg) {
    case ATMOS_ALG_MD5:
    {
        MD5_CTX md5;
        unsigned char md5_digest[MD5_DIGEST_LENGTH];
        char md5_hex[MD5_DIGEST_LENGTH*2+1];

        memcpy(&md5, self->internal_state, sizeof(MD5_CTX));
        if(!MD5_Final(md5_digest, &md5)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error finalizing MD5: %s\n", ERR_error_string(e, NULL));
            return NULL;
        }
        AtmosChecksum_to_hex_string(md5_digest, md5_hex, MD5_DIGEST_LENGTH);
        return strdup(md5_hex);
    }
    case ATMOS_ALG_SHA0:
    {
        SHA_CTX sha0;
        unsigned char sha0_digest[SHA_DIGEST_LENGTH];
        char sha0_hex[SHA_DIGEST_LENGTH*2+1];

        memcpy(&sha0, self->internal_state, sizeof(SHA_CTX));
        if(!SHA_Final(sha0_digest, &sha0)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error finalizing SHA0: %s\n", ERR_error_string(e, NULL));
            return NULL;
        }
        AtmosChecksum_to_hex_string(sha0_digest, sha0_hex, SHA_DIGEST_LENGTH);
        return strdup(sha0_hex);
    }
    case ATMOS_ALG_SHA1:
    {
        SHA_CTX sha1;
        unsigned char sha1_digest[SHA_DIGEST_LENGTH];
        char sha1_hex[SHA_DIGEST_LENGTH*2+1];

        memcpy(&sha1, self->internal_state, sizeof(SHA_CTX));
        if(!SHA1_Final(sha1_digest, &sha1)) {
            e = ERR_get_error();
            ATMOS_ERROR("Error finalizing SHA1: %s\n", ERR_error_string(e, NULL));
            return NULL;
        }
        AtmosChecksum_to_hex_string(sha1_digest, sha1_hex, SHA_DIGEST_LENGTH);
        return strdup(sha1_hex);
    }
    default:
        ATMOS_ERROR("Unknown checksum algorithm: %d\n", self->alg);
        return NULL;
    }

}

int
AtmosChecksum_verify_response(AtmosChecksum *self, RestResponse *response) {
    const char *rvalue;
    char *value, *lvalue;
    char buffer[ATMOS_SIMPLE_HEADER_MAX];


    if(!self->mode == ATMOS_GENERATE_CHECKSUM) {
        ATMOS_WARN("AtmosChecksum_verify_response called with mode %d "
                "instead of ATMOS_GENERATE_CHECKSUM\n", self->mode);
        return 1;
    }

    rvalue = RestResponse_get_header_value(response,
            ATMOS_HEADER_CONTENT_CHECKSUM);
    if(!rvalue) {
        ATMOS_WARN("Header %s not found in response\n",
                ATMOS_HEADER_CONTENT_CHECKSUM);
        return 0;
    }
    value = strdup(rvalue);

    lvalue = AtmosChecksum_get_value(self);
    if(!lvalue) {
        ATMOS_ERROR("Error getting local checksum! %s", "\n");
        free(value);
        return 0;
    }

    snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s/%s", self->algorithm, lvalue);
    AtmosUtil_lowercase(value);
    if(strcmp(buffer, value) != 0) {
        ATMOS_WARN("Checksum validation failed. Local: %s, Remote: %s\n",
                buffer, value);
        free(lvalue);
        free(value);
        return 0;
    }

    free(lvalue);
    free(value);
    return 1;
}

