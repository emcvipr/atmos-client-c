/*
 * atmos_read.c
 *
 *  Created on: Nov 19, 2012
 *      Author: cwikj
 */
#include <stdlib.h>
#include <string.h>

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

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/user", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetUserMetaRequest_common_init(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

    return self;
}

void
AtmosGetUserMetaRequest_destroy(AtmosGetUserMetaRequest *self) {
    memset(((void*)self)+sizeof(RestRequest), 0,
            sizeof(AtmosGetUserMetaRequest)-sizeof(RestRequest));
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

    if(((AtmosClient*)rest)->enable_utf8_metadata) {
        RestRequest_add_header((RestRequest*)request,
                ATMOS_HEADER_UTF8 ": true");
    }

    if(req->tags && req->tag_count > 0) {
        AtmosUtil_set_tags_header(request, req->tags, req->tag_count);
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

    snprintf(uri, ATMOS_PATH_MAX+64, "/rest/namespace/%s?metadata/system", path);
    RestRequest_init((RestRequest*)self, uri, HTTP_GET);

    AtmosGetSystemMetaRequest_init_common(self);

    strncpy(self->path, path, ATMOS_PATH_MAX);

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
        AtmosUtil_set_tags_header(request, req->tags, req->tag_count);
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
    doc = xmlReadMemory(xml, xml_size,
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
                self->replicas[i].id = strtol((char*)value, NULL, 10);
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

        xmlXPathFreeNodeSetList(xpathObj);
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

