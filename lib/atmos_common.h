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
#ifndef ATMOS_COMMON_H_
#define ATMOS_COMMON_H_

/**
 * @file atmos_common.h
 * @brief This header contains common Atmos objects, structs, and enums
 */

/**
 * This is the base AtmosResponse object.  It adds Atmos error information to
 * the basic RestResponse.
 */
typedef struct {
    /** Parent class */
    RestResponse parent;
    /** The Atmos error code.  Set to zero if no Atmos error was returned */
    int atmos_error;
    /** The Atmos error message string */
    char atmos_error_message[ATMOS_ERROR_MAX];
} AtmosResponse;

/**
 * Types of Atmos principals used in ACLs.
 */
enum atmos_acl_principal_type {
    /** an Atmos user */
    ATMOS_USER,
    /** an Atmos group */
    ATMOS_GROUP
};

/**
 * Atmos permission levels used in ACLs.
 */
enum atmos_acl_permission {
    /** No operations are allowed on the object */
    ATMOS_PERM_NONE,
    /** The object's content and metadata may be read */
    ATMOS_PERM_READ,
    /** The object's content and metadata may be read or written */
    ATMOS_PERM_READ_WRITE,
    /** The object's content, metadata, and ACL may be read or written */
    ATMOS_PERM_FULL
};

/**
 * An Atmos ACL entry.
 */
typedef struct {
    /** The name of the user or group */
    char principal[ATMOS_UID_MAX];
    /** Type of principal: user or group */
    enum atmos_acl_principal_type type;
    /** permissions granted to the principal */
    enum atmos_acl_permission permission;
} AtmosAclEntry;

/**
 * Atmos metadata name-value pair.
 */
typedef struct {
    /** Metadata name */
    char name[ATMOS_META_NAME_MAX];
    /** Metadata value */
    char value[ATMOS_META_VALUE_MAX];
} AtmosMetadata;

/**
 * System-level object metadata.  This is the internal Atmos information about
 * each object.  It is not directly modifiable by the end-user.
 */
typedef struct {
    /** Atmos Object ID */
    char object_id[ATMOS_OID_LENGTH];
    /** Atmos object name.  Only set for namespace objects */
    char objname[ATMOS_PATH_MAX];
    /** Access time of the object */
    time_t atime;
    /** Last metadata modification time */
    time_t ctime;
    /** Object Inception (create) time. */
    time_t itime;
    /** Object content modification time */
    time_t mtime;
    /** Object size in bytes */
    int64_t size;
    /** Number of hard links to an object */
    int nlink;
    /** Object type.  Either "regular" or "directory" */
    const char *type;
    /** User that created the object. */
    char uid[ATMOS_UID_MAX];
    /** Group of the object.  Always 'apache' */
    char gid[ATMOS_UID_MAX];
    /** Atmos policy name applied to the object */
    char policyname[ATMOS_UID_MAX];
    /** Object checksum, if applicable */
    char wschecksum[ATMOS_CHECKSUM_MAX];
} AtmosSystemMetadata;

/**
 * Initializes a new AtmosResponse.
 * @param self pointer to the AtmosResponse object.
 */
AtmosResponse*
AtmosResponse_init(AtmosResponse *self);

/**
 * Destroys an AtmosResponse.
 * @param self pointer to the AtmosResponse object.
 */
void
AtmosResponse_destroy(AtmosResponse *self);


#endif /* ATMOS_COMMON_H_ */
