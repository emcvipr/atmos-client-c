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
#ifndef ATMOS_CREATE_H_
#define ATMOS_CREATE_H_

/**
 * @file atmos_create.h
 * @brief This module contains the AtmosCreateObjectRequest class that is used
 * to create new objects in Atmos.
 * @defgroup AtmosCreateObjectRequest AtmosCreateObjectRequest
 * @brief This module contains the AtmosCreateObjectRequst class that is used
 * to create new objects in Atmos.
 * @{
 */

/**
 * This object contains the properties used to create a new object in Atmos.
 */
typedef struct {
    /** Parent class */
    RestRequest parent;
    /**
     * Atmos namespace path to create the object at.  Can be empty to create
     * an object with only an ID.
     */
    char path[ATMOS_PATH_MAX];
    /** Metadata entries for the new object */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata entries */
    int meta_count;
    /** Listable metadata entries for the new object */
    AtmosMetadata listable_meta[ATMOS_META_COUNT_MAX];
    /** Number of listable metadata entries */
    int listable_meta_count;
    /** ACL entries for the new object. */
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    /** Number of ACL entries.  May be zero to use the default ACL */
    int acl_count;
} AtmosCreateObjectRequest;

/**
 * Captures the create object response.  In particular, we're interested in the
 * object ID of the newly created object.
 */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /** ID of newly created object. */
    char object_id[ATMOS_OID_LENGTH];
} AtmosCreateObjectResponse;

/**
 * Initializes a new create object request.  With this version, an object will
 * be created with an Object ID only.
 * @param self pointer to the AtmosCreateObjectRequest to initialize.
 */
AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init(AtmosCreateObjectRequest *self);

/**
 * Initializes a new create object request.  With this version, an object wil
 * be created on the given path.
 * @param self pointer to the AtmosCreateObjectRequest to initialize.
 * @param path the Atmos namespace path for the new file, e.g. /dir/image.jpeg
 * or directory, e.g. /dir1/dir2/
 */
AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init_ns(AtmosCreateObjectRequest *self,
        const char *path);

/**
 * Destroys an AtmosCreateObjectRequest.
 * @param self pointer to the AtmosCreateObjectRequest to destroy.
 */
void
AtmosCreateObjectRequest_destroy(AtmosCreateObjectRequest *self);

/**
 * Adds a metadata entry to the create object request.
 * @param self the AtmosCreateObjectRequest to modify.
 * @param name name for the metadata entry.
 * @param value value for the metadata entry.
 * @param listable nonzero if this entry's name should be 'listable'.  Use with
 * caution.  See programmer's documentation.
 */
void
AtmosCreateObjectRequest_add_metadata(AtmosCreateObjectRequest *self,
        const char *name, const char *value,
        int listable);

/**
 * Adds and ACL entry to the create object request.
 * @param self the AtmosCreateObjectRequest to modify.
 * @param principal the ACL's user or group name.
 * @param type type of principal (ATMOS_USER or ATMOS_GROUP).
 * @param permission rights granted to the principal.
 */
void
AtmosCreateObjectRequest_add_acl(AtmosCreateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission);

/**
 * Initializes a new AtmosCreateObjectResponse.
 * @param self pointer to the AtmosCreateObjectResponse to initialize.
 */
AtmosCreateObjectResponse*
AtmosCreateObjectResponse_init(AtmosCreateObjectResponse *self);

/**
 * Destroys an AtmosCreateObjectResponse
 * @param self the AtmosCreateObjectResponse to destroy.
 */
void
AtmosCreateObjectResponse_destroy(AtmosCreateObjectResponse *self);

/**
 * @}
 */

#endif /* ATMOS_CREATE_H_ */
