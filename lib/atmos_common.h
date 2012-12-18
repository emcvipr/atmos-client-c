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

#include <stdint.h>
#include <stdio.h>

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

/**
 * Sets an API-level error.  This also sets the HTTP code to 400 (Bad Request).
 * @param self the AtmosResponse in error.
 * @param code the API error code.
 * @param message the API error message.
 */
void
AtmosResponse_set_api_error(AtmosResponse *self, int code, const char *message);

/**
 * @defgroup AtmosChecksum AtmosChecksum
 * @brief This module contains the AtmosChecksum class that is used
 * for calculating and verifying checksums on objects.
 * @{
 */

/**
 * Atmos permission levels used in ACLs.
 */
enum atmos_checksum_mode {
    /**
     * This is the traditional checksum mode (x-emc-wschecksum) implemented
     * in atmos 1.4 and above.  In this mode, the request data is checksummed
     * before sending to the server and the server will return an error if the
     * data checksum did not match.
     */
    ATMOS_WSCHECKSUM,
    /**
     * This is the new checksum mode from Atmos 2.1 and above.  In this mode,
     * we request that the server generate a checksum and in parallel we
     * also compute the checksum.  After the operation is complete, we can
     * compare the checksums and determine on the client side whether the
     * operation was successful.  This version performs better on large
     * objects but lacks some of the automatic checks on the server side.  Also,
     * if you use this method during an object append, it will replace the
     * whole object checksum, making it invalid (i.e. so only use this for
     * creates or replace).
     */
    ATMOS_GENERATE_CHECKSUM
};

/**
 * Supported checksumming algorithms.
 */
enum atmos_checksum_alg {
    /** The SHA-0 or 'SHA' algorithm.  Supported in Atmos 1.4.0+ */
    ATMOS_ALG_SHA0,
    /**
     * SHA-1.  Supported in Atmos 2.1.0+.  Recommended for new applications
     * where backwards compatibility with Atmos <2.1.0 is not required.
     */
    ATMOS_ALG_SHA1,
    /** MD-5.  Supported in Atmos 2.1.0+ */
    ATMOS_ALG_MD5
};

/**
 * Object to handle checksum state information.  If you are doing multiple
 * object appends with ATMOS_WSCHECKSUM (i.e. create, append, append, ...), you
 * must reuse this checksum object for all operations to "continue" the object
 * checksum state.
 */
typedef struct {
    /** Parent object */
    Object parent;
    /** Checksumming mode */
    enum atmos_checksum_mode mode;
    /** Checksumming algorithm used: SHA0, SHA1 (2.1+), or MD5 (2.1+) */
    enum atmos_checksum_alg alg;
    /** String rep of algorithm used: SHA0, SHA1 (2.1+), or MD5 (2.1+) */
    const char *algorithm;
    /** Byte count in checksum (offset into object checksum was computed at) */
    int64_t offset;
    /** Internal checksum state information */
    void *internal_state;
} AtmosChecksum;

/**
 * Initializes a new AtmosChecksum object.
 * @param self the AtmosChecksum to initialize.
 * @param mode the checksumming mode that's going to be used on the request.
 * @param algorithm the checksumming algorithm to be used.
 * @param self the AtmosChecksum object (same as 'self').
 */
AtmosChecksum*
AtmosChecksum_init(AtmosChecksum *self, enum atmos_checksum_mode mode,
        enum atmos_checksum_alg algorithm);

/**
 * Destroys an AtmosChecksum object.
 * @param self the AtmosChecksum object to destroy
 */
void
AtmosChecksum_destroy(AtmosChecksum *self);

/**
 * Updates a checksum with new data.
 * @param self the AtmosChecksum object to update.
 * @param data bytes to append to the checksum
 * @param count number of bytes to read from 'data'.
 */
void
AtmosChecksum_update(AtmosChecksum *self, const char *data, size_t count);

/**
 * Updates a checksum with new data.
 * @param self the AtmosChecksum object to update.
 * @param data bytes to append to the checksum
 * @param count number of bytes to read from 'data'.
 */
void
AtmosChecksum_update_file(AtmosChecksum *self, FILE *data, int64_t count);

/**
 * Serializes the object's state into a form suitable for sending to Atmos
 * or using in AtmosChecksum_init_existing().  Note that this method is safe
 * to call multiple times.  It copies the internal state before 'finalizing'
 * the checksum so it won't break if you call it more than once.
 * @param self the AtmosChecksum object to get the state of.
 * @return serialized state of the checksum (e.g.
 * "sha0/1037/6754eeaad9d752f079dcb9ab224ab716720b9dda").  Note that the
 * caller is responsible for calling free() on this string.
 */
char *
AtmosChecksum_get_state(AtmosChecksum *self);

/**
 * Gets the current value of the checksum, hex encoded.  Note that this
 * method is safe to call multiple times.  It copies the internal state
 * before 'finalizing' the checksum so it won't break if you call it more
 * than once.
 * @param self the AtmosChecksum object to get the value of.
 * @return value checksum (e.g. "6754eeaad9d752f079dcb9ab224ab716720b9dda").
 * Note that the caller is responsible for calling free() on this string.
 */
char *
AtmosChecksum_get_value(AtmosChecksum *self);

/**
 * When using checksum mode ATMOS_GENERATE_CHECKSUM, call this method against
 * the response.  The generated checksum will be extracted from the response
 * and compared with the locally generated value.
 * @param self the AtmosChecksum generated locally
 * @param response the RestResponse from the server to examine for the checksum.
 * @return 1 if the checksum is valid, 0 otherwise.  If the checksum mode is
 * not ATMOS_GENERATE_CHECKSUM, 1 is returned.
 * @since Atmos 2.1.0
 */
int
AtmosChecksum_verify_response(AtmosChecksum *self, RestResponse *response);

/**
 * @}
 */

/**
 * @defgroup AtmosWriteObjectRequest AtmosWriteObjectRequest
 * @brief This module contains the AtmosWriteObjectRequest class that is used
 * as a parent for both write request types (create and update)
 * @{
 */
typedef struct {
    RestRequest parent;
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
    /** If set, checksumming will be used during the operation */
    AtmosChecksum *checksum;
} AtmosWriteObjectRequest;

/**
 * Initializes a new AtmosWriteObjectRequest object.
 * @param self the AtmosWriteObjectRequest object to initialize.
 * @param uri the URI for the HTTP request.
 * @param method the HTTP method for the request (e.g. PUT or POST).
 * @return the AtmosWriteObjectRequest object (same as 'self').
 */
AtmosWriteObjectRequest*
AtmosWriteObjectRequest_init(AtmosWriteObjectRequest *self, const char *uri,
        enum http_method method);

/**
 * Destroys an AtmosWriteObjectRequest object.
 *
 */
void
AtmosWriteObjectRequest_destroy(AtmosWriteObjectRequest *self);

/**
 * Adds and ACL entry to the update object request.
 * @param self the AtmosWriteObjectRequest to modify.
 * @param principal the ACL's user or group name.
 * @param type type of principal (ATMOS_USER or ATMOS_GROUP).
 * @param permission rights granted to the principal.
 */
void
AtmosWriteObjectRequest_add_acl(AtmosWriteObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission);

/**
 * Adds a metadata entry to the update object request.
 * @param self the AtmosWriteObjectRequest to modify.
 * @param name name for the metadata entry.
 * @param value value for the metadata entry.
 * @param listable nonzero if this entry's name should be 'listable'.  Use with
 * caution.  See programmer's documentation.
 */
void
AtmosWriteObjectRequest_add_metadata(AtmosWriteObjectRequest *self,
        const char *name, const char *value,
        int listable);

/*
 * @}
 */


#endif /* ATMOS_COMMON_H_ */
