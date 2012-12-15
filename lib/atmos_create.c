/*
 * atmos_create.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atmos.h"
#include "atmos_util.h"

static AtmosCreateObjectRequest*
AtmosCreateObjectRequest_common_init(AtmosCreateObjectRequest *self) {
    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_OBJECT_REQUEST;

    memset(((void*)self)+sizeof(RestRequest), 0,
            sizeof(AtmosCreateObjectRequest) - sizeof(RestRequest));

    return self;
}

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init(AtmosCreateObjectRequest *self) {
    RestRequest_init((RestRequest*)self, "/rest/objects", HTTP_POST);
    return AtmosCreateObjectRequest_common_init(self);
}

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init_ns(AtmosCreateObjectRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX + 15];

    if(path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX, "/rest/namespace%s", path);

    RestRequest_init((RestRequest*)self, uri, HTTP_POST);
    strncpy(self->path, path, ATMOS_PATH_MAX);

    return AtmosCreateObjectRequest_common_init(self);
}

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init_keypool(AtmosCreateObjectRequest *self,
        const char *pool, const char *key) {
    char poolheader[ATMOS_PATH_MAX];
    char uri[ATMOS_PATH_MAX + 15];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/namespace/%s", key);

    RestRequest_init((RestRequest*)self, uri, HTTP_POST);

    AtmosCreateObjectRequest_common_init(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosCreateObjectRequest_destroy(AtmosCreateObjectRequest *self) {
    memset(((void*)self)+sizeof(RestRequest), 0,
            sizeof(AtmosCreateObjectRequest) - sizeof(RestRequest));

    RestRequest_destroy((RestRequest*)self);
}

void
AtmosCreateObjectRequest_add_metadata(AtmosCreateObjectRequest *self,
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
AtmosCreateObjectRequest_add_acl(AtmosCreateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission) {
    int i = self->acl_count++;
    self->acl[i].permission = permission;
    self->acl[i].type = type;
    strcpy(self->acl[i].principal, principal);
}

AtmosCreateObjectResponse*
AtmosCreateObjectResponse_init(AtmosCreateObjectResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);

    memset(((void*)self)+sizeof(AtmosResponse), 0,
            sizeof(AtmosCreateObjectResponse) - sizeof(AtmosResponse));

    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_OBJECT_RESPONSE;

    return self;
}

void
AtmosCreateObjectResponse_destroy(AtmosCreateObjectResponse *self) {
    memset(((void*)self)+sizeof(AtmosResponse), 0,
            sizeof(AtmosCreateObjectResponse) - sizeof(AtmosResponse));

    AtmosResponse_destroy((AtmosResponse*)self);
}

void AtmosFilter_parse_create_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    const char *location;
    size_t location_len;

    // Do nothing on the request, just pass to the next filter.
    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code != 201) {
        return;
    }

    // The ObjectID should be in the Location header
    location = RestResponse_get_header_value(response, HTTP_HEADER_LOCATION);

    if(!location) {
        ATMOS_ERROR("Could not find header %s in response",
                HTTP_HEADER_LOCATION);
        return;
    }

    // Header will be /rest/objects/oid
    // Take last 44 digits.
    location_len = strlen(location);
    if(location_len != ATMOS_OID_LENGTH-1+ATMOS_OID_LOCATION_PREFIX_SIZE) {
        ATMOS_ERROR("Error: location was %zd bytes; expected %zd",
                location_len,
                (size_t)ATMOS_OID_LENGTH-1+ATMOS_OID_LOCATION_PREFIX_SIZE);
        return;

    }
    strncpy(((AtmosCreateObjectResponse*)response)->object_id,
            location+ATMOS_OID_LOCATION_PREFIX_SIZE, ATMOS_OID_LENGTH);
}

void AtmosFilter_set_create_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {

    AtmosCreateObjectRequest *req = (AtmosCreateObjectRequest*)request;
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

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

}


