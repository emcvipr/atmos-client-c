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
typedef struct {
    RestRequest parent;
    char object_id[ATMOS_OID_LENGTH];
} AtmosCreateVersionRequest;

AtmosCreateVersionRequest*
AtmosCreateVersionRequest_init(AtmosCreateVersionRequest *self,
        const char *object_id);

void
AtmosCreateVersionRequest_destroy(AtmosCreateVersionRequest *self);

/**
 * @}
 * @defgroup AtmosCreateVersionResponse AtmosCreateVersionResponse
 * @brief This module contains the AtmosCreateVersionRequest class that is
 * used to capture the create version response from Atmos
 * @{
 */
typedef struct {
    AtmosResponse parent;
    char version_id[ATMOS_OID_LENGTH];
} AtmosCreateVersionResponse;

AtmosCreateVersionResponse*
AtmosCreateVersionResponse_init(AtmosCreateVersionResponse *self);

void
AtmosCreateVersionResponse_destroy(AtmosCreateVersionResponse *self);

/**
 * @}
 * @defgroup AtmosListVersionsRequest AtmosListVersionsRequest
 * @brief This module contains the AtmosListVersionsRequest class that is
 * used to request the list of versions of an object from Atmos.
 * @{
 */

typedef struct {
    AtmosPaginatedRequest parent;
    char object_id[ATMOS_OID_LENGTH];
} AtmosListVersionsRequest;

AtmosListVersionsRequest*
AtmosListVersionsRequest_init(AtmosListVersionsRequest *self,
        const char *object_id);

void
AtmosListVersionsRequest_destroy(AtmosListVersionsRequest *self);

/**
 * @}
 * @defgroup AtmosListVersionsResponse AtmosListVersionsResponse
 * @brief This module contains the AtmosListVersionsResponse class that is
 * used to capture the list of versions of an object from Atmos.
 * @{
 */
typedef struct {
    char object_id[ATMOS_OID_LENGTH];
    int version_number;
    time_t itime;
} AtmosVersion;

typedef struct {
    AtmosResponse parent;
    const char *token;
    AtmosVersion *versions;
    int version_count;
} AtmosListVersionsResponse;

AtmosListVersionsResponse*
AtmosListVersionsResponse_init(AtmosListVersionsResponse *self);

void
AtmosListVersionsResponse_destroy(AtmosListVersionsResponse *self);

/**
 * @}
 */



#endif /* ATMOS_VERSIONS_H_ */
