/*
 * atmos_private.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_PRIVATE_H_
#define ATMOS_PRIVATE_H_

/**
 * List of system metadata names for filtering from user metadata
 */
extern const char const *ATMOS_SYSTEM_META_NAMES[];
/**
 * Number of system metadata names.
 */
extern const int ATMOS_SYSTEM_META_NAME_COUNT;

/**
 * Signs a request using the client's credentials.
 * @param self the AtmosClient to use for signing.
 * @param hash_string the canonicalized string to sign.
 * @return the base-64 encoded signature.  Caller is responsible for calling
 * free() on the pointer.
 */
char *
AtmosClient_sign(AtmosClient *self, const char *hash_string);

/**
 * Takes a RestRequest object and generates an Atmos canonicalized string
 * sutiable for signing.
 * @param self the AtmosClient to use to canonicalize the request.
 * @param request the RestRequest to canonicalize.
 * @return the canonicalized request string.  The caller is responsible for
 * calling free() on the pointer.
 */
char *
AtmosClient_canonicalize_request(AtmosClient *self, RestRequest *request);

/**
 * Signs a request using the client's credentials.
 * @param self the AtmosClient to use for signing.
 * @param request the RestRequest to canonicalize and sign.
 * @return the base-64 encoded signature.  Caller is responsible for calling
 * free() on the pointer.
 */
char *
AtmosClient_sign_request(AtmosClient *self, RestRequest *request);

/*******************************
 * Private Filter Declarations *
 *******************************/
/**
 * Adds the x-emc-uid to the request.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_add_uid(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Adds the current date to the request.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_add_date(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Generates and adds the x-emc-signature to the request.  This should be
 * executed second-to-last, just before sending the request.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_sign_request(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Checks for HTTP error codes and attempts to parse an Atmos eror code and
 * message from the response body.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_atmos_error(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Create Object (in atmos_create.c)
/**
 * Parses the AtmosCreateObjectResponse from the RestResponse.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_create_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Sets the HTTP headers generated from the AtmosCreateObjectRequest.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_create_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Get Service Info (in atmos_service_info.c)
/**
 * Parses the AtmosGetServiceInformation response from the RestResponse.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_service_info_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Read Object
/**
 * Parses the AtmosReadObjectResponse from the RestResponse
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_read_object_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);
/**
 * Sets the HTTP headers generated from the AtmosReadObjectRequest.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_read_object_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filters for Get User Metadata
/**
 * Parses the AtmosGetUserMetaResponse from the RestResponse
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_get_user_meta_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);
/**
 * Sets the HTTP headers generated from the AtmosGetUserMetaRequest.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_get_user_meta_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filter for Set User Metadata
/**
 * Sets the HTTP headers generated from the AtmosSetUserMetaRequest.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_user_meta_headers(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filter for Get System Metadata
/**
 * Processes the request and response headers for get_system_metadata.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_get_system_metadata(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);



#endif /* ATMOS_PRIVATE_H_ */
