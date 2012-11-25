/*
 * atmos_service_info.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */
#include <string.h>
#include <stdlib.h>

#include <libxml/tree.h>
#include <libxml/parser.h>


#include "atmos.h"
#include "atmos_util.h"

AtmosServiceInfoResponse*
AtmosServiceInfoResponse_init(AtmosServiceInfoResponse *self) {
    AtmosResponse_init((AtmosResponse*)self);
    self->version[0] = 0;
    self->utf8_metadata_supported = 0;

    return self;
}

void
AtmosServiceInfoResponse_destroy(AtmosServiceInfoResponse *self) {
    self->version[0] = 0;
    AtmosResponse_destroy((AtmosResponse*)self);
}

void AtmosFilter_parse_service_info_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosServiceInfoResponse *res = (AtmosServiceInfoResponse*)response;

    // Do nothing on the request, just pass to the next filter.
    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // Now we're on the response.

    // Check to make sure we had success before parsing.
    if(response->http_code != 200) {
        return;
    }
    xmlDocPtr doc;
    xmlChar *value;

    /*
     * The document being in memory, it have no base per RFC 2396,
     * and the "noname.xml" argument will serve as its base.
     */
    doc = xmlReadMemory(response->body, response->content_length,
            "noname.xml", NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse error response\n");
        return;
    }

    value = AtmosUtil_select_single_node_value(doc, BAD_CAST "//cos:Atmos", 1);
    if(value) {
        strcpy(res->version, (char*)value);
        xmlFree(value);
    }

    // Check for UTF-8 support
    const char *utf8 = RestResponse_get_header_value(response,
            ATMOS_HEADER_SUPPORT_UTF8);
    if(utf8) {
        if(!strcmp(utf8, "true")) {
            res->utf8_metadata_supported = 1;
        }
    }

    xmlFreeDoc(doc);
}
