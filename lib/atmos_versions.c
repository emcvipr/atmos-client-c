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
#include "atmos_private.h"
#include "atmos_util.h"

AtmosCreateVersionRequest*
AtmosCreateVersionRequest_init(AtmosCreateVersionRequest *self,
        const char *object_id) {
    char uri[ATMOS_PATH_MAX];
    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s?versions", object_id);

    RestRequest_init((RestRequest*)self, uri, HTTP_POST);

    OBJECT_ZERO(self, AtmosCreateVersionRequest, RestRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_VERSION_REQUEST;

    return self;
}

void
AtmosCreateVersionRequest_destroy(AtmosCreateVersionRequest *self) {
    OBJECT_ZERO(self, AtmosCreateVersionRequest, RestRequest);
    RestRequest_destroy((RestRequest*)self);
}

AtmosCreateVersionResponse*
AtmosCreateVersionResponse_init(AtmosCreateVersionResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosCreateVersionResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_CREATE_VERSION_RESPONSE;

    return self;
}

void
AtmosCreateVersionResponse_destroy(AtmosCreateVersionResponse *self) {
    OBJECT_ZERO(self, AtmosCreateVersionResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}



AtmosListVersionsRequest*
AtmosListVersionsRequest_init(AtmosListVersionsRequest *self,
        const char *object_id) {
    char uri[ATMOS_PATH_MAX];
    snprintf(uri, ATMOS_PATH_MAX, "/rest/objects/%s?versions", object_id);
    AtmosPaginatedRequest_init((AtmosPaginatedRequest*)self, uri, HTTP_GET);

    OBJECT_ZERO(self, AtmosListVersionsRequest, AtmosPaginatedRequest);

    ((Object*)self)->class_name = CLASS_ATMOS_LIST_VERSIONS_REQUEST;

    return self;
}

void
AtmosListVersionsRequest_destroy(AtmosListVersionsRequest *self) {
    OBJECT_ZERO(self, AtmosListVersionsRequest, AtmosPaginatedRequest);
    AtmosPaginatedRequest_destroy((AtmosPaginatedRequest*)self);
}

AtmosListVersionsResponse*
AtmosListVersionsResponse_init(AtmosListVersionsResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    OBJECT_ZERO(self, AtmosListVersionsResponse, AtmosResponse);
    ((Object*)self)->class_name = CLASS_ATMOS_LIST_VERSIONS_RESPONSE;

    return self;
}

void
AtmosListVersionsResponse_destroy(AtmosListVersionsResponse *self) {
    OBJECT_ZERO(self, AtmosListVersionsResponse, AtmosResponse);
    AtmosResponse_destroy((AtmosResponse*)self);
}

static void
AtmosVersion_init(AtmosVersion *self) {
    memset(self, 0, sizeof(AtmosVersion));
}

static void
AtmosVersion_parse_entry(AtmosVersion *entry,
        xmlNode *entrynode) {
    xmlNode *child = NULL;
    xmlChar *value;

    for(child = entrynode->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!strcmp((char*)child->name, VER_NODE_OID)) {
            value = xmlNodeGetContent(child);
            if(!value) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(entry->object_id, (char*)value,
                        ATMOS_OID_LENGTH);
                xmlFree(value);
            }
        } else if(!strcmp((char*)child->name, VER_NODE_VER_NUM)) {
                value = xmlNodeGetContent(child);
                if(!value) {
                    ATMOS_WARN("No value found for %s\n", child->name);
                } else {
                    entry->version_number = (int)strtol((char*)value, NULL, 10);
                    xmlFree(value);
                }
        } else if(!strcmp((char*)child->name, VER_NODE_ITIME)) {
                value = xmlNodeGetContent(child);
                if(!value) {
                    ATMOS_WARN("No value found for %s\n", child->name);
                } else {
                    entry->itime = AtmosUtil_parse_xml_datetime((char*)value);
                    xmlFree(value);
                }
        } else {
            ATMOS_WARN("Unknown node in list versions response: %s\n",
                    (char*)child->name);
        }
    }
}



void
AtmosListVersionsResponse_parse(AtmosListVersionsResponse *self,
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
    i = AtmosUtil_count_nodes(doc, BAD_CAST "//cos:ListVersionsResponse", 1);
    if(i == 0) {
        // No results?
        ATMOS_WARN("No object list found in response: %s\n", xml);
        xmlFreeDoc(doc);
        return;
    }

    // See how many entries there are.
    xpathObjDirectoryEntry = AtmosUtil_select_nodes(doc, BAD_CAST "//cos:Ver", 1);
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
    self->versions = calloc(entrycount, sizeof(AtmosVersion));
    self->version_count = entrycount;

    // Iterate through entries
    for(i=0; i<entrycount; i++) {
        xmlNode *entrynode = xpathNodeSetDirectoryEntry->nodeTab[i];
        AtmosVersion_init(&(self->versions[i]));

        // Parse entry fields
        AtmosVersion_parse_entry(&(self->versions[i]), entrynode);
    }

    // Cleanup
    xmlXPathFreeObject(xpathObjDirectoryEntry);
    xmlFreeDoc(doc);

}

void AtmosFilter_list_versions(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosListVersionsResponse *res = (AtmosListVersionsResponse*)response;
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

    AtmosListVersionsResponse_parse(res,
            response->body, (size_t)response->content_length);

    // Check for token
    token = RestResponse_get_header_value(response, ATMOS_HEADER_TOKEN);
    if(token) {
        // Valid since this is a pointer into the response headers.
        res->token = token;
    }
}

void AtmosFilter_parse_create_version_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response) {
    const char *location;
    const char *type;
    size_t location_len;

    type = RestRequest_get_header_value(request, HTTP_HEADER_CONTENT_TYPE);
    if(!type) {
        // Set a content type header so curl doesn't make one up.
        RestRequest_add_header(request,
                HTTP_HEADER_CONTENT_TYPE ": application/octet-stream");
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
    strncpy(((AtmosCreateVersionResponse*)response)->version_id,
            location+ATMOS_OID_LOCATION_PREFIX_SIZE,
            location_len-ATMOS_OID_LOCATION_PREFIX_SIZE);
}

