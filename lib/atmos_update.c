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

/**
 * Adds a metadata entry to the set user metadata request.
 * @param self the AtmosSetUserMetaRequest to modify.
 * @param name name for the metadata entry.
 * @param value value for the metadata entry.
 * @param listable nonzero if this entry's name should be 'listable'.  Use with
 * caution.  See programmer's documentation.
 */
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


