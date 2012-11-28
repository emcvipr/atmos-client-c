/*

 Copyright (c) 2012, EMC Corporation

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of the EMC Corporation nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <string.h>

#include "atmos.h"
#include "atmos_util.h"
#include "atmos_private.h"

void AtmosFilter_set_user_meta_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosSetUserMetaRequest *req = (AtmosSetUserMetaRequest*)request;
    AtmosClient *atmos = (AtmosClient*)rest;

    // since there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    RestRequest_add_header(request,
            HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");


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

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

}

static void
AtmosSetUserMetaRequest_init_common(AtmosSetUserMetaRequest *self) {
    OBJECT_ZERO(self, AtmosSetUserMetaRequest, RestRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_SET_USER_META_REQUEST;
}

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init(AtmosSetUserMetaRequest *self,
        const char *object_id) {
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/user", object_id);
    RestRequest_init((RestRequest*)self, uri, HTTP_POST);

    AtmosSetUserMetaRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_ns(AtmosSetUserMetaRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_POST);

    AtmosSetUserMetaRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

void
AtmosSetUserMetaRequest_destroy(AtmosSetUserMetaRequest *self) {
    OBJECT_ZERO(self, AtmosSetUserMetaRequest, RestRequest);

    RestRequest_destroy((RestRequest*)self);
}

void
AtmosSetUserMetaRequest_add_metadata(AtmosSetUserMetaRequest *self,
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


void AtmosFilter_update_object(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    char buffer[ATMOS_SIMPLE_HEADER_MAX];
    AtmosUpdateObjectRequest *req;
    AtmosClient *atmos;

    atmos = (AtmosClient*)rest;
    req = (AtmosUpdateObjectRequest*)request;

    // Special case -- if there's no content, make sure there is a content
    // type set otherwise curl will chose its own.
    if (!((RestRequest*)request)->request_body) {
        RestRequest_add_header((RestRequest*)request,
                HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
    }

    // Build a range header.
    if(req->range_start != -1 && req->range_end != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=%lld-%lld", HTTP_HEADER_RANGE, req->range_start,
                req->range_end);
        RestRequest_add_header(request, buffer);
    } else if(req->range_start != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=%lld-", HTTP_HEADER_RANGE, req->range_start);
        RestRequest_add_header(request, buffer);
    } else if(req->range_end != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=-%lld", HTTP_HEADER_RANGE, req->range_end);
        RestRequest_add_header(request, buffer);
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

static void
AtmosUpdateObjectRequest_init_common(AtmosUpdateObjectRequest *self) {
    OBJECT_ZERO(self, AtmosUpdateObjectRequest, RestRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_UPDATE_OBJECT_REQUEST;

    self->range_start = -1;
    self->range_end = -1;
}

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init(AtmosUpdateObjectRequest *self,
        const char *object_id) {
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s", object_id);
    RestRequest_init((RestRequest*)self, uri, HTTP_PUT);

    AtmosUpdateObjectRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_ns(AtmosUpdateObjectRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_PUT);

    AtmosUpdateObjectRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

void
AtmosUpdateObjectRequest_destroy(AtmosUpdateObjectRequest *self) {

}

void
AtmosUpdateObjectRequest_add_metadata(AtmosUpdateObjectRequest *self,
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
AtmosUpdateObjectRequest_add_acl(AtmosUpdateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission) {
    self->acl[self->acl_count].permission = permission;
    self->acl[self->acl_count].type = type;
    strncpy(self->acl[self->acl_count].principal, principal, ATMOS_UID_MAX);
    self->acl_count++;
}

void
AtmosUpdateObjectRequest_set_range(AtmosUpdateObjectRequest *self,
        int64_t range_start, int64_t range_end) {
    self->range_start = range_start;
    self->range_end = range_end;
}

void
AtmosUpdateObjectRequest_set_range_offset_size(AtmosUpdateObjectRequest *self,
        int64_t offset, int64_t size) {
    self->range_start = offset;
    self->range_end = offset + size - 1;
}


