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
#ifndef ATMOS_CLIENT_H_
#define ATMOS_CLIENT_H_

/**
 * @file atmos_client.h
 * @brief This module contains the base AtmosClient class that is used as an
 * entrypoint into the SDK.
 * @defgroup AtmosClient AtmosClient
 * @brief This module contains the base AtmosClient class that is used as an
 * entrypoint into the SDK.
 * @{
 */

#include "atmos.h"

/**
 * The AtmosClient object contains the information and settings used to connect
 * to an Atmos cloud.
 */
typedef struct {
    /** Parent class */
    RestClient parent;
    /** The Atmos UID to connect as */
    char uid[ATMOS_UID_MAX];
    /** The shared secret key for the uid */
    char secret[ATMOS_SECRET_MAX];
    /** If nonzero, extra debugging will be displayed for calcualting the
     * signature.  Useful for debugging signature mismatch errors */
    int signature_debug;
    /** If nonzero, metadata will be encoded when reading and writing from
     * Atmos.  This allows for UTF-8 characters to be used in metadata names
     * and values.  It can, however, limit the amount of metadata returned from
     * the server since response headers are capped at 8kB.
     */
    int enable_utf8_metadata;
} AtmosClient;

/**
 * Initializes a new AtmosClient object.
 * @param self pointer to the AtmosClient object to initialize.
 * @param host hostname or IP address of the Atmos access point,
 * e.g. http://api.atmosonline.com.
 * @param port the TCP port to connect to.  Use -1 to use the default port (80
 * or 443).
 * @param uid the Atmos UID for authorization.
 * @param secret the shared secret key for the Atmos UID.
 * @return the AtmosClient pointer (same as 'self').
 */
AtmosClient*
AtmosClient_init(AtmosClient *self, const char *host, int port, const char *uid, const char *secret);

/**
 * Destroys and AtmosClient object.
 * @param self the AtmosClient object pointer.
 */
void
AtmosClient_destroy(AtmosClient *self);

/**
 * Gets the Atmos service information (version and feature flags).  This method
 * is also useful as a 'login' operation for checking your UID and shared
 * secret since it is fast and does not alter any data on Atmos nor require
 * and existing objects.
 * @param self the AtmosClient object pointer.
 * @param response pointer to the AtmosServiceInfoResponse object receiving
 * the response.
 */
void
AtmosClient_get_service_information(AtmosClient *self, AtmosServiceInfoResponse *response);

/**
 * Creates a new object in Atmos.
 * @param self the AtmosClient object pointer.
 * @param request the AtmosCreateObjectRequest specifying the attributes of the
 * new object in Atmos.
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 */
void
AtmosClient_create_object(AtmosClient *self, AtmosCreateObjectRequest *request,
        AtmosCreateObjectResponse *response);

/**
 * Creates a new object in Atmos.
 * @param self the AtmosClient object pointer.
 * @param data byte array containing the data for the new object's content.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 */
void
AtmosClient_create_object_simple(AtmosClient *self, const char *data,
        size_t data_size,
        const char *content_type, AtmosCreateObjectResponse *response);

/**
 * Creates a new object in Atmos using the namespace. Note that you will get an
 * error if an object already exists in the given location.  You can create a
 * directory by specifying NULL for data and append a trailing slash to the.
 * @param self the AtmosClient object pointer.
 * @param path the Atmos namespace path for the new file, e.g. /dir/image.jpeg
 * or directory, e.g. /dir1/dir2/
 * @param data byte array containing the data for the new object's content.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 */
void
AtmosClient_create_object_simple_ns(AtmosClient *self, const char *path,
        const char *data, size_t data_size, const char *content_type,
        AtmosCreateObjectResponse *response);

/**
 * Creates a new object in Atmos.
 * @param self the AtmosClient object pointer.
 * @param f file pointer to read the new object's content from.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 */
void
AtmosClient_create_object_file(AtmosClient *self, FILE *f,
        off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response);

/**
 * Creates a new object in Atmos using the namespace. Note that you will get an
 * error if an object already exists in the given location.  You can create a
 * directory by specifying NULL for data and append a trailing slash to the.
 * @param self the AtmosClient object pointer.
 * @param path the Atmos namespace path for the new file, e.g. /dir/image.jpeg
 * or directory, e.g. /dir1/dir2/
 * @param f file pointer to read the new object's content from.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 */
void
AtmosClient_create_object_file_ns(AtmosClient *self, const char *path, FILE *f,
        off_t content_length, const char *content_type,
        AtmosCreateObjectResponse *response);

/**
 * Deletes an object from Atmos by ID.
 * @param self the AtmosClient object pointer.
 * @param object_id the Atmos ObjectID to delete.
 * @param response the RestResponse object to receive the response information.
 * The http_code will be 204 on success.
 */
void
AtmosClient_delete_object(AtmosClient *self, const char *object_id,
        RestResponse *response);

/**
 * Deletes an object from the Atmos namespace.  Note that you cannot delete a
 * directory object if it is not empty.
 * @param self the AtmosClient object pointer.
 * @param path the Atmos namespace path to delete, e.g. /dir/image.jpeg
 * or directory, e.g. /dir1/dir2/
 * @param response the RestResponse object to receive the response information.
 * The http_code will be 204 on success.
 */
void
AtmosClient_delete_object_ns(AtmosClient *self, const char *path,
        RestResponse *response);

/**
 * Reads an object from Atmos. By default, the object's contents will be written
 * into memory.  To write the contents into a file, execute
 * RestResponse_use_file() on the response object.
 * @param self the AtmosClient object pointer.
 * @param request the AtmosReadObjectRequest specifying the read object
 * parameters.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * contents.
 */
void
AtmosClient_read_object(AtmosClient *self, AtmosReadObjectRequest *request,
        AtmosReadObjectResponse *response);

/**
 * Reads an object from Atmos. By default, the object's contents will be written
 * into memory.  To write the contents into a file, execute
 * RestResponse_use_file() on the response object.
 * @param self the AtmosClient object pointer.
 * @param object_id the Atmos object ID to read.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * contents.
 */
void
AtmosClient_read_object_simple(AtmosClient *self, const char *object_id,
        AtmosReadObjectResponse *response);

/**
 * Reads an object from Atmos. By default, the object's contents will be written
 * into memory.  To write the contents into a file, execute
 * RestResponse_use_file() on the response object.
 * @param self the AtmosClient object pointer.
 * @param path the Atmos namespace path to read. Note that you will generally
 * use AtmosClient_read_directory_simple() to read directory objects instead
 * of this method. Using this method on the directory will return the raw XML
 * of the directory contents.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * contents.
 */
void
AtmosClient_read_object_simple_ns(AtmosClient *self, const char *path,
        AtmosReadObjectResponse *response);

/**
 * }@
 */

#endif /* ATMOS_CLIENT_H_ */
