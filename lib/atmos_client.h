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
 * @param endpoint hostname or IP address of the Atmos access point with
 * protocol, e.g. http://api.atmosonline.com.
 * @param port the TCP port to connect to.  Use -1 to use the default port (80
 * or 443).
 * @param uid the Atmos UID for authorization.
 * @param secret the shared secret key for the Atmos UID.
 * @return the AtmosClient pointer (same as 'self').
 */
AtmosClient*
AtmosClient_init(AtmosClient *self, const char *endpoint, int port, const char *uid, const char *secret);

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
 * Creates a new object in Atmos using the namespace. Note that you will get an
 * error if an object already exists in the given location.  You can create a
 * directory by specifying NULL for data and append a trailing slash to the.
 * @param self the AtmosClient object pointer.
 * @param pool the name of the pool to contain the new object.
 * @param key the object key in the pool.
 * @param data byte array containing the data for the new object's content.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 * @since Atmos 2.1.0
 */
void
AtmosClient_create_object_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, const char *data, size_t data_size,
        const char *content_type, AtmosCreateObjectResponse *response);


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
 * Creates a new object in Atmos using a keypool. Note that you will get an
 * error if an object already exists in the given location.
 * @param self the AtmosClient object pointer.
 * @param pool the name of the pool to contain the new object.
 * @param key the object key in the pool.
 * @param f file pointer to read the new object's content from.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME content type of the object, e.g. "image/jpeg".
 * @param response the AtmosCreateObjectResponse object to receive the response
 * information.
 * @since Atmos 2.1.0
 */
void
AtmosClient_create_object_file_keypool(AtmosClient *self, const char *pool,
        const char *key, FILE *f, off_t content_length,
        const char *content_type, AtmosCreateObjectResponse *response);


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
 * Deletes an object from an Atmos keypool.
 * @param self the AtmosClient object pointer.
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the RestResponse object to receive the response information.
 * The http_code will be 204 on success.
 * @since Atmos 2.1.0
 */
void
AtmosClient_delete_object_keypool(AtmosClient *self, const char *pool,
        const char *key, RestResponse *response);

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
 * Reads an object from Atmos. By default, the object's contents will be written
 * into memory.  To write the contents into a file, execute
 * RestResponse_use_file() on the response object.
 * @param self the AtmosClient object pointer.
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * contents.
 * @since Atmos 2.1.0
 */
void
AtmosClient_read_object_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosReadObjectResponse *response);


/**
 * Gets information about an object by issuing a HEAD call.  This fetches
 * the object's system metadata, user metadata, and ACL.  Note that this
 * is slightly more expensive than get_system_metadata, so if all you want
 * is size and other system attributes, you should use that.  Likewise, if
 * all you want is user metadata, you should call get_user_meta.
 * @param self the AtmosClient object pointer.
 * @param object_id the Atmos object ID to read.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * information.
 */
void
AtmosClient_head_object(AtmosClient *self, const char *object_id,
        AtmosReadObjectResponse *response);

/**
 * Gets information about an object by issuing a HEAD call.  This fetches
 * the object's system metadata, user metadata, and ACL.  Note that this
 * is slightly more expensive than get_system_metadata, so if all you want
 * is size and other system attributes, you should use that.  Likewise, if
 * all you want is user metadata, you should call get_user_meta.
 * @param self the AtmosClient object pointer.
 * @param path the Atmos namespace path to read. Note that you will generally
 * use AtmosClient_read_directory_simple() to read directory objects instead
 * of this method. Using this method on the directory will return the raw XML
 * of the directory contents.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * information.
 */
void
AtmosClient_head_object_ns(AtmosClient *self, const char *path,
        AtmosReadObjectResponse *response);

/**
 * Gets information about an object by issuing a HEAD call.  This fetches
 * the object's system metadata, user metadata, and ACL.  Note that this
 * is slightly more expensive than get_system_metadata, so if all you want
 * is size and other system attributes, you should use that.  Likewise, if
 * all you want is user metadata, you should call get_user_meta.
 * @param self the AtmosClient object pointer.
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the AtmosReadObjectResponse object to receive the object's
 * information.
 * @since Atmos 2.1.0
 */
void
AtmosClient_head_object_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosReadObjectResponse *response);


/**
 * Updates an object in Atmos.  This is the advanced version that can update
 * the whole object, a range within the object, append to an object, or include
 * metadata and/or ACL updates with the data.
 * @param self the AtmosClient object pointer
 * @param request the AtmosUpdateObjectRequest
 * @param response the RestResponse object to receive the response.
 */
void
AtmosClient_update_object(AtmosClient *self, AtmosUpdateObjectRequest *request,
        RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied byte array.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to update
 * @param data the byte array containing the new data for the object.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 */
void
AtmosClient_update_object_simple(AtmosClient *self, const char *object_id,
        const char *data, size_t data_size, const char *content_type,
        RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied byte array.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to update
 * @param data the byte array containing the new data for the object.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 */
void
AtmosClient_update_object_simple_ns(AtmosClient *self, const char *path,
        const char *data, size_t data_size, const char *content_type,
        RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied byte array.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param data the byte array containing the new data for the object.
 * @param data_size the number of bytes to read from the byte array.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 * @since Atmos 2.1.0
 */
void
AtmosClient_update_object_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, const char *data, size_t data_size,
        const char *content_type, RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied file.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to update
 * @param f the file pointer containing the new data for the object.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 */
void
AtmosClient_update_object_file(AtmosClient *self, const char *object_id,
        FILE *f, off_t content_length, const char *content_type,
        RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied file.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to update
 * @param f the file pointer containing the new data for the object.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 */
void
AtmosClient_update_object_file_ns(AtmosClient *self, const char *path,
        FILE *f, off_t content_length, const char *content_type,
        RestResponse *response);

/**
 * Updates an object in Atmos.  This version will replace the entire object's
 * contents with the supplied file.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param f the file pointer containing the new data for the object.
 * @param content_length the number of bytes to read from the file.
 * @param content_type the MIME type of the object (e.g. image/jpeg)
 * @param response the RestResponse object to receive the response.
 * @since Atmos 2.1.0
 */
void
AtmosClient_update_object_file_keypool(AtmosClient *self, const char *pool,
        const char *key, FILE *f, off_t content_length,
        const char *content_type, RestResponse *response);

/**
 * Sets user metadata on an object.  Note that existing metadata elements on
 * the object that are not referenced when calling this method will remain
 * unmodified.  To remove metadata use AtmosClient_delete_user_meta().
 * @param self the AtmosClient object pointer
 * @param request the AtmosSetUserMetaRequest object containing the metadata
 * to set on the object.
 * @param response the AtmosResponse object to receive the response.
 */
void
AtmosClient_set_user_meta(AtmosClient *self, AtmosSetUserMetaRequest *request,
        AtmosResponse *response);

/**
 * Sets the ACL on an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to update
 * @param acl array of ACL entries for the object
 * @param acl_count number of items in 'acl'
 * @param response the AtmosResponse object to receive the response.
 */
void
AtmosClient_set_acl(AtmosClient *self, const char *object_id,
        AtmosAclEntry *acl, int acl_count, AtmosResponse *response);

/**
 * Sets the ACL on an object.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to update
 * @param acl array of ACL entries for the object
 * @param acl_count number of items in 'acl'
 * @param response the AtmosResponse object to receive the response.
 */
void
AtmosClient_set_acl_ns(AtmosClient *self, const char *path,
        AtmosAclEntry *acl, int acl_count, AtmosResponse *response);

/**
 * Sets the ACL on an object.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param acl array of ACL entries for the object
 * @param acl_count number of items in 'acl'
 * @param response the AtmosResponse object to receive the response.
 * @since 2.1.0
 */
void
AtmosClient_set_acl_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosAclEntry *acl, int acl_count,
        AtmosResponse *response);

/**
 * Removes metadata entries from an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to update
 * @param meta_names array of strings containing the metadata element names
 * to remove.
 * @param meta_name_count number of elements in 'meta_names'
 * @param response RestResponse object to receive the response.  Note that this
 * is a simple RestResponse because Atmos error messages are unavailable from
 * DELETE operations.
 */
void
AtmosClient_delete_user_meta(AtmosClient *self, const char *object_id,
        const char const **meta_names, int meta_name_count,
        RestResponse *response);

/**
 * Removes metadata entries from an object.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to update
 * @param meta_names array of strings containing the metadata element names
 * to remove.
 * @param meta_name_count number of elements in 'meta_names'
 * @param response RestResponse object to receive the response.  Note that this
 * is a simple RestResponse because Atmos error messages are unavailable from
 * DELETE operations.
 */
void
AtmosClient_delete_user_meta_ns(AtmosClient *self, const char *path,
        const char const **meta_names, int meta_name_count,
        RestResponse *response);

/**
 * Removes metadata entries from an object.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param meta_names array of strings containing the metadata element names
 * to remove.
 * @param meta_name_count number of elements in 'meta_names'
 * @param response RestResponse object to receive the response.  Note that this
 * is a simple RestResponse because Atmos error messages are unavailable from
 * DELETE operations.
 * @since 2.1.0
 */
void
AtmosClient_delete_user_meta_keypool(AtmosClient *self, const char *pool,
        const char *key, const char const **meta_names, int meta_name_count,
        RestResponse *response);

/**
 * Gets the ACL for an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to get the ACL for.
 * @param response AtmosGetAclResponse object to receive the response.
 */
void
AtmosClient_get_acl(AtmosClient *self, const char *object_id,
        AtmosGetAclResponse *response);

/**
 * Gets the ACL for an object.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to get the ACL for.
 * @param response AtmosGetAclResponse object to receive the response.
 */
void
AtmosClient_get_acl_ns(AtmosClient *self, const char *path,
        AtmosGetAclResponse *response);

/**
 * Gets the ACL for an object.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response AtmosGetAclResponse object to receive the response.
 * @since 2.1.0
 */
void
AtmosClient_get_acl_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetAclResponse *response);

/**
 * Gets the user metadata for an object.  This is the 'advanced' version that
 * allows you to specify all options like the list of metadata tags to
 * receive.  You can also use this version to request special object
 * properties like user.maui.expirationEnd (also avaliable through
 * AtmosClient_get_object_info())
 * @param self the AtmosClient object pointer
 * @param request the AtmosGetUserMetaRequest
 * @param response the AtmosGetUserMetaResponse to receive the response.
 */
void
AtmosClient_get_user_meta(AtmosClient *self, AtmosGetUserMetaRequest *request,
        AtmosGetUserMetaResponse *response);

/**
 * Gets the user metadata for an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to get the user metadata for.
 * @param response the AtmosGetUserMetaResponse to receive the response.
 */
void
AtmosClient_get_user_meta_simple(AtmosClient *self, const char *object_id,
        AtmosGetUserMetaResponse *response);

/**
 * Gets the user metadata for an object.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to get the user metadata for.
 * @param response the AtmosGetUserMetaResponse to receive the response.
 */
void
AtmosClient_get_user_meta_simple_ns(AtmosClient *self, const char *path,
        AtmosGetUserMetaResponse *response);

/**
 * Gets the user metadata for an object.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the AtmosGetUserMetaResponse to receive the response.
 * @since 2.1.0
 */
void
AtmosClient_get_user_meta_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetUserMetaResponse *response);

/**
 * Gets system metadata for an object.  This is the 'advanced' verison that
 * allows the setting of the list of metadata entries to receive.
 * @param self the AtmosClient object pointer
 * @param request the AtmosGetSystemMetaRequest object containing the request
 * parameters.
 * @param response the AtmosGetSystemMetaResponse object to receive the
 * response.
 */
void
AtmosClient_get_system_meta(AtmosClient *self,
        AtmosGetSystemMetaRequest *request,
        AtmosGetSystemMetaResponse *response);

/**
 * Gets system metadata for an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to get the system metadata for.
 * @param response the AtmosGetSystemMetaResponse object to receive the
 * response.
 */
void
AtmosClient_get_system_meta_simple(AtmosClient *self, const char *object_id,
        AtmosGetSystemMetaResponse *response);

/**
 * Gets system metadata for an object.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to get the system metadata for.
 * @param response the AtmosGetSystemMetaResponse object to receive the
 * response.
 */
void
AtmosClient_get_system_meta_simple_ns(AtmosClient *self, const char *path,
        AtmosGetSystemMetaResponse *response);

/**
 * Gets system metadata for an object.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the AtmosGetSystemMetaResponse object to receive the
 * response.
 * @since 2.1.0
 */
void
AtmosClient_get_system_meta_simple_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetSystemMetaResponse *response);

/**
 * Gets storage information about an object.
 * @param self the AtmosClient object pointer
 * @param object_id the object ID to get information about.
 * @param response the AtmosGetObjectInfoResponse object to receive information
 * about the object.
 */
void
AtmosClient_get_object_info(AtmosClient *self, const char *object_id,
        AtmosGetObjectInfoResponse *response);

/**
 * Gets storage information about an object.  Note that this will return an
 * error if called on a directory object since directories do not have storage.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to get information about.
 * @param response the AtmosGetObjectInfoResponse object to receive information
 * about the object.
 */
void
AtmosClient_get_object_info_ns(AtmosClient *self, const char *path,
        AtmosGetObjectInfoResponse *response);

/**
 * Gets storage information about an object.  Note that this will return an
 * error if called on a directory object since directories do not have storage.
 * @param self the AtmosClient object pointer
 * @param pool the name of the pool that contains the object.
 * @param key the object key in the pool.
 * @param response the AtmosGetObjectInfoResponse object to receive information
 * about the object.
 * @since 2.1.0
 */
void
AtmosClient_get_object_info_keypool(AtmosClient *self, const char *pool,
        const char *key, AtmosGetObjectInfoResponse *response);


/**
 * Lists the contents of a directory.  Note that you must check the value of
 * response->token after listing the directory.  If the token is not NULL,
 * additional results are available.  Set the token on the request and reissue
 * the request to receive additional results until the token is NULL.
 * @param self the AtmosClient object pointer
 * @param request the AtmosListDirectoryRequest object containing the list
 * directory parameters.
 * @param response the AtmosListDirectoryResponse object to receive the
 * directory listing.
 */
void
AtmosClient_list_directory(AtmosClient *self,
        AtmosListDirectoryRequest *request,
        AtmosListDirectoryResponse *response);

/**
 * Renames (moves) an object in the Atmos namespace.  By default, if the
 * destination exists, the object will not be moved.  Set force to nonzero to
 * overwrite any existing objects.  Note that Atmos may cache objects for up to
 * five seconds.  If you read an object on one node, overwrite it on a second
 * node, then reread the object on the first node within five seconds of the
 * first you may get an object not found error until the cache expires.
 * Therefore, it is recommended to perform a short wait after overwrites before
 * executing further operations on the same object.
 * Due to limitations on character handling in HTTP headers, it's very highly
 * recommended to enable UTF-8 support when calling this method.
 * @param self the AtmosClient object pointer
 * @param source the namespace path to the object to rename.
 * @param destination the new namespace path for the object.
 * @param force if nonzero, overwrite the destination if it exists.
 * @param response the AtmosResponse object to receive the response.
 */
void
AtmosClient_rename_object(AtmosClient *self, const char *source,
        const char *destination, int force, AtmosResponse *response);

/**
 * Gets the list of listable tags for the subtenant.
 * @param self the AtmosClient object pointer
 * @param request the AtmosGetListableTagsRequest containing the parameters
 * for the request.
 * @param response the AtmosGetListableTagsResponse to receive the list of
 * tags.
 */
void
AtmosClient_get_listable_tags(AtmosClient *self,
        AtmosGetListableTagsRequest *request,
        AtmosGetListableTagsResponse *response);

/**
 * Lists the objects indexed by a listable tag.  Note that you must check the
 * value of response->token after listing the tag.  If the token is not NULL,
 * additional results are available.  Set the token on the request and reissue
 * the request to receive additional results until the token is NULL.
 * @param self the AtmosClient object pointer
 * @param request the AtmosListDirectoryRequest object containing the list
 * object parameters.
 * @param response the AtmosListDirectoryResponse object to receive the
 * tag contents.
 */
void
AtmosClient_list_objects(AtmosClient *self, AtmosListObjectsRequest *request,
        AtmosListObjectsResponse *response);

/**
 * Creates a new version of the object.
 * @param self the AtmosClient object pointer
 * @param object_id the ID of the object to create a new version of.
 * @param response the AtmosCreateVersionResponse to receive the response.
 */
void
AtmosClient_create_version(AtmosClient *self, const char *object_id,
        AtmosCreateVersionResponse *response);

/**
 * Lists the versions of an object.  Note that you must check the
 * value of response->token after listing the versions.  If the token is not
 * NULL, additional results are available.  Set the token on the request and
 * reissue the request to receive additional results until the token is NULL.
 * @param self the AtmosClient object pointer
 * @param request the AtmosListVersionsRequest containing the parameters for the
 * operation.
 * @param response the AtmosListVersionsResponse to receive the response.
 */
void
AtmosClient_list_versions(AtmosClient *self, AtmosListVersionsRequest *request,
        AtmosListVersionsResponse *response);

/**
 * Deletes a version of an object.
 * @param self the AtmosClient object pointer
 * @param version_id the ID of the version to delete.
 * @param response the RestResponse to receive the response.  Upon success,
 * http_code will be 204 (no content).
 */
void
AtmosClient_delete_version(AtmosClient *self, const char *version_id,
        RestResponse *response);

/**
 * Replaces an object's contents with those of a previous version.
 * @param self the AtmosClient object pointer
 * @param object_id the ID of the Atmos object to replace
 * @param version_id the previous version to replace the object with.
 * @param response the RestResponse object to receive the response.  Upon
 * success, http_code will be 200 (OK).
 */
void
AtmosClient_restore_version(AtmosClient *self, const char *object_id,
        const char *version_id, RestResponse *response);

/**
 * Creates an access token allowing anonymous upload or download of objects
 * in Atmos.
 * @param self the AtmosClient object pointer
 * @param request the AtmosCreateAccessTokenRequest object specifying the
 * parameters for the new access token.
 * @param response the AtmosCreateAccessTokenResponse object to receive the
 * response.
 */
void
AtmosClient_create_access_token(AtmosClient *self,
        AtmosCreateAccessTokenRequest *request,
        AtmosCreateAccessTokenResponse *response);

/**
 * Deletes an existing access token.
 * @param self the AtmosClient object pointer
 * @param token_id the ID of the token to delete.
 * @param response the RestResponse object to receive the response.  On success,
 * http_code will be 204 (no content).
 */
void
AtmosClient_delete_access_token(AtmosClient *self, const char *token_id,
        RestResponse *response);

/**
 * Lists the set of access tokens for the current UID.  Note that you must
 * check the value of response->token after listing the versions.  If the token
 * is not NULL, additional results are available.  Set the token on the
 * request and reissue the request to receive additional results until
 * the token is NULL.
 * @param self the AtmosClient object pointer
 * @param request the AtmosListAccessTokensRequest containing the parameters for
 * the list tokens operation.
 * @param response the AtmosListAccessTokensResponse object to receive the
 * response.
 */
void
AtmosClient_list_access_tokens(AtmosClient *self,
        AtmosListAccessTokensRequest *request,
        AtmosListAccessTokensResponse *response);

/**
 * Gets information about an access token.
 * @param self the AtmosClient object pointer
 * @param token_id the ID of the token to get information about.
 * @param response the AtmosGetAccesTokenInfoResponse object to receive the
 * response.
 */
void
AtmosClient_get_access_token_info(AtmosClient *self, const char *token_id,
        AtmosGetAccessTokenInfoResponse *response);

/**
 * Generates a "shareable URL" for the given object.  Shareable URLs are
 * presigned URLs that can be used to fetch an object from Atmos without
 * authentication.
 * @param self the AtmosClient object pointer
 * @param object_id the ID of the object to share.
 * @param expires expiration of the URL in standard UNIX time (seconds since
 * epoch, GMT).
 * @param disposition used to set the value of the Content-Disposition header
 * on the response.  This can be used to supply a filename to the browser
 * and/or force the browser to download the file instead of displaying it
 * inline.  Example: "attachment;filename=foo.txt".  See RFCs 6266 and 2183
 * for more information, especially when using UTF-8 filenames.  Also be sure
 * to test with browsers; support varies widely.
 * @return the shareable URL.  The caller is responsible for freeing this
 * pointer.
 */
char *
AtmosClient_get_shareable_url(AtmosClient *self, const char *object_id,
        time_t expires, const char *disposition);

/**
 * Generates a "shareable URL" for the given object.  Shareable URLs are
 * presigned URLs that can be used to fetch an object from Atmos without
 * authentication.
 * @param self the AtmosClient object pointer
 * @param path the namespace path to the object to share.
 * @param expires expiration of the URL in standard UNIX time (seconds since
 * epoch, GMT).
 * @param disposition used to set the value of the Content-Disposition header
 * on the response.  This can be used to supply a filename to the browser
 * and/or force the browser to download the file instead of displaying it
 * inline.  Example: "attachment;filename=foo.txt".  See RFCs 6266 and 2183
 * for more information, especially when using UTF-8 filenames.  Also be sure
 * to test with browsers; support varies widely.
 * @return the shareable URL.  The caller is responsible for freeing this
 * pointer.
 */
char *
AtmosClient_get_shareable_url_ns(AtmosClient *self, const char *path,
        time_t expires, const char *disposition);

/**
 * @}
 */

#endif /* ATMOS_CLIENT_H_ */
