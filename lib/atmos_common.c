/*
 * atmos_common.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */
#include "atmos.h"
#include "atmos_util.h"

const char const *ATMOS_SYSTEM_META_NAMES[] = {ATMOS_SYSTEM_META_ATIME,
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

