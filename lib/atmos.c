/*
 * atmos.c
 *
 *  Created on: Oct 19, 2012
 *      Author: cwikj
 */
#include <string.h>
#include <stdlib.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "atmos.h"
#include "atmos_private.h"
#include "atmos_util.h"


/**********************************
 * Private filter implementations *
 **********************************/

void AtmosFilter_add_uid(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    AtmosClient *atmos = (AtmosClient*)rest;
    char header[ATMOS_SIMPLE_HEADER_MAX];
    snprintf(header, ATMOS_SIMPLE_HEADER_MAX, "%s:%s",
            ATMOS_HEADER_UID, atmos->uid);
    RestRequest_add_header(request, header);

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}

void AtmosFilter_add_date(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {

    char header[ATMOS_SIMPLE_HEADER_MAX];
    char date[ATMOS_SIMPLE_HEADER_MAX];

    AtmosUtil_get_date(date);

    snprintf(header, ATMOS_SIMPLE_HEADER_MAX, "%s:%s",
            HTTP_HEADER_DATE, date);
    RestRequest_add_header(request, header);

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}


void AtmosFilter_sign_request(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {
    char header[ATMOS_SIMPLE_HEADER_MAX];

    char *signature = AtmosClient_sign_request((AtmosClient*)rest, request);
    snprintf(header, ATMOS_SIMPLE_HEADER_MAX, "%s:%s", ATMOS_HEADER_SIGNATURE,
            signature);
    RestRequest_add_header(request, header);
    free(signature);

    // Pass to the next filter
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }
}

typedef struct {
    FILE *f;
    size_t content_length;
    size_t bytes_read;
} XMLFileParseData;

/**
 * Reads XML from a file, taking care to only read content_length number of
 * bytes.
 */
int AtmosFilter_file_read_xml(void * context, char * buffer, int len) {
    XMLFileParseData *data = (XMLFileParseData*)context;
    size_t bytes_to_read = len;
    size_t read;

    if(bytes_to_read + data->bytes_read > data->content_length) {
        bytes_to_read = data->content_length - data->bytes_read;
    }

    read = fread(buffer, 1, bytes_to_read, data->f);

    bytes_to_read += read;

    return read;
}


void AtmosFilter_parse_atmos_error(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response) {

    AtmosResponse *atmos_response = (AtmosResponse*)response;

    // Do nothing on the request, just pass to the next filter.
    if(self->next) {
        ((rest_http_filter)self->next->func)(self->next, rest, request, response);
    }

    // HEAD, PUT, and DELETE requests do not have a body to parse.
    if(request->method == HTTP_DELETE || request->method == HTTP_PUT
            || request->method == HTTP_HEAD) {
        return;
    }

    // If HTTP code >399, there's probably an associated Atmos error in the
    // response body.
    if(response->http_code > 399) {
        xmlDocPtr doc = NULL; /* the resulting document tree */
        xmlNodePtr root = NULL;

        if(response->file_body) {
            XMLFileParseData data;

            data.f = response->file_body;
            data.bytes_read = 0;
            data.content_length = response->content_length;

            // the response was written to a file!  Try to parse from there!
            fseeko(response->file_body, response->file_body_start_pos, SEEK_SET);
            doc = xmlReadIO(AtmosFilter_file_read_xml, NULL, &data,
                    "noname.xml", NULL, 0);
            if (doc == NULL) {
                fprintf(stderr, "Failed to parse error response\n");
                return;
            }
        } else {

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

        }

        // We have a document.  Try to find the code and message.
        root = xmlDocGetRootElement(doc);

        // Check to ensure the root node is what we expect.
        if(xmlStrcmp(BAD_CAST ATMOS_ERROR_ROOT_NODE, root->name)) {
            fprintf(stderr,
                    "Failed to parse error response; root node was %s\n",
                    root->name);
        } else {
            // Looks good.
            xmlChar *value;

            value = AtmosUtil_select_single_node_value(doc,
                    BAD_CAST ATMOS_ERROR_CODE_XPATH, 0);
            if(value) {
                atmos_response->atmos_error = strtol((char*)value, NULL, 10);
                xmlFree(value);
            }
            value = AtmosUtil_select_single_node_value(doc,
                    BAD_CAST ATMOS_ERROR_MESSAGE_XPATH, 0);
            if(value) {
                strcpy(atmos_response->atmos_error_message, (char*)value);
                xmlFree(value);
            }
        }

        xmlFreeDoc(doc);

    }
}

