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
#ifndef ATMOS_UPDATE_H_
#define ATMOS_UPDATE_H_

/**
 * @file atmos_update.h
 * @brief This file contains the classes used to update objects in Atmos.
 */
#include "atmos.h"

/**
 * @defgroup AtmosUpdateObjectRequest AtmosUpdateObjectRequest
 * @brief This module contains the AtmosUpdateObjectRequest class that is used
 * to update objects in Atmos.
 * @{
 */

/**
 * Contains the parameters used to update an object in Atmos.
 */
typedef struct {
    /** Parent object */
    RestRequest parent;
    /** Object ID getting updated.  Will be empty if path is not */
    char object_id[ATMOS_UID_MAX];
    /**
     * Namespace path to the object getting updated.  Will be empty if
     * object_id is not.
     */
    char path[ATMOS_PATH_MAX];
    /**
     * For keypool requests, this is the pool used to create the object.  The
     * key will be stored in 'path'.
     */
    char pool[ATMOS_PATH_MAX];
    /** Metadata to apply to the object */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata entries */
    int meta_count;
    /** Listable metadata entries to apply to the object */
    AtmosMetadata listable_meta[ATMOS_META_COUNT_MAX];
    /** Number of listable metadata entries */
    int listable_meta_count;
    /** ACL entries to apply to the object. */
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    /** Number of ACL entries.  May be zero to keep the existing ACL. */
    int acl_count;
    /** Start of object range to update. Set to -1 for no start range */
    int64_t range_start;
    /** End of object range to update.  Set to -1 for no end range */
    int64_t range_end;
} AtmosUpdateObjectRequest;

/**
 * Initializes a new AtmosUpdateObjectRequest.
 * @param self the AtmosUpdateObjectRequest to initialize.
 * @param object_id the Atmos object ID to update.
 * @return the AtmosUpdateObjectRequest object (same as 'self')
 */
AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init(AtmosUpdateObjectRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosUpdateObjectRequest.
 * @param self the AtmosUpdateObjectRequest to initialize.
 * @param path the namespace path to the object in Atmos to update.
 * @return the AtmosUpdateObjectRequest object (same as 'self')
 */
AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_ns(AtmosUpdateObjectRequest *self,
        const char *path);

/**
 * Initializes a new AtmosUpdateObjectRequest.  This version updates an object
 * in an Atmos 2.1.0+ keypool.
 * @param self the AtmosUpdateObjectRequest to initialize.
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosUpdateObjectRequest object (same as 'self')
 * @since Atmos 2.1.0
 */
AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_keypool(AtmosUpdateObjectRequest *self,
        const char *pool, const char *key);

/**
 * Destroys an AtmosUpdateObjectRequest.
 * @param self the AtmosUpdateObjectRequest object to destroy.
 */
void
AtmosUpdateObjectRequest_destroy(AtmosUpdateObjectRequest *self);

/**
 * Adds a metadata entry to the update object request.
 * @param self the AtmosUpdateObjectRequest to modify.
 * @param name name for the metadata entry.
 * @param value value for the metadata entry.
 * @param listable nonzero if this entry's name should be 'listable'.  Use with
 * caution.  See programmer's documentation.
 */
void
AtmosUpdateObjectRequest_add_metadata(AtmosUpdateObjectRequest *self,
        const char *name, const char *value,
        int listable);

/**
 * Adds and ACL entry to the update object request.
 * @param self the AtmosUpdateObjectRequest to modify.
 * @param principal the ACL's user or group name.
 * @param type type of principal (ATMOS_USER or ATMOS_GROUP).
 * @param permission rights granted to the principal.
 */
void
AtmosUpdateObjectRequest_add_acl(AtmosUpdateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission);

/**
 * Sets the object content range to update, inclusive. Set to [-1,-1] to update
 * the entire object.  Set to [x,-1] to update from x to the end of the object.
 * Set to [-1,x] to update the last x bytes of an object.
 * @param self the AtmosUpdateObjectRequest to configure.
 * @param range_start the start of the object range to read.  Set to -1 for no
 * start range.
 * @param range_end the end of the object range to read.  Set to -1 for no end
 * range.
 */
void
AtmosUpdateObjectRequest_set_range(AtmosUpdateObjectRequest *self,
        int64_t range_start, int64_t range_end);

/**
 * A convenience function that allows you to specify the extent using offset
 * and size.
 * @param self the AtmosUpdateObjectRequest to configure.
 * @param offset start of the object range to read.
 * @param size the number of bytes to read starting at the offset.
 */
void
AtmosUpdateObjectRequest_set_range_offset_size(AtmosUpdateObjectRequest *self,
        int64_t offset, int64_t size);


/**
 * @}
 * @defgroup AtmosSetUserMetaRequest AtmosSetUserMetaRequest
 * @brief This module contains the AtmosSetUserMetaRequest class that is used
 * to set user metadata on an object in Atmos.
 * @{
 */

/**
 * Contains the parameters for a set user metadata operation.
 */
typedef struct {
    /** Parent class */
    RestRequest parent;
    /** Object ID to update.  Will be empty if path is set */
    char object_id[ATMOS_UID_MAX];
    /** Path (or key) to update.  Will be empty if object_id is set */
    char path[ATMOS_PATH_MAX];
    /** Pool containing the key to update.  Only used for keypool requests */
    char pool[ATMOS_PATH_MAX];
    /** Metadata entries for the new object */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata entries */
    int meta_count;
    /** Listable metadata entries for the new object */
    AtmosMetadata listable_meta[ATMOS_META_COUNT_MAX];
    /** Number of listable metadata entries */
    int listable_meta_count;
} AtmosSetUserMetaRequest;

/**
 * Initializes a new AtmosSetUserMetaRequest object.
 * @param self the AtmosSetUserMetaRequest object to initialize.
 * @param object_id the Atmos ID of the object to update.
 * @return the AtmosSetUserMetaRequest object (same as 'self')
 */
AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init(AtmosSetUserMetaRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosSetUserMetaRequest object.
 * @param self the AtmosSetUserMetaRequest object to initialize.
 * @param path the Atmos namespace path to the object to update.
 * @return the AtmosSetUserMetaRequest object (same as 'self')
 */
AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_ns(AtmosSetUserMetaRequest *self,
        const char *path);

/**
 * Initializes a new AtmosSetUserMetaRequest object.  This version updates an
 * object in an Atmos 2.1.0+ keypool.
 * @param self the AtmosSetUserMetaRequest object to initialize.
 * @param pool the name of the pool containing the object.
 * @param key the object key in the pool.
 * @return the AtmosSetUserMetaRequest object (same as 'self')
 * @since Atmos 2.1.0
 */
AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_keypool(AtmosSetUserMetaRequest *self,
        const char *pool, const char *key);


/**
 * Destroys an AtmosSetUserMetaRequest object.
 * @param self the AtmosSetUserMetaRequest object to destroy.
 */
void
AtmosSetUserMetaRequest_destroy(AtmosSetUserMetaRequest *self);

/**
 * Adds a metadata entry to the set user metadata request.
 * @param self the AtmosSetUserMetaRequest to modify.
 * @param name name for the metadata entry.
 * @param value value for the metadata entry.
 * @param listable nonzero if this entry's name should be 'listable'.  Use with
 * caution.  See programmer's documentation.
 */
void
AtmosSetUserMetaRequest_add_metadata(AtmosSetUserMetaRequest *self,
        const char *name, const char *value,
        int listable);

/**
 * @}
 */

#endif /* ATMOS_UPDATE_H_ */
