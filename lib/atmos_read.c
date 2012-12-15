/*
 * atmos_read.c
 *
 *  Created on: Nov 19, 2012
 *      Author: cwikj
 */
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "atmos.h"
#include "atmos_private.h"
#include "atmos_util.h"

static void
AtmosReadObjectRequest_init_common(AtmosReadObjectRequest *self) {
    memset(((void*) self) + sizeof(RestRequest), 0,
            sizeof(AtmosReadObjectRequest) - sizeof(RestRequest));
    self->range_start = -1;
    self->range_end = -1;

    ((Object*) self)->class_name = CLASS_ATMOS_READ_OBJECT_REQUEST;
}

AtmosReadObjectRequest*
AtmosReadObjectRequest_init(AtmosReadObjectRequest *self, const char *object_id) {
    char uri[15+ATMOS_OID_LENGTH];

    snprintf(uri, 15+ATMOS_OID_LENGTH, "/rest/objects/%s", object_id);

    RestRequest_init((RestRequest*) self, uri, HTTP_GET);
    AtmosReadObjectRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosReadObjectRequest*
AtmosReadObjectRequest_init_ns(AtmosReadObjectRequest *self, const char *path) {
    char uri[ATMOS_PATH_MAX + 15];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX + 15, "/rest/namespace%s", path);

    RestRequest_init((RestRequest*) self, uri, HTTP_GET);
    AtmosReadObjectRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

AtmosReadObjectRequest*
AtmosReadObjectRequest_init_keypool(AtmosReadObjectRequest *self,
        const char *pool, const char *key) {
    char uri[ATMOS_PATH_MAX + 15];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX + 15, "/rest/namespace/%s", key);

    RestRequest_init((RestRequest*) self, uri, HTTP_GET);
    AtmosReadObjectRequest_init_common(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
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
    AtmosUtil_parse_system_meta_header(response,
            &(res->system_metadata));
    AtmosUtil_parse_user_meta_headers(response, res->meta,
            &(res->meta_count), res->listable_metadata,
            &(res->listable_meta_count));
    AtmosUtil_parse_acl_headers(response, res->acl, &(res->acl_count));
}

void AtmosFilter_set_read_object_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosReadObjectRequest *req = (AtmosReadObjectRequest*)request;
    char buffer[ATMOS_SIMPLE_HEADER_MAX];

    // Build a range header.
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

    if(((AtmosClient*)rest)->enable_utf8_metadata) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}

const char *
AtmosReadObjectResponse_get_metadata_value(AtmosReadObjectResponse *self,
        const char *name, int listable) {
    return AtmosUtil_get_metadata_value(name,
            listable?self->listable_metadata:self->meta,
            listable?self->listable_meta_count:self->meta_count);
}

enum atmos_acl_permission
AtmosReadObjectResponse_get_acl_permission(AtmosReadObjectResponse *self,
        const char *principal, enum atmos_acl_principal_type principal_type) {
    return AtmosUtil_get_acl_permission(self->acl, self->acl_count,
            principal, principal_type);
}

static void
AtmosGetUserMetaRequest_common_init(AtmosGetUserMetaRequest *self) {
    ((Object*)self)->class_name = CLASS_ATMOS_GET_USER_META_REQUEST;

    memset(((void*)self)+sizeof(RestRequest), 0,
            sizeof(AtmosGetUserMetaRequest)-sizeof(RestRequest));

}

AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init(AtmosGetUserMetaRequest *self,
        const char *object_id) {
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/user", object_id);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetUserMetaRequest_common_init(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init_ns(AtmosGetUserMetaRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?metadata/user", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetUserMetaRequest_common_init(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init_keypool(AtmosGetUserMetaRequest *self,
        const char *pool, const char *key) {
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", key);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetUserMetaRequest_common_init(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosGetUserMetaRequest_destroy(AtmosGetUserMetaRequest *self) {
    OBJECT_ZERO(self, AtmosGetUserMetaRequest, RestRequest);
    RestRequest_destroy((RestRequest*)self);
}

void
AtmosGetUserMetaRequest_add_tag(AtmosGetUserMetaRequest *self, const char *tag) {
    strncpy(self->tags[self->tag_count++], tag, ATMOS_META_NAME_MAX);
}

void AtmosFilter_parse_get_user_meta_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosGetUserMetaResponse *res = (AtmosGetUserMetaResponse*)response;

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
    AtmosUtil_parse_user_meta_headers(response, res->meta,
            &(res->meta_count), res->listable_metadata,
            &(res->listable_meta_count));
}

void AtmosFilter_set_get_user_meta_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosGetUserMetaRequest *req = (AtmosGetUserMetaRequest*)request;
    int utf8 = ((AtmosClient*)rest)->enable_utf8_metadata;

    if(utf8) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    if(req->tags && req->tag_count > 0) {
        AtmosUtil_set_tags_header(request, req->tags, req->tag_count, utf8);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

}

AtmosGetUserMetaResponse*
AtmosGetUserMetaResponse_init(AtmosGetUserMetaResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);

    OBJECT_ZERO(self, AtmosGetUserMetaResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_GET_USER_META_RESPONSE;

    return self;
}

void
AtmosGetUserMetaResponse_destroy(AtmosGetUserMetaResponse *self) {
    OBJECT_ZERO(self, AtmosGetUserMetaResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}


const char *
AtmosGetUserMetaResponse_get_metadata_value(AtmosGetUserMetaResponse *self,
        const char *name, int listable) {
    return AtmosUtil_get_metadata_value(name,
            listable?self->listable_metadata:self->meta,
            listable?self->listable_meta_count:self->meta_count);
}

static void
AtmosGetSystemMetaRequest_init_common(AtmosGetSystemMetaRequest *self) {
    OBJECT_ZERO(self, AtmosGetSystemMetaRequest, RestRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_GET_SYSTEM_META_REQUEST;
}

AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init(AtmosGetSystemMetaRequest *self,
        const char *object_id) {
    char uri[ATMOS_OID_LENGTH+64];

    snprintf(uri, ATMOS_OID_LENGTH+64, "/rest/objects/%s?metadata/system", object_id);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetSystemMetaRequest_init_common(self);

    strncpy(self->object_id, object_id, ATMOS_OID_LENGTH);

    return self;
}

AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init_ns(AtmosGetSystemMetaRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX+64];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace%s?metadata/system", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetSystemMetaRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init_keypool(AtmosGetSystemMetaRequest *self,
        const char *pool, const char *key) {
    char uri[ATMOS_PATH_MAX+64];
    char poolheader[ATMOS_PATH_MAX];

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/system", key);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetSystemMetaRequest_init_common(self);

    snprintf(poolheader, ATMOS_PATH_MAX, ATMOS_HEADER_POOL ": %s", pool);
    RestRequest_add_header((RestRequest*)self, poolheader);

    strncpy(self->path, key, ATMOS_PATH_MAX);
    strncpy(self->pool, pool, ATMOS_PATH_MAX);

    return self;
}


void
AtmosGetSystemMetaRequest_destroy(AtmosGetSystemMetaRequest *self) {
    OBJECT_ZERO(self, AtmosGetSystemMetaRequest, RestRequest);

    RestRequest_destroy((RestRequest*)self);
}

void
AtmosGetSystemMetaRequest_add_tag(AtmosGetSystemMetaRequest *self,
        const char *tag) {
    strncpy(self->tags[self->tag_count++], tag, ATMOS_META_NAME_MAX);
}

AtmosGetSystemMetaResponse*
AtmosGetSystemMetaResponse_init(AtmosGetSystemMetaResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);

    OBJECT_ZERO(self, AtmosGetSystemMetaResponse, AtmosResponse);

    ((Object*)self)->class_name = CLASS_ATMOS_GET_SYSTEM_META_RESPONSE;

    return self;
}

void
AtmosGetSystemMetaResponse_destroy(AtmosGetSystemMetaResponse *self) {
    OBJECT_ZERO(self, AtmosGetSystemMetaResponse, AtmosResponse);

    AtmosResponse_destroy((AtmosResponse*)self);
}


void
AtmosFilter_get_system_metadata(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosGetSystemMetaRequest *req = (AtmosGetSystemMetaRequest*)request;
    AtmosGetSystemMetaResponse *res = (AtmosGetSystemMetaResponse*)response;

    if(((AtmosClient*)rest)->enable_utf8_metadata) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    if(req->tags && req->tag_count > 0) {
        AtmosUtil_set_tags_header(request, req->tags, req->tag_count, 0);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    // Parse out the metadata
    AtmosUtil_parse_system_meta_header(response,
            &(res->system_metadata));
}

void
AtmosFilter_parse_get_acl_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosGetAclResponse *res = (AtmosGetAclResponse*)response;

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    AtmosUtil_parse_acl_headers(response, res->acl, &(res->acl_count));
}

AtmosGetAclResponse*
AtmosGetAclResponse_init(AtmosGetAclResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);

    OBJECT_ZERO(self, AtmosGetAclResponse, AtmosResponse);

    ((Object*)self)->class_name = CLASS_ATMOS_GET_ACL_RESPONSE;

    return self;
}

void
AtmosGetAclResponse_destroy(AtmosGetAclResponse *self) {
    AtmosResponse_destroy((AtmosResponse*)self);

    OBJECT_ZERO(self, AtmosGetAclResponse, AtmosResponse);
}

enum atmos_acl_permission
AtmosGetAclResponse_get_acl_permission(AtmosGetAclResponse *self,
        const char *principal, enum atmos_acl_principal_type principal_type) {
    return AtmosUtil_get_acl_permission(self->acl, self->acl_count,
            principal, principal_type);
}

AtmosGetObjectInfoResponse*
AtmosGetObjectInfoResponse_init(AtmosGetObjectInfoResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);

    OBJECT_ZERO(self, AtmosGetObjectInfoResponse, AtmosResponse);

    ((Object*)self)->class_name = CLASS_ATMOS_GET_OBJECT_INFO_RESPONSE;

    return self;
}

void
AtmosGetObjectInfoResponse_destroy(AtmosGetObjectInfoResponse *self) {
    if(self->replicas) {
        free(self->replicas);
    }
    OBJECT_ZERO(self, AtmosGetObjectInfoResponse, AtmosResponse);

    AtmosResponse_destroy((AtmosResponse*)self);
}

void
AtmosGetObjectInfoResponse_parse(AtmosGetObjectInfoResponse *self,
        const char *xml, size_t xml_size) {
    xmlDocPtr doc;
    xmlChar *value;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;
    char xpathBuffer[ATMOS_SIMPLE_HEADER_MAX];
    int i;

    /*
     * The document being in memory, it have no base per RFC 2396,
     * and the "noname.xml" argument will serve as its base.
     */
    doc = xmlReadMemory(xml, (int)xml_size,
            "noname.xml", NULL, 0);
    if (doc == NULL) {
        ATMOS_ERROR("Failed to parse info response: %s\n", xml);
        return;
    }

    // objectId
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:objectId", 1);
    if(value) {
        strncpy(self->object_id, (char*)value, ATMOS_OID_LENGTH);
        xmlFree(value);
    }

    // selection
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:selection", 1);
    if(value) {
        strncpy(self->selection, (char*)value, ATMOS_SIMPLE_HEADER_MAX);
        xmlFree(value);
    }

    // numReplicas
    // There is a bug in 2.0 that causes this value to become a printf
    // specifier (%zu).  Therefore, we ignore it and simply use the number of
    // replica elements in the response.
//    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:numReplicas", 1);
//    if(value) {
//        self->replica_count = strtol((char*)value, NULL, 10);
//        xmlFree(value);
//    }

    // Replicas
    xpathObj = AtmosUtil_select_nodes(doc, BAD_CAST "//cos:replicas/cos:replica", 1);
    if(xpathObj) {
        xpathNodeSet = xpathObj->nodesetval;
        self->replica_count = xpathNodeSet->nodeNr;
        self->replicas = calloc(self->replica_count, sizeof(AtmosReplicaInfo));

        for(i=0; i<self->replica_count; i++) {
            // XPath indexes are 1-based
            int xpathIndex = i+1;
            // id
            snprintf(xpathBuffer, ATMOS_SIMPLE_HEADER_MAX,
                    "//cos:replicas/cos:replica[%d]/cos:id", xpathIndex);
            value = AtmosUtil_select_single_node_value(doc, BAD_CAST xpathBuffer, 1);
            if(value) {
                self->replicas[i].id = (int)strtol((char*)value, NULL, 10);
                xmlFree(value);
            }

            // type
            snprintf(xpathBuffer, ATMOS_SIMPLE_HEADER_MAX,
                    "//cos:replicas/cos:replica[%d]/cos:type", xpathIndex);
            value = AtmosUtil_select_single_node_value(doc, BAD_CAST xpathBuffer, 1);
            if(value) {
                strncpy(self->replicas[i].type, (char*)value, 16);
                xmlFree(value);
            }

            // current
            snprintf(xpathBuffer, ATMOS_SIMPLE_HEADER_MAX,
                    "//cos:replicas/cos:replica[%d]/cos:current", xpathIndex);
            value = AtmosUtil_select_single_node_value(doc, BAD_CAST xpathBuffer, 1);
            if(value) {
                self->replicas[i].current = strcmp("true", (char*)value) == 0;
                xmlFree(value);
            }

            // location
            snprintf(xpathBuffer, ATMOS_SIMPLE_HEADER_MAX,
                    "//cos:replicas/cos:replica[%d]/cos:location", xpathIndex);
            value = AtmosUtil_select_single_node_value(doc, BAD_CAST xpathBuffer, 1);
            if(value) {
                strncpy(self->replicas[i].location, (char*)value, ATMOS_SIMPLE_HEADER_MAX);
                xmlFree(value);
            }

            // storageType
            snprintf(xpathBuffer, ATMOS_SIMPLE_HEADER_MAX,
                    "//cos:replicas/cos:replica[%d]/cos:storageType", xpathIndex);
            value = AtmosUtil_select_single_node_value(doc, BAD_CAST xpathBuffer, 1);
            if(value) {
                strncpy(self->replicas[i].storage_type, (char*)value, ATMOS_SIMPLE_HEADER_MAX);
                xmlFree(value);
            }
        }

        xmlXPathFreeObject(xpathObj);
    }

    // Retention Enabled
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:retention/cos:enabled", 1);
    if(value) {
        self->retention_enabled = strcmp("true", (char*)value) == 0;
        xmlFree(value);
    }

    // Retention End
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:retention/cos:endAt", 1);
    if(value) {
        if(strlen((char*)value)>0) {
            self->retention_end = AtmosUtil_parse_xml_datetime((char*)value);
        }
        xmlFree(value);
    }

    // Expiration Enabled
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:expiration/cos:enabled", 1);
    if(value) {
        self->expiration_enabled = strcmp("true", (char*)value) == 0;
        xmlFree(value);
    }

    // Expiration End
    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:expiration/cos:endAt", 1);
    if(value) {
        if(strlen((char*)value)>0) {
            self->expiration_end = AtmosUtil_parse_xml_datetime((char*)value);
        }
        xmlFree(value);
    }
    xmlFreeDoc(doc);

}

/**
 * Processes the response headers for get object info.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_get_info_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    AtmosGetObjectInfoResponse_parse((AtmosGetObjectInfoResponse*)response,
            response->body, (size_t)response->content_length);
}

void AtmosFilter_set_pagination_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosPaginatedRequest *req = (AtmosPaginatedRequest*)request;
    char buffer[ATMOS_TOKEN_MAX+64];

    // Check for token
    if(strlen(req->token) > 0) {
        snprintf(buffer, ATMOS_TOKEN_MAX+64,
                ATMOS_HEADER_TOKEN ": %s", req->token);
        RestRequest_add_header(request, buffer);
    }

    // See if limit was specified
    if(req->limit > 0) {
        snprintf(buffer, ATMOS_TOKEN_MAX+64, ATMOS_HEADER_LIMIT ": %d",
                req->limit);
        RestRequest_add_header((RestRequest*)request, buffer);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}


void AtmosFilter_list_directory(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosListDirectoryRequest *req = (AtmosListDirectoryRequest*)request;
    AtmosListDirectoryResponse *res = (AtmosListDirectoryResponse*)response;
    const char *token;
    char *tag_header;
    size_t tag_header_size;
    int i;
    int utf8;
    CURL *curl = NULL;

    utf8 = ((AtmosClient*)rest)->enable_utf8_metadata;

    if(utf8) {
        RestRequest_add_header(request, ATMOS_HEADER_UTF8 ": true");
        curl = curl_easy_init();
    }

    // Check the include metadata flag
    if(req->include_meta) {
        RestRequest_add_header(request, ATMOS_HEADER_INCLUDE_META ": true");

        // If include metadata was used, the tags might also be limited.
        if(req->user_tag_count > 0) {
            tag_header = NULL;
            tag_header_size = 0;
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    ATMOS_HEADER_USER_TAGS ": ");
            if(utf8) {
                tag_header = AtmosUtil_cstring_append_utf8(tag_header,
                        &tag_header_size, req->user_tags[0], curl);
            } else {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    req->user_tags[0]);
            }
            for(i=1; i<req->user_tag_count; i++) {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        ", ");
                if(utf8) {
                    tag_header = AtmosUtil_cstring_append_utf8(tag_header,
                            &tag_header_size, req->user_tags[i], curl);
                } else {
                    tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        req->user_tags[i]);
                }
            }

            RestRequest_add_header((RestRequest*)request, tag_header);
            free(tag_header);
        }

        if(req->system_tag_count > 0) {
            tag_header = NULL;
            tag_header_size = 0;
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    ATMOS_HEADER_SYSTEM_TAGS ": ");
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    req->system_tags[0]);
            for(i=1; i<req->system_tag_count; i++) {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        ", ");
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        req->system_tags[i]);
            }

            RestRequest_add_header((RestRequest*)request, tag_header);
            free(tag_header);
        }
    }
    if(curl) {
        curl_easy_cleanup(curl);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    AtmosListDirectoryResponse_parse(res,
            response->body, (size_t)response->content_length);

    // Check for token
    token = RestResponse_get_header_value(response, ATMOS_HEADER_TOKEN);
    if(token) {
        // Valid since this is a pointer into the response headers.
        res->token = token;
    }

}

AtmosListDirectoryResponse*
AtmosListDirectoryResponse_init(AtmosListDirectoryResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosListDirectoryResponse, AtmosReadObjectResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_DIRECTORY_RESPONSE;

    return self;
}

void
AtmosListDirectoryResponse_destroy(AtmosListDirectoryResponse *self) {
    // Free the directory contents.
    if(self->entries) {
        int i;
        for(i=0; i<self->entry_count; i++) {
            AtmosDirectoryEntry_destroy(&(self->entries[i]));
        }
        free(self->entries);
    }

    OBJECT_ZERO(self, AtmosListDirectoryResponse, AtmosReadObjectResponse);

    AtmosReadObjectResponse_destroy((AtmosReadObjectResponse*)self);
}


static void
AtmosObjectMetadata_parse_system_meta(AtmosObjectMetadata *self,
        xmlNode *sysmeta) {
    char name[ATMOS_META_NAME_MAX];
    char value[ATMOS_META_VALUE_MAX];
    xmlNode *child;
    int listable;

    for(child = sysmeta->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!(strcmp(DIR_NODE_METADATA, (char*)child->name))) {
            name[0] = 0;
            value[0] = 0;
            AtmosUtil_parse_meta(child, name, value, &listable);
            AtmosUtil_set_system_meta_entry(&(self->system_metadata),
                    name, value, 0, NULL);
        } else {
            ATMOS_WARN("Unexpected node in %s: %s",
                    DIR_NODE_SYSTEM_METADATA_LIST, (char*)child->name);
        }
    }
}

static void
AtmosObjectMetadata_parse_usermeta(AtmosObjectMetadata *self,
        xmlNode *usermeta) {

    char name[ATMOS_META_NAME_MAX];
    char value[ATMOS_META_VALUE_MAX];
    int listable;
    xmlNode *child;
    int listable_count = 0, regular_count = 0;


    // First go through and count number of listable and regular
    for(child = usermeta->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!(strcmp(DIR_NODE_METADATA, (char*)child->name))) {
            name[0] = 0;
            value[0] = 0;
            listable = 0;
            AtmosUtil_parse_meta(child, name, value,
                    &listable);
            if(listable) {
                listable_count++;
            } else {
                regular_count++;
            }
        } else {
            ATMOS_WARN("Unexpected node in %s: %s",
                    DIR_NODE_USER_METADATA_LIST, (char*)child->name);
        }
    }

    if(regular_count>0) {
        self->meta = calloc(regular_count, sizeof(AtmosMetadata));
        self->meta_count = regular_count;
    }
    if(listable_count>0) {
        self->listable_meta = calloc(listable_count, sizeof(AtmosMetadata));
        self->listable_meta_count = listable_count;
    }

    regular_count = listable_count = 0;

    // Now go through again and copy the values.
    for(child = usermeta->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!(strcmp(DIR_NODE_METADATA, (char*)child->name))) {
            name[0] = 0;
            value[0] = 0;
            listable = 0;
            AtmosUtil_parse_meta(child, name, value, &listable);
            if(listable) {
                strncpy(self->listable_meta[listable_count].name,
                        name, ATMOS_META_NAME_MAX);
                strncpy(self->listable_meta[listable_count].value,
                        value, ATMOS_META_VALUE_MAX);
                listable_count++;
            } else {
                strncpy(self->meta[regular_count].name,
                        name, ATMOS_META_NAME_MAX);
                strncpy(self->meta[regular_count].value,
                        value, ATMOS_META_VALUE_MAX);
                regular_count++;
            }
        } else {
            ATMOS_WARN("Unexpected node in %s: %s",
                    DIR_NODE_USER_METADATA_LIST, (char*)child->name);
        }
    }
}

static void
AtmosDirectoryEntry_parse_entry(AtmosDirectoryEntry *entry,
        xmlNode *entrynode) {
    xmlNode *child = NULL;
    xmlChar *value;

    for(child = entrynode->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!strcmp((char*)child->name, DIR_NODE_OBJECT_ID)) {
            value = xmlNodeGetContent(child);
            if(!value) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(entry->object_id, (char*)value,
                        ATMOS_OID_LENGTH);
                xmlFree(value);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_FILE_TYPE)) {
            value = xmlNodeGetContent(child);
            if(!value) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                if(!strcmp(ATMOS_TYPE_DIRECTORY, (char*)value)) {
                    entry->type = ATMOS_TYPE_DIRECTORY;
                } else if(!strcmp(ATMOS_TYPE_REGULAR, (char*)value)) {
                    entry->type = ATMOS_TYPE_REGULAR;
                } else {
                    ATMOS_WARN("Unknown object type %s\n", (char*)value);
                }
                xmlFree(value);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_FILE_NAME)) {
            value = xmlNodeGetContent(child);
            if(!value) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(entry->filename, (char*)value,
                        ATMOS_PATH_MAX);
                xmlFree(value);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_SYSTEM_METADATA_LIST)) {
            AtmosObjectMetadata_parse_system_meta((AtmosObjectMetadata*)entry,
                    child);
        } else if(!strcmp((char*)child->name, DIR_NODE_USER_METADATA_LIST)) {
            AtmosObjectMetadata_parse_usermeta((AtmosObjectMetadata*)entry,
                    child);
        }
    }
}


void
AtmosListDirectoryResponse_parse(AtmosListDirectoryResponse *self,
        const char *xml, size_t xml_size) {
    xmlDocPtr doc;
    xmlXPathObjectPtr xpathObjDirectoryEntry;
    xmlNodeSetPtr xpathNodeSetDirectoryEntry;
    int i;
    int entrycount;

    /*
     * The document being in memory, it have no base per RFC 2396,
     * and the "noname.xml" argument will serve as its base.
     */
    doc = xmlReadMemory(xml, (int)xml_size,
            "noname.xml", NULL, 0);
    if (doc == NULL) {
        ATMOS_ERROR("Failed to parse list directory response: %s\n", xml);
        return;
    }

    // Check response
    i = AtmosUtil_count_nodes(doc, BAD_CAST "//cos:DirectoryList", 1);
    if(i == 0) {
        // No results?
        ATMOS_WARN("No directory list found in response: %s\n", xml);
        xmlFreeDoc(doc);
        return;
    }

    // See how many entries there are.
    xpathObjDirectoryEntry = AtmosUtil_select_nodes(doc, BAD_CAST "//cos:DirectoryEntry", 1);
    if(!xpathObjDirectoryEntry) {
        // Empty directory
        xmlFreeDoc(doc);
        return;
    }
    xpathNodeSetDirectoryEntry = xpathObjDirectoryEntry->nodesetval;
    entrycount = xpathNodeSetDirectoryEntry->nodeNr;
    if(entrycount == 0) {
        // Empty directory
        xmlXPathFreeObject(xpathObjDirectoryEntry);
        xmlFreeDoc(doc);
        return;
    }

    // Allocate directory entries
    self->entries = calloc(entrycount, sizeof(AtmosDirectoryEntry));
    self->entry_count = entrycount;

    // Iterate through entries
    for(i=0; i<entrycount; i++) {
        xmlNode *entrynode = xpathNodeSetDirectoryEntry->nodeTab[i];
        AtmosDirectoryEntry_init(&(self->entries[i]));

        // Parse entry fields
        AtmosDirectoryEntry_parse_entry(&(self->entries[i]), entrynode);
    }

    // Cleanup
    xmlXPathFreeObject(xpathObjDirectoryEntry);
    xmlFreeDoc(doc);
}

AtmosObjectMetadata*
AtmosObjectMetadata_init(AtmosObjectMetadata *self) {
    Object_init_with_class_name((Object*)self, CLASS_ATMOS_OBJECT_METADATA);
    OBJECT_ZERO(self, AtmosObjectMetadata, Object);

    return self;
}

void
AtmosObjectMetadata_destroy(AtmosObjectMetadata *self) {
    OBJECT_ZERO(self, AtmosObjectMetadata, Object);

    Object_destroy((Object*)self);
}


AtmosDirectoryEntry*
AtmosDirectoryEntry_init(AtmosDirectoryEntry *self) {
    AtmosObjectMetadata_init((AtmosObjectMetadata*)self);
    OBJECT_ZERO(self, AtmosDirectoryEntry, AtmosObjectMetadata);
    ((Object*)self)->class_name = CLASS_ATMOS_DIRECTORY_ENTRY;

    return self;
}

void
AtmosDirectoryEntry_destroy(AtmosDirectoryEntry *self) {
    OBJECT_ZERO(self, AtmosDirectoryEntry, AtmosObjectMetadata);

    AtmosObjectMetadata_destroy((AtmosObjectMetadata*)self);
}

const char *
AtmosDirectoryEntry_get_metadata_value(AtmosDirectoryEntry *self,
        const char *name, int listable) {
    return AtmosUtil_get_metadata_value(name,
            listable?self->parent.listable_meta:self->parent.meta,
            listable?self->parent.listable_meta_count:self->parent.meta_count);
}

AtmosListDirectoryRequest*
AtmosListDirectoryRequest_init(AtmosListDirectoryRequest *self,
        const char *path) {
    char uri[ATMOS_PATH_MAX + 15];

    if (path[0] != '/') {
        ATMOS_ERROR("Path must start with a '/': %s\n", path);
        return NULL;
    }
    if(path[strlen(path)-1] != '/') {
        ATMOS_ERROR("Directory path must end with a '/': %s\n", path);
        return NULL;
    }

    snprintf(uri, ATMOS_PATH_MAX + 15, "/rest/namespace%s", path);

    AtmosPaginatedRequest_init((AtmosPaginatedRequest*) self, uri, HTTP_GET);

    OBJECT_ZERO(self, AtmosListDirectoryRequest, AtmosPaginatedRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_DIRECTORY_REQUEST;

    return self;
}

void
AtmosListDirectoryRequest_destroy(AtmosListDirectoryRequest *self) {
    OBJECT_ZERO(self, AtmosListDirectoryRequest, AtmosPaginatedRequest);
    AtmosPaginatedRequest_destroy((AtmosPaginatedRequest*)self);
}

void
AtmosListDirectoryRequest_add_user_tag(AtmosListDirectoryRequest *self,
        const char *tag) {
    strncpy(self->user_tags[self->user_tag_count++], tag, ATMOS_META_NAME_MAX);
}

void
AtmosListDirectoryRequest_add_system_tag(AtmosListDirectoryRequest *self,
        const char *tag) {
    strncpy(self->system_tags[self->system_tag_count++], tag, ATMOS_META_NAME_MAX);

}


AtmosGetListableTagsRequest*
AtmosGetListableTagsRequest_init(AtmosGetListableTagsRequest *self,
        const char *parent_tag) {
    AtmosPaginatedRequest_init((AtmosPaginatedRequest*)self,
            "/rest/objects?listabletags", HTTP_GET);
    OBJECT_ZERO(self, AtmosGetListableTagsRequest, AtmosPaginatedRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_GET_LISTABLE_TAGS_REQUEST;
    if(parent_tag) {
        strncpy(self->parent_tag, parent_tag, ATMOS_PATH_MAX);
    }

    return self;
}

void
AtmosGetListableTagsRequest_destroy(AtmosGetListableTagsRequest *self) {
    OBJECT_ZERO(self, AtmosGetListableTagsRequest, AtmosPaginatedRequest);
    AtmosPaginatedRequest_destroy((AtmosPaginatedRequest*)self);
}

AtmosGetListableTagsResponse*
AtmosGetListableTagsResponse_init(AtmosGetListableTagsResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosGetListableTagsResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_GET_LISTABLE_TAGS_RESPONSE;

    return self;
}

void
AtmosGetListableTagsResponse_destroy(AtmosGetListableTagsResponse *self) {
    if(self->tags) {
        int i;
        for(i=0; i<self->tag_count; i++) {
            if(self->tags[i]) {
                free(self->tags[i]);
            }
        }
        free(self->tags);
    }
    OBJECT_ZERO(self, AtmosGetListableTagsResponse, AtmosResponse);

    AtmosResponse_destroy((AtmosResponse*)self);
}

void AtmosFilter_get_listable_tags(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosGetListableTagsRequest *req = (AtmosGetListableTagsRequest*)request;
    AtmosGetListableTagsResponse *res = (AtmosGetListableTagsResponse*)response;
    char parent_header[ATMOS_PATH_MAX+12];
    const char *token;
    int tags_alloc;
    const char *tag;
    int utf8 = 0;
    CURL *curl = NULL;

    if(req->parent_tag[0]) {
        snprintf(parent_header, ATMOS_PATH_MAX+12,
                ATMOS_HEADER_TAGS ": %s", req->parent_tag);
        RestRequest_add_header(request, parent_header);
    }

    if(((AtmosClient*)rest)->enable_utf8_metadata) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    const char *utf_mode = RestResponse_get_header_value(response, ATMOS_HEADER_UTF8);
    if(utf_mode && !strcmp("true", utf_mode)) {
        utf8 = 1;
        curl = curl_easy_init();
    }

    // Parse the tags.
    tags_alloc = 100;
    res->tag_count = 0;
    res->tags = calloc(tags_alloc, sizeof(char*));
    tag = RestResponse_get_header_value(response, ATMOS_HEADER_LISTABLE_TAGS);
    while(tag && *tag) {
        int c = 0;
        if(*tag == ' ') {
            // skip whitespace
            tag++;
            continue;
        }
        const char *comma = strchr(tag, ',');
        if(!comma) {
            // End of string reached.  Take what's left.
            c = (int)strlen(tag);
        } else {
            c = (int)(comma - tag);
        }
        if(res->tag_count + 1 > tags_alloc) {
            char **newalloc;
            tags_alloc += 100;
            newalloc = realloc(res->tags, tags_alloc * sizeof(char*));
            if(!newalloc) {
                ATMOS_ERROR("Failed to realloc %d tags\n", tags_alloc);
                return;
            }
            res->tags = newalloc;
        }

        if(utf8) {
            int decoded_size;
            char *decoded = curl_easy_unescape(curl, tag, c, &decoded_size);
            res->tags[res->tag_count++] = strndup(decoded, decoded_size);
            curl_free(decoded);
        } else {
            res->tags[res->tag_count++] = strndup(tag, c);
        }

        if(comma) {
            tag = comma+1;
        } else {
            tag = NULL;
        }
    }


    // Check for token
    token = RestResponse_get_header_value(response, ATMOS_HEADER_TOKEN);
    if(token) {
        // Valid since this is a pointer into the response headers and will
        // be freed at the same time as this object.
        res->token = token;
    }

    if(curl) {
        curl_easy_cleanup(curl);
    }
}

AtmosPaginatedRequest*
AtmosPaginatedRequest_init(AtmosPaginatedRequest *self, const char *uri,
        enum http_method method) {
    RestRequest_init((RestRequest*)self, uri, method);
    OBJECT_ZERO(self, AtmosPaginatedRequest, RestRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_PAGINATED_REQUEST;

    return self;
}

void
AtmosPaginatedRequest_destroy(AtmosPaginatedRequest *self) {
    OBJECT_ZERO(self, AtmosPaginatedRequest, RestRequest);

    RestRequest_destroy((RestRequest*)self);
}

AtmosListObjectsRequest*
AtmosListObjectsRequest_init(AtmosListObjectsRequest *self, const char *tag) {
    AtmosPaginatedRequest_init((AtmosPaginatedRequest*) self, "/rest/objects",
            HTTP_GET);

    OBJECT_ZERO(self, AtmosListDirectoryRequest, AtmosPaginatedRequest);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_DIRECTORY_REQUEST;

    strncpy(self->tag, tag, ATMOS_PATH_MAX);

    return self;
}

void
AtmosListObjectsRequest_destroy(AtmosListObjectsRequest *self) {
    OBJECT_ZERO(self, AtmosListDirectoryRequest, AtmosPaginatedRequest);
    AtmosPaginatedRequest_destroy((AtmosPaginatedRequest*)self);
}


AtmosObjectListing*
AtmosObjectListing_init(AtmosObjectListing *self) {
    AtmosObjectMetadata_init((AtmosObjectMetadata*)self);
    OBJECT_ZERO(self, AtmosObjectListing, AtmosObjectMetadata);

    ((Object*)self)->class_name = CLASS_ATMOS_OBJECT_LISTING;

    return self;
}

void
AtmosObjectListing_destroy(AtmosObjectListing *self) {
    OBJECT_ZERO(self, AtmosObjectListing, AtmosObjectMetadata);

    AtmosObjectMetadata_destroy((AtmosObjectMetadata*)self);
}

AtmosListObjectsResponse*
AtmosListObjectsResponse_init(AtmosListObjectsResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosListObjectsResponse, AtmosResponse);

    ((Object*)self)->class_name = CLASS_ATMOS_LIST_OBJECTS_RESPONSE;

    return self;
}

void
AtmosListObjectsResponse_destroy(AtmosListObjectsResponse *self) {
    if(self->results) {
        free(self->results);
    }
    OBJECT_ZERO(self, AtmosListObjectsResponse, AtmosResponse);

    AtmosResponse_destroy((AtmosResponse*)self);
}

static void
AtmosObjectListing_parse_entry(AtmosObjectListing *entry,
        xmlNode *entrynode) {
    xmlNode *child = NULL;
    xmlChar *value;

    for(child = entrynode->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!strcmp((char*)child->name, DIR_NODE_OBJECT_ID)) {
            value = xmlNodeGetContent(child);
            if(!value) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(entry->object_id, (char*)value,
                        ATMOS_OID_LENGTH);
                xmlFree(value);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_SYSTEM_METADATA_LIST)) {
            AtmosObjectMetadata_parse_system_meta((AtmosObjectMetadata*)entry,
                    child);
        } else if(!strcmp((char*)child->name, DIR_NODE_USER_METADATA_LIST)) {
            AtmosObjectMetadata_parse_usermeta((AtmosObjectMetadata*)entry,
                    child);
        }
    }
}


void
AtmosListObjectsResponse_parse(AtmosListObjectsResponse *self,
        const char *xml, size_t xml_size) {
    xmlDocPtr doc;
    xmlXPathObjectPtr xpathObjDirectoryEntry;
    xmlNodeSetPtr xpathNodeSetDirectoryEntry;
    int i;
    int entrycount;

    /*
     * The document being in memory, it have no base per RFC 2396,
     * and the "noname.xml" argument will serve as its base.
     */
    doc = xmlReadMemory(xml, (int)xml_size,
            "noname.xml", NULL, 0);
    if (doc == NULL) {
        ATMOS_ERROR("Failed to parse list objects response: %s\n", xml);
        return;
    }

    // Check response
    i = AtmosUtil_count_nodes(doc, BAD_CAST "//cos:ListObjectsResponse", 1);
    if(i == 0) {
        // No results?
        ATMOS_WARN("No object list found in response: %s\n", xml);
        xmlFreeDoc(doc);
        return;
    }

    // See how many entries there are.
    xpathObjDirectoryEntry = AtmosUtil_select_nodes(doc, BAD_CAST "//cos:Object", 1);
    if(!xpathObjDirectoryEntry) {
        // Empty directory
        xmlFreeDoc(doc);
        return;
    }
    xpathNodeSetDirectoryEntry = xpathObjDirectoryEntry->nodesetval;
    entrycount = xpathNodeSetDirectoryEntry->nodeNr;
    if(entrycount == 0) {
        // Empty directory
        xmlXPathFreeObject(xpathObjDirectoryEntry);
        xmlFreeDoc(doc);
        return;
    }

    // Allocate directory entries
    self->results = calloc(entrycount, sizeof(AtmosObjectListing));
    self->result_count = entrycount;

    // Iterate through entries
    for(i=0; i<entrycount; i++) {
        xmlNode *entrynode = xpathNodeSetDirectoryEntry->nodeTab[i];
        AtmosObjectListing_init(&(self->results[i]));

        // Parse entry fields
        AtmosObjectListing_parse_entry(&(self->results[i]), entrynode);
    }

    // Cleanup
    xmlXPathFreeObject(xpathObjDirectoryEntry);
    xmlFreeDoc(doc);

}

void
AtmosFilter_list_objects(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosListObjectsRequest *req = (AtmosListObjectsRequest*)request;
    AtmosListObjectsResponse *res = (AtmosListObjectsResponse*)response;
    const char *token;
    char *tag_header;
    size_t tag_header_size;
    int i;
    char tagbuf[1][ATMOS_META_NAME_MAX];
    int utf8;
    CURL *curl = NULL;

    utf8 = ((AtmosClient*)rest)->enable_utf8_metadata;

    if(utf8) {
        RestRequest_add_header(request, ATMOS_HEADER_UTF8 ": true");
        curl = curl_easy_init();
    }

    // Set the tag header
    strncpy(tagbuf[0], req->tag, ATMOS_META_NAME_MAX);
    AtmosUtil_set_tags_header(request, tagbuf, 1, utf8);

    // Check the include metadata flag
    if(req->include_meta) {
        RestRequest_add_header(request, ATMOS_HEADER_INCLUDE_META ": true");

        // If include metadata was used, the tags might also be limited.
        if(req->user_tag_count > 0) {
            tag_header = NULL;
            tag_header_size = 0;
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    ATMOS_HEADER_USER_TAGS ": ");
            if(utf8) {
                tag_header = AtmosUtil_cstring_append_utf8(tag_header,
                        &tag_header_size, req->user_tags[0], curl);
            } else {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    req->user_tags[0]);
            }
            for(i=1; i<req->user_tag_count; i++) {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        ", ");
                if(utf8) {
                    tag_header = AtmosUtil_cstring_append_utf8(tag_header,
                            &tag_header_size, req->user_tags[i], curl);
                } else {
                    tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        req->user_tags[i]);
                }
            }

            RestRequest_add_header((RestRequest*)request, tag_header);
            free(tag_header);
        }

        if(req->system_tag_count > 0) {
            tag_header = NULL;
            tag_header_size = 0;
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    ATMOS_HEADER_SYSTEM_TAGS ": ");
            tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                    req->system_tags[0]);
            for(i=1; i<req->system_tag_count; i++) {
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        ", ");
                tag_header = AtmosUtil_cstring_append(tag_header, &tag_header_size,
                        req->system_tags[i]);
            }

            RestRequest_add_header((RestRequest*)request, tag_header);
            free(tag_header);
        }
    }
    if(curl) {
        curl_easy_cleanup(curl);
    }

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code > 299) {
        return;
    }

    AtmosListObjectsResponse_parse(res,
            response->body, (size_t)response->content_length);

    // Check for token
    token = RestResponse_get_header_value(response, ATMOS_HEADER_TOKEN);
    if(token) {
        // Valid since this is a pointer into the response headers.
        res->token = token;
    }

}

void
AtmosListObjectsRequest_add_user_tag(AtmosListObjectsRequest *self,
        const char *tag) {
    strncpy(self->user_tags[self->user_tag_count++], tag, ATMOS_META_NAME_MAX);
}

void
AtmosListObjectsRequest_add_system_tag(AtmosListObjectsRequest *self,
        const char *tag) {
    strncpy(self->system_tags[self->system_tag_count++], tag,
            ATMOS_META_NAME_MAX);
}

const char *
AtmosObjectListing_get_metadata_value(AtmosObjectListing *self,
        const char *name, int listable) {
    return AtmosUtil_get_metadata_value(name,
            listable?self->parent.listable_meta:self->parent.meta,
            listable?self->parent.listable_meta_count:self->parent.meta_count);
}


