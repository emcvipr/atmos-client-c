/*
 * atmos_read.c
 *
 *  Created on: Nov 19, 2012
 *      Author: cwikj
 */
#include "atmos.h"
#include "atmos_private.h"

static AtmosReadObjectRequest*
AtmosReadObjectRequest_init_common(AtmosReadObjectRequest *self) {
    memset(((void*) self) + sizeof(RestRequest), 0,
            sizeof(AtmosReadObjectRequest) - sizeof(RestRequest));
    self->range_start = -1;
    self->range_end = -1;

    ((Object*) self)->class_name = CLASS_ATMOS_READ_OBJECT_REQUEST;
    return self;
}

AtmosReadObjectRequest*
AtmosReadObjectRequest_init(AtmosReadObjectRequest *self, const char *object_id) {
    char uri[ATMOS_PATH_MAX + ATMOS_OID_LENGTH + 1];
}

AtmosReadObjectRequest*
AtmosReadObjectRequest_init_ns(AtmosReadObjectRequest *self, const char *path) {
    char uri[ATMOS_PATH_MAX + 15];

    if (path[0] != '/') {
        fprintf(stderr, "Path must start with a '/'\n");
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX + 15, "/rest/namespace%s", path);

    RestRequest_init((RestRequest*) self, uri, HTTP_GET);
    return AtmosReadObjectRequest_init_common(self);
}

void AtmosReadObjectRequest_destroy(AtmosReadObjectRequest *self) {
    self->object_id[0] = 0;
    self->path[0] = 0;
    self->range_start = -1;
    self->range_end = -1;
    RestRequest_destroy((RestRequest*) self);
}

void AtmosReadObjectRequest_set_range(AtmosReadObjectRequest *self,
        int64_t range_start, int64_t range_end) {
    self->range_start = range_start;
    self->range_end = range_end;
}

void AtmosReadObjectRequest_set_range_offset_size(AtmosReadObjectRequest *self,
        int64_t offset, int64_t size) {
    self->range_start = offset;
    self->range_end = offset + size - 1;
}

AtmosReadObjectResponse*
AtmosReadObjectResponse_init(AtmosReadObjectResponse *self) {
    AtmosResponse_init((AtmosResponse*) self);
    memset(((void*) self) + sizeof(AtmosResponse), 0,
            sizeof(AtmosReadObjectResponse) - sizeof(AtmosResponse));
    ((Object*) self)->class_name = CLASS_ATMOS_READ_OBJECT_RESPONSE;
    return self;
}

void AtmosReadObjectResponse_destroy(AtmosReadObjectResponse *self) {
    memset(((void*) self) + sizeof(AtmosResponse), 0,
            sizeof(AtmosReadObjectResponse) - sizeof(AtmosResponse));
    AtmosResponse_destroy((AtmosResponse*) self);
}

void AtmosFilter_parse_read_object_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosReadObjectResponse *res = (AtmosReadObjectResponse*)response;

    // Do nothing on the request, just pass to the next filter.
    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    // Parse out the metadata and ACL.
    AtmosUtil_parse_system_meta_header(res, &(res->system_metadata));
    AtmosUtil_parse_user_meta_headers(response, &(res->meta),
            &(res->meta_count), &(res->listable_meta),
            &(res->listable_meta_count));
    AtmosUtil_parse_acl_headers(res, &(res->acl), &(res->acl_count));
}

void AtmosFilter_set_read_object_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosReadObjectRequest *req = (AtmosReadObjectRequest*)request;
    char buffer[ATMOS_SIMPLE_HEADER_MAX];

    // Build a range header.
    if(req->range_start != -1 && req->range_end != -1) {
        snprintf("%s: %lld-%lld", HTTP_HEADER_RANGE, req->range_start,
                req->range_end);
    } else if(req->range_start != -1) {
        snprintf("%s: %lld-", HTTP_HEADER_RANGE, req->range_start);
    } else if(req->range_end != -1) {
        snprintf("%s: -%lld", HTTP_HEADER_RANGE, req->range_end);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}

