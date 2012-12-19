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
#include <inttypes.h>

#include "atmos.h"
#include "atmos_util.h"
#include "atmos_private.h"

static void
AtmosSetUserMetaRequest_init_common(AtmosSetUserMetaRequest *self) {
    OBJECT_ZERO(self, AtmosSetUserMetaRequest, AtmosWriteObjectRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_SET_USER_META_REQUEST;
}

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init(AtmosSetUserMetaRequest *self,
        const char *object_id) {
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/user", object_id);
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_POST);

    AtmosSetUserMetaRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_ns(AtmosSetUserMetaRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?metadata/user", path);
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_POST);

    AtmosSetUserMetaRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_keypool(AtmosSetUserMetaRequest *self,
        const char *pool, const char *key) {
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", key);
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_POST);

    AtmosSetUserMetaRequest_init_common(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosSetUserMetaRequest_destroy(AtmosSetUserMetaRequest *self) {
    OBJECT_ZERO(self, AtmosSetUserMetaRequest, AtmosWriteObjectRequest);

    AtmosWriteObjectRequest_destroy((AtmosWriteObjectRequest*)self);
}

void
AtmosSetUserMetaRequest_add_metadata(AtmosSetUserMetaRequest *self,
        const char *name, const char *value,
        int listable) {
    AtmosWriteObjectRequest_add_metadata((AtmosWriteObjectRequest*)self, name,
            value, listable);
}


void AtmosFilter_update_object(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    char buffer[ATMOS_SIMPLE_HEADER_MAX];
    AtmosUpdateObjectRequest *req;

    req = (AtmosUpdateObjectRequest*)request;

    // Build a range header if needed.
    if(req->range_start != -1 && req->range_end != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=%" PRId64 "-%" PRId64, HTTP_HEADER_RANGE, req->range_start,
                req->range_end);
        RestRequest_add_header(request, buffer);
    } else if(req->range_start != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=%" PRId64 "-", HTTP_HEADER_RANGE, req->range_start);
        RestRequest_add_header(request, buffer);
    } else if(req->range_end != -1) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX, "%s: Bytes=-%" PRId64, HTTP_HEADER_RANGE, req->range_end);
        RestRequest_add_header(request, buffer);
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
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_PUT);

    AtmosUpdateObjectRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_ns(AtmosUpdateObjectRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s", path);
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_PUT);

    AtmosUpdateObjectRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_keypool(AtmosUpdateObjectRequest *self,
        const char *pool, const char *key) {
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s", key);
    AtmosWriteObjectRequest_init((AtmosWriteObjectRequest*)self, uri, HTTP_PUT);

    AtmosUpdateObjectRequest_init_common(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosUpdateObjectRequest_destroy(AtmosUpdateObjectRequest *self) {
    OBJECT_ZERO(self, AtmosUpdateObjectRequest, AtmosWriteObjectRequest);
    AtmosWriteObjectRequest_destroy((AtmosWriteObjectRequest*)self);
}

void
AtmosUpdateObjectRequest_add_metadata(AtmosUpdateObjectRequest *self,
        const char *name, const char *value,
        int listable) {
    AtmosWriteObjectRequest_add_metadata((AtmosWriteObjectRequest*)self, name,
            value, listable);
}

void
AtmosUpdateObjectRequest_add_acl(AtmosUpdateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission) {
    AtmosWriteObjectRequest_add_acl((AtmosWriteObjectRequest*)self, principal,
            type, permission);
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


