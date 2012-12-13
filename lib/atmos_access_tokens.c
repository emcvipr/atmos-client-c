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

#include "atmos.h"
#include "atmos_private.h"
#include "atmos_util.h"

static void
AtmosCreateAccessTokenRequest_init_common(AtmosCreateAccessTokenRequest *self) {
    OBJECT_ZERO(self, AtmosCreateAccessTokenRequest, RestRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_ACCESS_TOKEN_REQUEST;
}

AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init(AtmosCreateAccessTokenRequest *self) {
    RestRequest_init((RestRequest*)self, "/rest/accesstokens", HTTP_POST);

    AtmosCreateAccessTokenRequest_init_common(self);

    return self;
}


AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_obj(AtmosCreateAccessTokenRequest *self,
        const char *object_id) {
    RestRequest_init((RestRequest*)self, "/rest/accesstokens", HTTP_POST);
    AtmosCreateAccessTokenRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}


AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_ns(AtmosCreateAccessTokenRequest *self,
        const char *path) {
    RestRequest_init((RestRequest*)self, "/rest/accesstokens", HTTP_POST);
    AtmosCreateAccessTokenRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}


void
AtmosCreateAccessTokenRequest_destroy(AtmosCreateAccessTokenRequest *self) {
    if(self->policy) {
        Atmos_policyType_destroy(self->policy);
        free(self->policy);
    }
    OBJECT_ZERO(self, AtmosCreateAccessTokenRequest, RestRequest);
    RestRequest_destroy((RestRequest*)self);
}


AtmosCreateAccessTokenResponse*
AtmosCreateAccessTokenResponse_init(AtmosCreateAccessTokenResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosCreateAccessTokenResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_ACCESS_TOKEN_RESPONSE;

    return self;
}


void
AtmosCreateAccessTokenResponse_destroy(AtmosCreateAccessTokenResponse *self) {
    OBJECT_ZERO(self, AtmosCreateAccessTokenResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}


AtmosListAccessTokensRequest*
AtmosListAccessTokensRequest_init(AtmosListAccessTokensRequest *self) {
    AtmosPaginatedRequest_init((AtmosPaginatedRequest*)self,
            "/rest/accesstokens", HTTP_GET);
    OBJECT_ZERO(self, AtmosListAccessTokensRequest, AtmosPaginatedRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_ACCESS_TOKENS_REQUEST;

    return self;
}


void
AtmosListAccessTokensRequest_destroy(AtmosListAccessTokensRequest *self) {
    OBJECT_ZERO(self, AtmosListAccessTokensRequest, AtmosPaginatedRequest);
    AtmosPaginatedRequest_destroy((AtmosPaginatedRequest*)self);
}


AtmosListAccessTokensResponse*
AtmosListAccessTokensResponse_init(AtmosListAccessTokensResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosListAccessTokensResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_ACCESS_TOKENS_RESPONSE;

    return self;
}


void
AtmosListAccessTokensResponse_destroy(AtmosListAccessTokensResponse *self) {
    if(self->token_list) {
        Atmos_listAccessTokenResultType_destroy(self->token_list);
        free(self->token_list);
    }
    OBJECT_ZERO(self, AtmosListAccessTokensResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}


AtmosGetAccessTokenInfoResponse*
AtmosGetAccessTokenInfoResponse_init(AtmosGetAccessTokenInfoResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosGetAccessTokenInfoResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_GET_ACCESS_TOKEN_INFO_RESPONSE;

    return self;
}


void
AtmosGetAccessTokenInfoResponse_destroy(AtmosGetAccessTokenInfoResponse *self) {
    if(self->token_info) {
        Atmos_accessTokenType_destroy(self->token_info);
        free(self->token_info);
    }
    OBJECT_ZERO(self, AtmosGetAccessTokenInfoResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}

void AtmosFilter_create_access_token(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response) {
    AtmosCreateAccessTokenRequest *req = (AtmosCreateAccessTokenRequest*)request;
    AtmosCreateAccessTokenResponse *res = (AtmosCreateAccessTokenResponse*)response;
    AtmosClient *atmos = (AtmosClient*)rest;
    xmlChar *policy = NULL;
    char buffer[ATMOS_SIMPLE_HEADER_MAX];
    const char *location;
    size_t location_len;

    if(strlen(req->object_id) > 0) {
        // Set objectid
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX,
                ATMOS_HEADER_OBJECTID ": %s", req->object_id);
        RestRequest_add_header(request, buffer);
    } else if(strlen(req->path) > 0) {
        snprintf(buffer, ATMOS_SIMPLE_HEADER_MAX,
                ATMOS_HEADER_PATH ": %s", req->path);
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
        RestRequest_add_header(request,
                ATMOS_HEADER_UTF8 ": true");
    }

    if(req->acl_count > 0) {
        AtmosUtil_set_acl_header(req->acl, req->acl_count, request);
    }

    if(req->policy) {
        // Serialze the policy object.
        policy = Atmos_policyType_marshal(req->policy);
        if(!policy) {
            ATMOS_ERROR("Could not serialize policy object! %s\n", "");
            response->http_code = 400;
            res->parent.atmos_error = 0;
            strcpy(res->parent.atmos_error_message,
                    "Could not serialize policy object!");
            return;
        }
        RestRequest_set_array_body(request, (char*)policy,
                strlen((char*)policy), "text/xml");
    } else {
        // If there's no policy, set the content-type so curl doesn't pick its
        // own.
        RestRequest_add_header(request, HTTP_HEADER_CONTENT_TYPE ": text/xml");
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.
    if(policy) {
        xmlFree(policy);
    }

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    // Extract the token from the response
    // The ObjectID should be in the Location header
    location = RestResponse_get_header_value(response, HTTP_HEADER_LOCATION);

    if(!location) {
        ATMOS_ERROR("Could not find header %s in response",
                HTTP_HEADER_LOCATION);
        return;
    }

    // Header will be /rest/accesstokens/token
    location_len = strlen(ATMOS_ACCESS_TOKEN_LOCATION_PREFIX);
    if(strstr(location, ATMOS_ACCESS_TOKEN_LOCATION_PREFIX) != location) {
        ATMOS_ERROR("Access token prefix wasn't %s: %s",
                ATMOS_ACCESS_TOKEN_LOCATION_PREFIX, location);
        return;
    }

    strncpy(res->token_id, location+location_len, ATMOS_SIMPLE_HEADER_MAX);
}

void AtmosFilter_parse_list_access_tokens_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response) {
    AtmosListAccessTokensResponse *res = (AtmosListAccessTokensResponse*)response;
    const char *token;

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    res->token_list = Atmos_listAccessTokenResultType_unmarshal(
            response->body);

    // Check for token
    token = RestResponse_get_header_value(response, ATMOS_HEADER_TOKEN);
    if(token) {
        // Valid since this is a pointer into the response headers.
        res->token = token;
    }
}

void AtmosFilter_parse_get_token_info_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response) {
    AtmosGetAccessTokenInfoResponse *res =
            (AtmosGetAccessTokenInfoResponse*)response;

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    res->token_info = Atmos_accessTokenType_unmarshal(response->body);

}

