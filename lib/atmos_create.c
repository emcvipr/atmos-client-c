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
            sizeof(AtmosCreateObjectRequest) - sizeof(AtmosWriteObjectRequest));

    return self;
}

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init(AtmosCreateObjectRequest *self) {
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, "/rest/objects", HTTP_POST);
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

    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_POST);
    strncpy(self->path, path, ATMOS_PATH_MAX);

    return AtmosCreateObjectRequest_common_init(self);
}

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init_keypool(AtmosCreateObjectRequest *self,
        const char *pool, const char *key) {
    char poolheader[ATMOS_PATH_MAX];
    char uri[ATMOS_PATH_MAX + 15];

    snprintf(uri, ATMOS_PATH_MAX, "/rest/namespace/%s", key);

    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_POST);

    AtmosCreateObjectRequest_common_init(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosCreateObjectRequest_destroy(AtmosCreateObjectRequest *self) {
    OBJECT_ZERO(self, AtmosCreateObjectRequest, AtmosWriteObjectRequest);

    AtmosWriteObjectRequest_destroy((AtmosWriteObjectRequest*)self);
}

void
AtmosCreateObjectRequest_add_metadata(AtmosCreateObjectRequest *self,
        const char *name, const char *value,
        int listable) {
    AtmosWriteObjectRequest_add_metadata((AtmosWriteObjectRequest*)self, name,
            value, listable);
}

void
AtmosCreateObjectRequest_add_acl(AtmosCreateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission) {
    AtmosWriteObjectRequest_add_acl((AtmosWriteObjectRequest*)self, principal,
            type, permission);
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
    // Take last 44-64 digits.
    location_len = strlen(location);
    if(location_len > ATMOS_OID_LENGTH-1+ATMOS_OID_LOCATION_PREFIX_SIZE) {
        ATMOS_ERROR("Error: location was %zd bytes; expected max %zd",
                location_len,
                (size_t)ATMOS_OID_LENGTH-1+ATMOS_OID_LOCATION_PREFIX_SIZE);
        return;

    }
    strncpy(((AtmosCreateObjectResponse*)response)->object_id,
            location+ATMOS_OID_LOCATION_PREFIX_SIZE,
            location_len-ATMOS_OID_LOCATION_PREFIX_SIZE);
}



