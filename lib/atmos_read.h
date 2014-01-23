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
    /**
     * For keypool requests, this is the pool used to create the object.  The
     * key will be stored in 'path'.
     */
    char pool[ATMOS_PATH_MAX];
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
 * Initializes a new AtmosReadObjectRequest.  This version will read an object
 * from an Atmos 2.1.0+ keypool.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosReadObjectRequest pointer (same as self)
 * @since Atmos 2.1.0
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init_keypool(AtmosReadObjectRequest *self,
        const char *pool, const char *key);

/**
 * Initializes a new AtmosReadObjectRequest in HEAD mode.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param object_id the ID of the object to read.
 * @return the AtmosReadObjectRequest pointer (same as self)
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init_head(AtmosReadObjectRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosReadObjectRequest in HEAD mode.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param path the namespace path of the object to read.
 * @return the AtmosReadObjectRequest pointer (same as self)
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init_ns_head(AtmosReadObjectRequest *self, const char *path);

/**
 * Initializes a new AtmosReadObjectRequest in HEAD mode.  This version will read an object
 * from an Atmos 2.1.0+ keypool.
 * @param self pointer to the AtmosReadObjectRequest to initialize
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosReadObjectRequest pointer (same as self)
 * @since Atmos 2.1.0
 */
AtmosReadObjectRequest*
AtmosReadObjectRequest_init_keypool_head(AtmosReadObjectRequest *self,
        const char *pool, const char *key);


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

/**
 * Contains the response from a get acl operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /** ACL entries for the object */
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    /** Number of ACL entries. */
    int acl_count;
} AtmosGetAclResponse;

/**
 * Initializes a new AtmosGetAclResponse object.
 * @param self the AtmosGetAclResponse object to initialize
 * @return the AtmosGetAclResponse object (same as 'self')
 */
AtmosGetAclResponse*
AtmosGetAclResponse_init(AtmosGetAclResponse *self);

/**
 * Destroys an AtmosGetAclResponse object.
 * @param self the AtmosGetAclResponse object to destory.
 */
void
AtmosGetAclResponse_destroy(AtmosGetAclResponse *self);

/**
 * Convenience method to read the ACL permission granted to a principal.
 * @param self the AtmosGetAclResponse to search
 * @param principal the principal name to look for
 * @param principal_type the type of principal (ATMOS_USER or ATMOS_GROUP)
 * @return the permissions granted to the principal.  If the principal is not
 * found in the ACL, the permission NONE is returned.
 */
enum atmos_acl_permission
AtmosGetAclResponse_get_acl_permission(AtmosGetAclResponse *self,
        const char *principal, enum atmos_acl_principal_type principal_type);


/**
 * @}
 * @defgroup AtmosGetUserMetaRequest AtmosGetUserMetaRequest
 * @brief This module contains the AtmosGetUserMetaRequest class that is used
 * to get user metadata from Atmos.
 * @{
 */

/** Contains the parameters for a get user metadata request */
typedef struct {
    /** Parent object */
    RestRequest parent;
    /** Object ID requested.  Should be empty if path is not */
    char object_id[ATMOS_OID_LENGTH];
    /** Namespace path requested.  Should be empty if object_id is not */
    char path[ATMOS_PATH_MAX];
    /** Atmos 2.1.0 pool containing the key.  Only used for keypool requests */
    char pool[ATMOS_PATH_MAX];
    /** Names of the user metadata elements (tags) to load */
    char tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of tags to load.  If zero, all tags will be fetched */
    int tag_count;
} AtmosGetUserMetaRequest;

/**
 * Initializes a new AtmosGetUserMetaRequest object.
 * @param self the AtmosGetUserMetaRequest object to initialize.
 * @param object_id the Atmos object ID to load user metadata for.
 * @return the AtmosGetUserMetaRequest object (same as 'self')
 */
AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init(AtmosGetUserMetaRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosGetUserMetaRequest object.
 * @param self the AtmosGetUserMetaRequest object to initialize.
 * @param path the Atmos namespace path to the object to load user metadata for.
 * @return the AtmosGetUserMetaRequest object (same as 'self')
 */
AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init_ns(AtmosGetUserMetaRequest *self, const char *path);

/**
 * Initializes a new AtmosGetUserMetaRequest object.  This version gets the
 * metadata for an object in an Atmos 2.1.0+ keypool.
 * @param self the AtmosGetUserMetaRequest object to initialize.
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosGetUserMetaRequest object (same as 'self')
 * @since Atmos 2.1.0
 */
AtmosGetUserMetaRequest*
AtmosGetUserMetaRequest_init_keypool(AtmosGetUserMetaRequest *self,
        const char *pool, const char *key);

/**
 * Destroys a AtmosGetUserMetaRequest object.
 * @param self the AtmosGetUserMetaRequest object to destroy.
 */
void
AtmosGetUserMetaRequest_destroy(AtmosGetUserMetaRequest *self);

/**
 * Adds a user metadata name (tag) to the list of tags to fetch with the
 * request.  Note that by default, all metadata entries will be fetched.  By
 * setting this, only the specified entries will be fetched.
 * @param self the AtmosGetUserMetaRequest to modify
 * @param tag the user metadata name add to the tag list.
 */
void
AtmosGetUserMetaRequest_add_tag(AtmosGetUserMetaRequest *self, const char *tag);


/**
 * @}
 * @defgroup AtmosGetUserMetaResponse AtmosGetUserMetaResponse
 * @brief This module contains the AtmosGetUserMetaResponse class that is used
 * to capture the user metadata response from Atmos.
 * @{
 */

/**
 * Contains the response from a get user metadata operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /** Regular metadata items returned in the response */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata elements in meta */
    int meta_count;
    /** Listable metadata items returned in the response */
    AtmosMetadata listable_metadata[ATMOS_META_COUNT_MAX];
    /** Number of elements in listable_meta */
    int listable_meta_count;
} AtmosGetUserMetaResponse;

/**
 * Initializes a new AtmosGetUserMetaResponse object.
 * @param self the AtmosGetUserMetaResponse object to initialize
 * @return the AtmosGetUserMetaResponse object (same as 'self')
 */
AtmosGetUserMetaResponse*
AtmosGetUserMetaResponse_init(AtmosGetUserMetaResponse *self);

/**
 * Destroys a AtmosGetUserMetaResponse object.
 * @param self the AtmosGetUserMetaResponse object to destroy.
 */
void
AtmosGetUserMetaResponse_destroy(AtmosGetUserMetaResponse *self);

/**
 * Convenience method to read user metadata from the response.
 * @param self the AtmosGetUserMetaResponse to search
 * @param name the name of the metadata item
 * @param listable set to 1 to search listable metadata, 0 to search regular.
 * @return the value of the metadata, or NULL if the metadata was not found.
 */
const char *
AtmosGetUserMetaResponse_get_metadata_value(AtmosGetUserMetaResponse *self,
        const char *name, int listable);


/**
 * @}
 * @defgroup AtmosGetSystemMetaRequest AtmosGetSystemMetaRequest
 * @brief This module contains the AtmosGetSystemMetaRequest class that is used
 * to request the system metadata from Atmos.
 * @{
 */

/**
 * Contains the request parameters to get the system metadata for an object.
 */
typedef struct {
    /** Parent object */
    RestRequest parent;
    /** object_id requested. Should be empty if path is not */
    char object_id[ATMOS_OID_LENGTH];
    /** path requested. Should be empty if object_id is not */
    char path[ATMOS_PATH_MAX];
    /** Atmos 2.1.0 pool containing the key.  Only used for keypool requests */
    char pool[ATMOS_PATH_MAX];
    /** Array of tags requested.  See ATMOS_SYSTEM_META* macros */
    char tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of tags requested.  If zero, all tags will be fetched */
    int tag_count;
} AtmosGetSystemMetaRequest;

/**
 * Initializes a new AtmosGetSystemMetaRequest.
 * @param self the AtmosGetSystemMetaRequest object pointer to initialize.
 * @param object_id the Atmos object ID to request system metadata for.
 * @return the AtmosGetSystemMetaRequest object (same as 'self').
 */
AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init(AtmosGetSystemMetaRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosGetSystemMetaRequest.
 * @param self the AtmosGetSystemMetaRequest object pointer to initialize.
 * @param path the Atmos namespace path to the object to request system
 * metadata for.
 * @return the AtmosGetSystemMetaRequest object (same as 'self').
 */
AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init_ns(AtmosGetSystemMetaRequest *self,
        const char *path);

/**
 * Initializes a new AtmosGetSystemMetaRequest.  This version gets the system
 * metadata for an object in an Atmos 2.1.0 keypool.
 * @param self the AtmosGetSystemMetaRequest object pointer to initialize.
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosGetSystemMetaRequest object (same as 'self').
 * @since Atmos 2.1.0
 */
AtmosGetSystemMetaRequest*
AtmosGetSystemMetaRequest_init_keypool(AtmosGetSystemMetaRequest *self,
        const char *pool, const char *key);

/**
 * Destroys a AtmosGetSystemMetaRequest object.
 * @param self the AtmosGetSystemMetaRequest object to destroy.
 */
void
AtmosGetSystemMetaRequest_destroy(AtmosGetSystemMetaRequest *self);

/**
 * Adds a tag to the list of system metadata items to fetch.  Note if you call
 * this method, only the requested tags will be valid in the result.  If you
 * do not call this method, all metadata tags will be returned by default.
 * @param self the AtmosGetSystemMetaRequest object to modify.
 * @param tag the name of the system metadata field to request.  See the
 * ATMOS_SYSTEM_META* macros for valid values.
 */
void
AtmosGetSystemMetaRequest_add_tag(AtmosGetSystemMetaRequest *self,
        const char *tag);

/**
 * @}
 * @defgroup AtmosGetSystemMetaResponse AtmosGetSystemMetaResponse
 * @brief This module contains the AtmosGetSystemMetaResponse class that is used
 * to capture the system metadata response from Atmos.
 * @{
 */

/**
 * Contains the result from a get system metadata operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /** System metadata for the requested object */
    AtmosSystemMetadata system_metadata;
} AtmosGetSystemMetaResponse;

/**
 * Initializes a new AtmosGetSystemMetaRequest object.
 * @param self the AtmosGetSystemMetaRequest to initialize
 * @return the AtmosGetSystemMetaRequest object (same as 'self')
 */
AtmosGetSystemMetaResponse*
AtmosGetSystemMetaResponse_init(AtmosGetSystemMetaResponse *self);

/**
 * Destroys a AtmosGetSystemMetaRequest object.
 * @param self the AtmosGetSystemMetaRequest object to destroy.
 */
void
AtmosGetSystemMetaResponse_destroy(AtmosGetSystemMetaResponse *self);

/**
 * @}
 * @defgroup AtmosGetObjectInfoResponse AtmosGetObjectInfoResponse
 * @brief This module contains the AtmosGetObjectInfoResponse class that is used
 * to capture the object info response from Atmos.
 * @{
 */

/**
 * Contains information about a replica
 */
typedef struct {
    /** Internal identifier of the replica */
    int id;
    /** Replica type.  Current values are "sync" or "async" */
    char type[16];
    /** Nonzero if the replica is current (synchronized) */
    int current;
    /** Atmos "Location" of the RMG hosting the replica, e.g. Cambridge */
    char location[ATMOS_SIMPLE_HEADER_MAX];
    /**
     * The replica's storage type.  Current values are normal, stripe,
     * cloud, compression, ErasureCode, and dedup
     */
    char storage_type[ATMOS_SIMPLE_HEADER_MAX];
} AtmosReplicaInfo;

/**
 * Encapsulates the response from the get object info operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /** The object's ID */
    char object_id[ATMOS_OID_LENGTH];
    /**
     * The replica selection for read access.  Current values are geographic
     * or random.
     */
    char selection[ATMOS_SIMPLE_HEADER_MAX];
    /** Array of replicas. */
    AtmosReplicaInfo *replicas;
    /** Number of replicas in array */
    int replica_count;
    /** Nonzero if retention is enabled on this object. */
    int retention_enabled;
    /** Time that retention ends (seconds since epoch, GMT) */
    time_t retention_end;
    /** Nonzero if expiration is enabled on this object. */
    int expiration_enabled;
    /**
     * Time that this object expires (seconds since epoch, GMT).  Note that
     * the expiration process runs daily, so the object will persist from
     * expiration time until the next expiration process runs.
     */
    time_t expiration_end;
} AtmosGetObjectInfoResponse;

/**
 * Initializes a new AtmosGetObjectInfoResponse object.
 * @param self pointer to the AtmosGetObjectInfoResponse object to initialize.
 * @return the AtmosGetObjectInfoResponse object (same as 'self')
 */
AtmosGetObjectInfoResponse*
AtmosGetObjectInfoResponse_init(AtmosGetObjectInfoResponse *self);

/**
 * Destroys a AtmosGetObjectInfoResponse object.
 * @param self pointer to the AtmosGetObjectInfoResponse object to destroy.
 */
void
AtmosGetObjectInfoResponse_destroy(AtmosGetObjectInfoResponse *self);

/**
 * @}
 * @defgroup AtmosPaginatedRequest AtmosPaginatedRequest
 * @brief This module contains the AtmosPaginatedRequest class that is used
 * as a parent class for requests that contain paginated results.
 * @{
 */

/**
 * An AtmosPaginatedRequest is the base class for operations that have
 * paginated results (list directory, list objects, get listable tags).
 */
typedef struct {
    /** Parent object */
    RestRequest parent;
    /** Number of items per page to request */
    int limit;
    /** Page continuation token.  Empty on the first request */
    char token[ATMOS_TOKEN_MAX];
} AtmosPaginatedRequest;

/**
 * Initializes a new AtmosPaginatedRequest object.
 * @param self the AtmosPaginatedRequest object to initialize.
 * @param uri the URI request string.
 * @param method the HTTP method to use for the request.
 * @return the AtmosPaginatedRequest object (same as 'self')
 */
AtmosPaginatedRequest*
AtmosPaginatedRequest_init(AtmosPaginatedRequest *self, const char *uri,
        enum http_method method);

/**
 * Destroys an AtmosPaginatedRequest object
 * @param self the AtmosPaginatedRequest object to destroy.
 */
void
AtmosPaginatedRequest_destroy(AtmosPaginatedRequest *self);

/**
 * @}
 * @defgroup AtmosListDirectoryRequest AtmosListDirectoryRequest
 * @brief This module contains the AtmosListDirectoryRequest class that is used
 * to request a directory listing from Atmos.
 * @{
 */

/**
 * Request object that enacapsulates the parameters for listing a directory.
 */
typedef struct {
    /** parent object */
    AtmosPaginatedRequest parent;
    /** Path to the directory being listed */
    char path[ATMOS_PATH_MAX];
    /**
     * If nonzero, include metadata for each object in the directory.  By
     * default, this is all the metadata for the object.  Use the user_tags
     * and system_tags to limit the fields returned.
     */
    int include_meta;
    /** List of user metadata tags to return for each object */
    char user_tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of items in user_tags */
    int user_tag_count;
    /** List of system metadata tags to return for each object */
    char system_tags[ATMOS_SYSTEM_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of items in system_tags */
    int system_tag_count;
} AtmosListDirectoryRequest;

/**
 * Initializes a new AtmosListDirectory request object.
 * @param self the AtmosListDirectoryRequest to initialize.
 * @param path namespace path to the directory to list.
 * @return the AtmosListDirectoryRequest object (same as 'self')
 */
AtmosListDirectoryRequest*
AtmosListDirectoryRequest_init(AtmosListDirectoryRequest *self,
        const char *path);

/**
 * Destroys an AtmosListDirectoryRequest object.
 * @param self the AtmosListDirectoryRequest object to destroy.
 */
void
AtmosListDirectoryRequest_destroy(AtmosListDirectoryRequest *self);

/**
 * Adds a tag to the list of user metadata elements to include with the
 * directory listing.  Be sure to set include_metadata to true.  If you don't
 * call either add_user_tag or add_system_tag, all metadata elements for
 * all the objects will be returned.
 * @param self the AtmosListDirectoryRequest to modify.
 * @param tag the user metadata tag name to include in the results.
 */
void
AtmosListDirectoryRequest_add_user_tag(AtmosListDirectoryRequest *self,
        const char *tag);

/**
 * Adds a tag to the list of system metadata elements to include with the
 * directory listing.  Be sure to set include_metadata to true.  If you don't
 * call either add_user_tag or add_system_tag, all metadata elements for all
 * of the objects will be returned.
 * @param self the AtmosListDirectoryRequest to modify.
 * @param tag the system metadata tag name to include in the results.  See the
 * ATMOS_SYSTEM_META_* macros for valid values.
 */
void
AtmosListDirectoryRequest_add_system_tag(AtmosListDirectoryRequest *self,
        const char *tag);

/**
 * @}
 * @defgroup AtmosListDirectoryResponse AtmosListDirectoryResponse
 * @brief This module contains the AtmosListDirectoryResponse class that is used
 * to capture a directory listing response from Atmos.
 * @{
 */

/**
 * Contains common object metadata used by AtmosDirectoryEntry and
 * AtmosObjectListing.
 */
typedef struct {
    /** Parent class */
    Object parent;
    /** Array of AtmosMetadata containing the regular metadata for the object */
    AtmosMetadata *meta;
    /** Number of elements in meta */
    int meta_count;
    /** Array of AtmosMetadata containing the listable metadata for the object*/
    AtmosMetadata *listable_meta;
    /** Number of elements in listable_meta */
    int listable_meta_count;
    /** Structure containing the system metadata properties for the object */
    AtmosSystemMetadata system_metadata;
} AtmosObjectMetadata;

/**
 * Initializes a new AtmosObjectMetadata object.
 * @param self the AtmosObjectMetadata object to initialize.
 * @return the AtmosObjectMetadata object (same as 'self')
 */
AtmosObjectMetadata*
AtmosObjectMetadata_init(AtmosObjectMetadata *self);

/**
 * Destroys an AtmosObjectMetadata object.
 * @param self the AtmosObjectMetadata object to destroy.
 */
void
AtmosObjectMetadata_destroy(AtmosObjectMetadata *self);


/**
 * Contains all of the information for one object in a directory listing.
 */
typedef struct {
    /** Parent object */
    AtmosObjectMetadata parent;
    /** The Object's ID */
    char object_id[ATMOS_OID_LENGTH];
    /** The type of file.  Either ATMOS_TYPE_REGULAR or ATMOS_TYPE_DIRECTORY */
    const char *type;
    /** Filename of the object (local name only, not full path) */
    char filename[ATMOS_PATH_MAX];
} AtmosDirectoryEntry;

/**
 * Initializes an AtmosDirectoryEntry object.
 * @param self the AtmosDirectoryEntry object to initialize
 * @return the AtmosDirectoryEntry object (same as 'self')
 */
AtmosDirectoryEntry*
AtmosDirectoryEntry_init(AtmosDirectoryEntry *self);

/**
 * Destroys an AtmosDirectoryEntry object.
 * @param self the AtmosDirectoryEntry object to destroy.
 */
void
AtmosDirectoryEntry_destroy(AtmosDirectoryEntry *self);

/**
 * Returns the value of a metadata element on the directory entry.
 * @param self the AtmosDirectoryEntry to search.
 * @param name the name of the metadata element to look for
 * @param listable zero for regular metadata, nonzero for listable metadata.
 * @return the value of the metadata element or NULL if the element was not
 * found on the directory entry.
 */
const char *
AtmosDirectoryEntry_get_metadata_value(AtmosDirectoryEntry *self,
        const char *name, int listable);

/**
 * Contains the results for a list directory operation.
 */
typedef struct {
    /** Parent object.  Also contains the metadata for the directory itself */
    AtmosReadObjectResponse parent;
    /**
     * Pagination continuation token.  If not NULL, there are more results.
     * Copy this token value into another AtmosListDirectoryRequest and execute
     * it for more results.
     */
    const char *token;
    /**
     * Array of AtmosDirectoryEntry objects containing the list of objects in
     * the directory.
     */
    AtmosDirectoryEntry *entries;
    /**
     * Number of items in entries.
     */
    int entry_count;
} AtmosListDirectoryResponse;

/**
 * Initializes a new AtmosListDirectoryResponse object.
 * @param self the AtmosListDirectoryResponse object to initialize.
 * @return the AtmosListDirectoryRespnose object (same as 'self')
 */
AtmosListDirectoryResponse*
AtmosListDirectoryResponse_init(AtmosListDirectoryResponse *self);

/**
 * Destroys an AtmosListDirectoryResponse object.
 * @param self the AtmosListDirectoryResponse object to destroy.
 */
void
AtmosListDirectoryResponse_destroy(AtmosListDirectoryResponse *self);

/**
 * @}
 * @defgroup AtmosGetListableTagsRequest AtmosGetListableTagsRequest
 * @brief This module contains the AtmosGetListableTagsRequest class that is
 * used to request the set of listable tags from Atmos.
 * @{
 */

/**
 * The AtmosGetListableTagsRequest object encapsulates the parameters for a
 * get listable tags operation.
 * Note that even though this class extends AtmosPaginatedRequest, the "limit"
 * field is not used.  The server will always decide how many tags to return
 * based on server load and system parameters.
 */
typedef struct {
    /** Parent class */
    AtmosPaginatedRequest parent;
    /** Parent tag to list the child tags of.  Empty to list the top level tags*/
    char parent_tag[ATMOS_PATH_MAX];
} AtmosGetListableTagsRequest;

/**
 * Initializes a new AtmosGetListableTagsRequest object.
 * @param self the AtmosGetListableTagsRequest object to initialize.
 * @param parent_tag the parent tag to list the subtags for.  Set to NULL to
 * fetch the top-level tags.
 * @return the AtmosGetListableTagsRequest object (same as self)
 */
AtmosGetListableTagsRequest*
AtmosGetListableTagsRequest_init(AtmosGetListableTagsRequest *self,
        const char *parent_tag);

/**
 * Destroys an AtmosGetListableTagsRequest object.
 * @param self the AtmosGetListableTagsRequest object to destroy.
 */
void
AtmosGetListableTagsRequest_destroy(AtmosGetListableTagsRequest *self);

/**
 * @}
 * @defgroup AtmosGetListableTagsResponse AtmosGetListableTagsResponse
 * @brief This module contains the AtmosGetListableTagsResponse class that is
 * used to capture a listable tag listing response from Atmos.
 * @{
 */

/**
 * Contains the response from a get listable tags operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /**
     * Pagination token.  If not NULL, there are more results available.  Copy
     * this token into another AtmosGetListableTagsRequest and execute it to
     * get more results.
     */
    const char *token;
    /** Array of listable tag names returned from the operation */
    char **tags;
    /** number of items in tags */
    int tag_count;
} AtmosGetListableTagsResponse;

/**
 * Initializes an AtmosGetListableTagsResponse object.
 * @param self the AtmosGetListableTagsResponse object to initialize.
 * @return the AtmosGetListableTagsResponse object (Same as 'self').
 */
AtmosGetListableTagsResponse*
AtmosGetListableTagsResponse_init(AtmosGetListableTagsResponse *self);

/**
 * Destroys an AtmosGetListableTagsResponse object.
 * @param self the AtmosGetListableTagsResponse object to destroy.
 */
void
AtmosGetListableTagsResponse_destroy(AtmosGetListableTagsResponse *self);

/**
 * @}
 * @defgroup AtmosListObjectsRequest AtmosListObjectsRequest
 * @brief This module contains the AtmosListObjectsRequest class that is
 * used to list objects indexed by a tag in Atmos.
 * @{
 */

/**
 * Encapsulates the parameters for a list objects request.  This lists the
 * objects associated with a listable tag.
 */
typedef struct {
    /** Parent object */
    AtmosPaginatedRequest parent;
    /** Listable tag getting listed */
    char tag[ATMOS_PATH_MAX];
    /** If nonzero, include object metadata with each object */
    int include_meta;
    /**
     * List of user metadata tags to include in the listing.  If both
     * user_tags and system_tags are empty, all metadata for the object will
     * be returned.
     */
    char user_tags[ATMOS_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of items in user_tags */
    int user_tag_count;
    /**
     * List of system metadata tags to include in the listing.  See the
     * ATMOS_SYSTEM_META_* macros for valid values.
     */
    char system_tags[ATMOS_SYSTEM_META_COUNT_MAX][ATMOS_META_NAME_MAX];
    /** Number of items in system_tags */
    int system_tag_count;
} AtmosListObjectsRequest;

/**
 * Initializes a new AtmosListObjectsRequest.
 * @param self the AtmosListObjectsRequest to initialize.
 * @param tag the tag you wish to get a listing for.
 * @return the AtmosListObjectsRequest (same as 'self')
 */
AtmosListObjectsRequest*
AtmosListObjectsRequest_init(AtmosListObjectsRequest *self, const char *tag);

/**
 * Destroys an AtmosListObjectsRequest object.
 * @param self the AtmosListObjectsRequest to destroy.
 */
void
AtmosListObjectsRequest_destroy(AtmosListObjectsRequest *self);

/**
 * Adds a tag to the list of user metadata elements to include with the
 * object listing.  Be sure to set include_metadata to true.  If you don't
 * call either add_user_tag or add_system_tag, all metadata elements for
 * all the objects will be returned.
 * @param self the AtmosListObjectsRequest to modify.
 * @param tag the user metadata tag name to include in the results.
 */
void
AtmosListObjectsRequest_add_user_tag(AtmosListObjectsRequest *self,
        const char *tag);

/**
 * Adds a tag to the list of system metadata elements to include with the
 * object listing.  Be sure to set include_metadata to true.  If you don't
 * call either add_user_tag or add_system_tag, all metadata elements for all
 * of the objects will be returned.
 * @param self the AtmosListObjectsRequest to modify.
 * @param tag the system metadata tag name to include in the results.  See the
 * ATMOS_SYSTEM_META_* macros for valid values.
 */
void
AtmosListObjectsRequest_add_system_tag(AtmosListObjectsRequest *self,
        const char *tag);


/**
 * @}
 * @defgroup AtmosListObjectsResponse AtmosListObjectsResponse
 * @brief This module contains the AtmosListObjectsResponse class that is
 * used to capture the list of objects in a tag from Atmos.
 * @{
 */

/**
 * Contains the results from an object listing.
 */
typedef struct {
    /** Parent object. */
    AtmosObjectMetadata parent;
    /** ID of the object. */
    char object_id[ATMOS_OID_LENGTH];
} AtmosObjectListing;

/**
 * Initializes a new AtmosObjectListing object.
 * @param self the AtmosObjectListing to initialize.
 * @return the AtmosObjectListing object (same as 'self')
 */
AtmosObjectListing*
AtmosObjectListing_init(AtmosObjectListing *self);

/**
 * Destroys an AtmosObjectListing object.
 * @param self the AtmosObjectListing object to destroy.
 */
void
AtmosObjectListing_destroy(AtmosObjectListing *self);

/**
 * Returns the value of a metadata element on the object entry.
 * @param self the AtmosObjectListing to search.
 * @param name the name of the metadata element to look for
 * @param listable zero for regular metadata, nonzero for listable metadata.
 * @return the value of the metadata element or NULL if the element was not
 * found on the directory entry.
 */
const char *
AtmosObjectListing_get_metadata_value(AtmosObjectListing *self,
        const char *name, int listable);

/**
 * Contains the response for a list objects operation.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /**
     * Pagination continuation token.  If not NULL, there are more results.
     * Copy this token value into another AtmosListObjectsRequest and execute
     * it for more results.
     */
    const char *token;
    /** Array of result objects */
    AtmosObjectListing *results;
    /** Number of result objects. */
    int result_count;
} AtmosListObjectsResponse;

/**
 * Initializes a new AtmosListObjectsResponse.
 * @param self the AtmosListObjectsResponse to initialize.
 * @return the AtmosListObjectsResponse object (same as 'self')
 */
AtmosListObjectsResponse*
AtmosListObjectsResponse_init(AtmosListObjectsResponse *self);

/**
 * Destroys an AtmosListObjectsResponse object.
 * @param self the AtmosListObjectsResponse object to destroy.
 */
void
AtmosListObjectsResponse_destroy(AtmosListObjectsResponse *self);

/**
 * @}
 */


#endif /* ATMOS_READ_H_ */
