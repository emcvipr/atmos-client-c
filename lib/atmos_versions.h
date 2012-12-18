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
#ifndef ATMOS_VERSIONS_H_
#define ATMOS_VERSIONS_H_

/**
 * @file atmos_versions.h
 * @brief This file contains the objects used to manipulate object versions
 * in Atmos
 */

#include "atmos.h"

/**
 * @defgroup AtmosCreateVersionRequest AtmosCreateVersionRequest
 * @brief This module contains the AtmosCreateVersionRequest class that is
 * used to create a new version of an object in Atmos
 * @{
 */

/** Contains the parameters for the create object version request */
typedef struct {
    /** Parent class */
    RestRequest parent;
    /** Object ID to create a version of */
    char object_id[ATMOS_OID_LENGTH];
} AtmosCreateVersionRequest;

/**
 * Initializes a new AtmosCreateVersionRequest object.
 * @param self the AtmosCreateVersionRequest object to initialize.
 * @param object_id the Atmos Object ID to create a version of.
 * @return the AtmosCreateVersionRequest object (same as 'self')
 */
AtmosCreateVersionRequest*
AtmosCreateVersionRequest_init(AtmosCreateVersionRequest *self,
        const char *object_id);

/**
 * Destroys an AtmosCreateVersionRequest object.
 * @param self the AtmosCreateVersionRequest object to destroy.
 */
void
AtmosCreateVersionRequest_destroy(AtmosCreateVersionRequest *self);

/**
 * @}
 * @defgroup AtmosCreateVersionResponse AtmosCreateVersionResponse
 * @brief This module contains the AtmosCreateVersionRequest class that is
 * used to capture the create version response from Atmos
 * @{
 */
/** Contains the response from a create version operation */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /** The new version ID that was created */
    char version_id[ATMOS_OID_LENGTH];
} AtmosCreateVersionResponse;

/**
 * Initializes a new AtmosCreateVersionResponse object.
 * @param self the AtmosCreateVersionResponse object to initialize.
 * @return the AtmosCreateVersionResponse object (same as 'self')
 */
AtmosCreateVersionResponse*
AtmosCreateVersionResponse_init(AtmosCreateVersionResponse *self);

/**
 * Destroys an AtmosCreateVersionResponse object.
 * @param self the AtmosCreateVersionResponse object to destroy.
 */
void
AtmosCreateVersionResponse_destroy(AtmosCreateVersionResponse *self);

/**
 * @}
 * @defgroup AtmosListVersionsRequest AtmosListVersionsRequest
 * @brief This module contains the AtmosListVersionsRequest class that is
 * used to request the list of versions of an object from Atmos.
 * @{
 */

/** Contains the parameters for a list versions request */
typedef struct {
    /** Parent class. */
    AtmosPaginatedRequest parent;
    /** The Atmos Object ID to list versions for */
    char object_id[ATMOS_OID_LENGTH];
} AtmosListVersionsRequest;

/**
 * Initializes a new AtmosListVersionsRequest object.
 * @param self the AtmosListVersionsRequest object to initialize.
 * @param object_id the Atmos Object ID to retrieve versions for.
 * @return the AtmosListVersionsRequest object (same as 'self')
 */
AtmosListVersionsRequest*
AtmosListVersionsRequest_init(AtmosListVersionsRequest *self,
        const char *object_id);

/**
 * Destroys an AtmosListVersionsRequest object.
 * @param self the AtmosListVersionsRequest object to destroy.
 */
void
AtmosListVersionsRequest_destroy(AtmosListVersionsRequest *self);

/**
 * @}
 * @defgroup AtmosListVersionsResponse AtmosListVersionsResponse
 * @brief This module contains the AtmosListVersionsResponse class that is
 * used to capture the list of versions of an object from Atmos.
 * @{
 */

/** Contains information about an Atmos version */
typedef struct {
    /** Object ID of the version */
    char object_id[ATMOS_OID_LENGTH];
    /** Version sequence number */
    int version_number;
    /** Inception (create) time of the version */
    time_t itime;
} AtmosVersion;

/** Contains the response from an list versions request */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /**
     * Pagination token.  If not null, there are more results.  Issue another
     * list versions request for the object with this token set to fetch more
     * results.
     */
    const char *token;
    /** List of object versions */
    AtmosVersion *versions;
    /** Number of object versions in list */
    int version_count;
} AtmosListVersionsResponse;

/**
 * Initializes a new AtmosListVersionsResponse object.
 * @param self the AtmosListVersionsResponse object to initialize
 * @return the AtmosListVersionsResponse object (same as 'self')
 */
AtmosListVersionsResponse*
AtmosListVersionsResponse_init(AtmosListVersionsResponse *self);

/**
 * Destroys an AtmosListVersionsResponse object.
 * @param self the AtmosListVersionsResponse object to destroy.
 */
void
AtmosListVersionsResponse_destroy(AtmosListVersionsResponse *self);

/**
 * @}
 */



#endif /* ATMOS_VERSIONS_H_ */
