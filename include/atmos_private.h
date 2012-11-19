/*
 * atmos_private.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_PRIVATE_H_
#define ATMOS_PRIVATE_H_

char *
AtmosClient_sign(AtmosClient *self, const char *hash_string);

char *
AtmosClient_canonicalize_request(AtmosClient *self, RestRequest *request);

char *
AtmosClient_sign_request(AtmosClient *self, RestRequest *request);

/*******************************
 * Private Filter Declarations *
 *******************************/
void AtmosFilter_add_uid(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

void AtmosFilter_add_date(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

void AtmosFilter_sign_request(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

void AtmosFilter_parse_atmos_error(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Create Object (in atmos_create.c)
void AtmosFilter_parse_create_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

void AtmosFilter_set_create_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Get Service Info (in atmos_service_info.c)
void AtmosFilter_parse_service_info_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);




#endif /* ATMOS_PRIVATE_H_ */
