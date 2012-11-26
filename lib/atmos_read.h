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
#ifndef ATMOS_READ_H_
#define ATMOS_READ_H_

/**
 * @file atmos_read.h
 * @brief This file contains the classes used to read objects from Atmos.
 * @defgroup AtmosReadObjectRequest AtmosReadObjectRequest
 * @brief This module contains the AtmosReadObjectRequest class that is used
 * to read objects from Atmos.
 * @{
 */

#include "atmos.h"

/**
 * Contains the parameters used to read an object from Atmos.
 */
typedef struct {
    /** Parent class */
    RestRequest parent;
    /** Namespace path of object to read.  Should be empty if object_id is used. */
    char path[ATMOS_PATH_MAX];
    /** Object ID of object to be read.  Should be empty if path is used. */
    char object_id[ATMOS_OID_LENGTH];
    /** Start of object range to read. Set to -1 for no start range */
    int64_t range_start;
    /** End of object range to read.  Set to -1 for no end range */
    int64_t range_end;
} AtmosReadObjectRequest;

/**
 * Initializes a new AtmosReadObjectRequest.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param object_id the ID of the object to read.
 * @return the AtmosReadObjectRequest pointer (same as self)
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init(AtmosReadObjectRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosReadObjectRequest.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param path the namespace path of the object to read.
 * @return the AtmosReadObjectRequest pointer (same as self)
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init_ns(AtmosReadObjectRequest *self, const char *path);

/**
 * Destroys an AtmosReadObjectRequest.
 * @param self the AtmosReadObjectRequest to destroy.
 */
void
AtmosReadObjectRequest_destroy(AtmosReadObjectRequest *self);

/**
 * Sets the object content range to read, inclusive. Set to [-1,-1] to read the
 * entire object.  Set to [x,-1] to read from x to the end of the object.  Set
 * to [-1,x] to read the last x bytes of an object.
 * @param self the AtmosReadObjectRequest to configure.
 * @param range_start the start of the object range to read.  Set to -1 for no
 * start range.
 * @param range_end the end of the object range to read.  Set to -1 for no end
 * range.
 */
void
AtmosReadObjectRequest_set_range(AtmosReadObjectRequest *self,
        int64_t range_start, int64_t range_end);

/**
 * A convenience function that allows you to specify the extent using offset
 * and size.
 * @param self the AtmosReadObjectRequest to configure.
 * @param offset start of the object range to read.
 * @param size the number of bytes to read starting at the offset.
 */
void
AtmosReadObjectRequest_set_range_offset_size(AtmosReadObjectRequest *self,
        int64_t offset, int64_t size);

/**
 * @}
 * @defgroup AtmosReadObjectResponse AtmosReadObjectResponse
 * @brief This module contains the AtmosReadObjectResponse class that is used
 * to capture the read object response from Atmos.
 * @{
 */


/**
 * Receives data from the read object operation.
 */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /** Object metadata */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata elements in meta */
    int meta_count;
    /** Object listable metadata */
    AtmosMetadata listable_metadata[ATMOS_META_COUNT_MAX];
    /** Number of elements in listable_meta */
    int listable_meta_count;
    /** ACL entries for the object */
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    /** Number of ACL entries */
    int acl_count;
    /** System metadata for the object */
    AtmosSystemMetadata system_metadata;
} AtmosReadObjectResponse;

/**
 * Initializes a new AtmosReadObjectResponse
 * @param self pointer to the AtmosReadObjectResponse to initialize.
 * @return the AtmosReadObjectResponse (same as 'self')
 */
AtmosReadObjectResponse*
AtmosReadObjectResponse_init(AtmosReadObjectResponse *self);

/**
 * Destroys the AtmosReadObjectResponse.  This deallocates any data read from
 * the object into memory.
 * @param self the AtmosReadObjectResponse to destroy.
 */
void
AtmosReadObjectResponse_destroy(AtmosReadObjectResponse *self);

/**
 * Convenience method to read user metadata from the response.
 * @param self the AtmosReadObjectResponse to search
 * @param name the name of the metadata item
 * @param listable set to 1 to search listable metadata, 0 to search regular.
 * @return the value of the metadata, or NULL if the metadata was not found.
 */
const char *
AtmosReadObjectResponse_get_metadata_value(AtmosReadObjectResponse *self,
        const char *name, int listable);

/**
 * Convenience method to read the ACL permission granted to a principal.
 * @param self the AtmosReadObjectResponse to search
 * @param principal the principal name to look for
 * @param principal_type the type of principal (ATMOS_USER or ATMOS_GROUP)
 * @return the permissions granted to the principal.  If the principal is not
 * found in the ACL, the permission NONE is returned.
 */
enum atmos_acl_permission
AtmosReadObjectResponse_get_acl_permission(AtmosReadObjectResponse *self,
        const char *principal, enum atmos_acl_principal_type principal_type);


/**
 * @}
 * @defgroup AtmosGetAclResponse AtmosGetAclResponse
 * @brief This module contains the AtmosGetAclResponse class that is used
 * to capture the get ACL response from Atmos.
 * @{
 */
typedef struct {
    AtmosResponse parent;
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    /** Number of ACL entries. */
    int acl_count;
} AtmosGetAclResponse;

AtmosGetAclResponse*
AtmosGetAclResponse_init(AtmosGetAclResponse *self);

void
AtmosGetAclResponse_destroy(AtmosGetAclResponse *self);

/**
 * @}
 * @defgroup AtmosGetUserMetaRequest AtmosGetUserMetaRequest
 * @brief This module contains the AtmosGetUserMetaRequest class that is used
 * to get user metadata from Atmos.
 * @{
 */

typedef struct {
    RestRequest parent;
    char object_id[ATMOS_OID_LENGTH];
    char path[ATMOS_PATH_MAX];
    char tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int tag_count;
} AtmosGetUserMetaRequest;

AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init(AtmosGetUserMetaRequest *self,
        const char *object_id);

AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init_ns(AtmosGetUserMetaRequest *self, const char *path);

void
AtmosGetUserMetaRequest_destroy(AtmosGetUserMetaRequest *self);


/**
 * @}
 * @defgroup AtmosGetUserMetaResponse AtmosGetUserMetaResponse
 * @brief This module contains the AtmosGetUserMetaResponse class that is used
 * to capture the user metadata response from Atmos.
 * @{
 */
typedef struct {
    AtmosResponse parent;
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata elements in meta */
    int meta_count;
    /** Object listable metadata */
    AtmosMetadata listable_metadata[ATMOS_META_COUNT_MAX];
    /** Number of elements in listable_meta */
    int listable_meta_count;
} AtmosGetUserMetaResponse;

AtmosGetUserMetaResponse*
AtmosGetUserMetaResponse_init(AtmosGetUserMetaResponse *self);

void
AtmosGetUserMetaResponse_destroy(AtmosGetUserMetaResponse *self);

/**
 * @}
 * @defgroup AtmosGetSystemMetaRequest AtmosGetSystemMetaRequest
 * @brief This module contains the AtmosGetSystemMetaRequest class that is used
 * to request the system metadata from Atmos.
 * @{
 */

typedef struct {
    RestRequest parent;
    char object_id[ATMOS_OID_LENGTH];
    char path[ATMOS_PATH_MAX];
    char tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int tag_count;
} AtmosGetSystemMetaRequest;

AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init(AtmosGetSystemMetaRequest *self,
        const char *object_id);

AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init_ns(AtmosGetSystemMetaRequest *self,
        const char *path);

void
AtmosGetSystemMetaRequest_destroy(AtmosGetSystemMetaRequest self);

/**
 * @}
 * @defgroup AtmosGetSystemMetaResponse AtmosGetSystemMetaResponse
 * @brief This module contains the AtmosGetSystemMetaResponse class that is used
 * to capture the system metadata response from Atmos.
 * @{
 */

typedef struct {
    AtmosResponse parent;
    AtmosSystemMetadata system_metadata;
} AtmosGetSystemMetaResponse;

AtmosGetSystemMetaResponse*
AtmosGetSystemMetaResponse_init(AtmosGetSystemMetaResponse *self);

void
AtmosGetSystemMetaResponse_destroy(AtmosGetSystemMetaResponse *self);

/**
 * @}
 * @defgroup AtmosGetObjectInfoResponse AtmosGetObjectInfoResponse
 * @brief This module contains the AtmosGetObjectInfoResponse class that is used
 * to capture the object info response from Atmos.
 * @{
 */

typedef struct {
    AtmosResponse parent;
    // TODO: finish
} AtmosGetObjectInfoResponse;

AtmosGetObjectInfoResponse*
AtmosGetObjectInfoResponse_init(AtmosGetObjectInfoResponse *self);

void
AtmosGetObjectInfoResponse_destroy(AtmosGetObjectInfoResponse *self);

/**
 * @}
 * @defgroup AtmosListDirectoryRequest AtmosListDirectoryRequest
 * @brief This module contains the AtmosListDirectoryRequest class that is used
 * to request a directory listing from Atmos.
 * @{
 */
typedef struct {
    RestRequest parent;
    char path[ATMOS_PATH_MAX];
    char token[ATMOS_OID_LENGTH];
    int limit;
    int include_meta;
    char user_tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int user_tag_count;
    char system_tags[ATMOS_SYSTEM_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int system_tag_count;
} AtmosListDirectoryRequest;

AtmosListDirectoryRequest*
AtmosListDirectoryRequest_init(AtmosListDirectoryRequest *self,
        const char *path);

void
AtmosListDirectoryRequest_destroy(AtmosListDirectoryRequest *self);

/**
 * @}
 * @defgroup AtmosListDirectoryResponse AtmosListDirectoryResponse
 * @brief This module contains the AtmosListDirectoryResponse class that is used
 * to capture a directory listing response from Atmos.
 * @{
 */
typedef struct {
    AtmosResponse parent;
    const char *token;

    // TODO: finish
} AtmosListDirectoryResponse;

AtmosListDirectoryResponse*
AtmosListDirectoryResponse_init(AtmosListDirectoryResponse *self);

void
AtmosListDirectoryResponse_destroy(AtmosListDirectoryResponse *self);

/**
 * @}
 * @defgroup AtmosGetListableTagsRequest AtmosGetListableTagsRequest
 * @brief This module contains the AtmosGetListableTagsRequest class that is
 * used to request the set of listable tags from Atmos.
 * @{
 */
typedef struct {
    RestRequest parent;
    char parent_tag[ATMOS_PATH_MAX];
    char token[ATMOS_OID_LENGTH];
    int limit;
} AtmosGetListableTagsRequest;

AtmosGetListableTagsRequest*
AtmosGetListableTagsRequest_init(AtmosGetListableTagsRequest *self,
        const char *parent_tag);

void
AtmosGetListableTagsRequest_destroy(AtmosGetListableTagsRequest *self);

/**
 * @}
 * @defgroup AtmosGetListableTagsResponse AtmosGetListableTagsResponse
 * @brief This module contains the AtmosGetListableTagsResponse class that is
 * used to capture a listable tag listing response from Atmos.
 * @{
 */

typedef struct {
    AtmosResponse parent;
    const char *token;
    char **tags;
    int tag_count;
} AtmosGetListableTagsResponse;

AtmosGetListableTagsResponse*
AtmosGetListableTagsResponse_init(AtmosGetListableTagsResponse *self);

void
AtmosGetListableTagsResponse_destroy(AtmosGetListableTagsResponse *self);

/**
 * @}
 * @defgroup AtmosListObjectsRequest AtmosListObjectsRequest
 * @brief This module contains the AtmosListObjectsRequest class that is
 * used to list objects indexed by a tag in Atmos.
 * @{
 */
typedef struct {
    RestRequest parent;
    char tag[ATMOS_PATH_MAX];
    char token[ATMOS_OID_LENGTH];
    int limit;
    int include_meta;
    char user_tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int user_tag_count;
    char system_tags[ATMOS_SYSTEM_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    int system_tag_count;
} AtmosListObjectsRequest;

AtmosListObjectsRequest*
AtmosListObjectsRequest_init(AtmosListObjectsRequest *self, const char *tag);

void
AtmosListObjectsRequest_destroy(AtmosListObjectsRequest *self);

/**
 * @}
 * @defgroup AtmosListObjectsResponse AtmosListObjectsResponse
 * @brief This module contains the AtmosListObjectsResponse class that is
 * used to capture the list of objects in a tag from Atmos.
 * @{
 */
typedef struct {
    AtmosResponse parent;
    // TODO: finish
} AtmosListObjectsResponse;

AtmosListObjectsResponse*
AtmosListObjectsResponse_init(AtmosListObjectsResponse *self);

void
AtmosListObjectsResponse_destroy(AtmosListObjectsResponse *self);

/**
 * @}
 */


#endif /* ATMOS_READ_H_ */
