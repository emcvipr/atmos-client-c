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

/**
 * Request filter for paginated requests.  Sets the limit and token headers
 * if required.  Request should be a sublass of AtmosPaginatedRequest
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_pagination_headers(RestFilter *self, RestClient *rest,
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
 * Sets the HTTP headers generated from the AtmosWriteObjectRequest.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_set_write_headers(RestFilter *self, RestClient *rest,
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

// Special filter for Get ACL
/**
 * Processes the response headers for get_acl.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_get_acl_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special filter for Update Object
/**
 * Processes the headers for update_object
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_update_object(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Special methods for Get Object Info

/**
 * Parses an XML response and populates the AtmosGetObjectInfoResponse.
 * @param self the AtmosGetObjectInfoResponse object to populate.
 * @param xml the XML document to parse.
 * @param xml_size number of bytes in the XML document.
 */
void
AtmosGetObjectInfoResponse_parse(AtmosGetObjectInfoResponse *self,
        const char *xml, size_t xml_size);

/**
 * Processes the response headers for get object info.
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_get_info_response(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// Filter for list directory
/**
 * Processes request and response for list directory
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_list_directory(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Parses an XML response and populates the AtmosListDirectoryResponse.
 * @param self the AtmosListDirectoryResponse to populate
 * @param xml the XML document to parse
 * @param xml_size number of bytes in the XML document.
 */
void
AtmosListDirectoryResponse_parse(AtmosListDirectoryResponse *self,
        const char *xml, size_t xml_size);

/**
 * Parses an XML response and populates the AtmosListObjectsResponse.
 * @param self the AtmosListObjectsResponse to populate
 * @param xml the XML document to parse
 * @param xml_size number of bytes in the XML document.
 */
void
AtmosListObjectsResponse_parse(AtmosListObjectsResponse *self,
        const char *xml, size_t xml_size);

// Get listable tags filter
/**
 * Processes request and response for list directory
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_get_listable_tags(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

// List objects filter
/**
 * Processes request and response for list objects
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_list_objects(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);


// List versions
/**
 * Parses an XML response and populates the AtmosListVersionsResponse.
 * @param self the AtmosListVersionsResponse to populate
 * @param xml the XML document to parse
 * @param xml_size number of bytes in the XML document.
 */
void
AtmosListVersionsResponse_parse(AtmosListVersionsResponse *self,
        const char *xml, size_t xml_size);
/**
 * Processes request and response for list versions
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_list_versions(RestFilter *self, RestClient *rest,
        RestRequest *request, RestResponse *response);

/**
 * Processes response for create version
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_create_version_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response);

/**
 * Processes request and response for create access token
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_create_access_token(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response);

/**
 * Processes response for list access tokens
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_list_access_tokens_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response);

/**
 * Processes response for get token info
 * @param self the current filter element in the chain.
 * @param rest the RestClient executing the request.
 * @param request the RestRequest to execute.
 * @param response the RestResponse receiving the response.
 */
void AtmosFilter_parse_get_token_info_response(RestFilter *self,
        RestClient *rest, RestRequest *request, RestResponse *response);



#endif /* ATMOS_PRIVATE_H_ */
