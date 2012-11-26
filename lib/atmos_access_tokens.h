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

#ifndef ATMOS_ACCESS_TOKENS_H_
#define ATMOS_ACCESS_TOKENS_H_

/**
 * @file atmos_access_tokens.h
 * @brief This file contains the objects used to manipulate access tokens in
 * Atmos.
 */

#include "atmos.h"

/**
 * @defgroup AtmosCreateAccessTokenRequest AtmosCreateAccessTokenRequest
 * @brief This module contains the AtmosCreateAccessTokenRequest class that is
 * used to create a new access token in Atmos
 * @{
 */
typedef struct {
    RestRequest parent;
    char object_id[ATMOS_OID_LENGTH];
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
    // TODO: policy
} AtmosCreateAccessTokenRequest;

AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init(AtmosCreateAccessTokenRequest *self);

AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_obj(AtmosCreateAccessTokenRequest *self,
        const char *object_id);

AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_ns(AtmosCreateAccessTokenRequest *self,
        const char *path);

void
AtmosCreateAccessTokenRequest_destroy(AtmosCreateAccessTokenRequest *self);

/**
 * @}
 * @defgroup AtmosCreateAccessTokenResponse AtmosCreateAccessTokenResponse
 * @brief This module contains the AtmosCreateAccessTokenResponse class that is
 * used to capture the create access token response from Atmos
 * @{
 */

typedef struct {
    AtmosResponse parent;
    char token_id[ATMOS_OID_LENGTH];
} AtmosCreateAccessTokenResponse;

AtmosCreateAccessTokenResponse*
AtmosCreateAccessTokenResponse_init(AtmosCreateAccessTokenResponse *self);

void
AtmosCreateAccessTokenResponse_destroy(AtmosCreateAccessTokenResponse *self);

/**
 * @}
 * @defgroup AtmosListAccessTokensRequest AtmosListAccessTokensRequest
 * @brief This module contains the AtmosListAccessTokensRequest class that is
 * used to list access tokens from Atmos
 * @{
 */
typedef struct {
    RestRequest parent;
    char token[ATMOS_OID_LENGTH];
    int limit;
} AtmosListAccessTokensRequest;

AtmosListAccessTokensRequest*
AtmosListAccessTokensRequest_init(AtmosListAccessTokensRequest *self);

void
AtmosListAccessTokensRequest_destroy(AtmosListAccessTokensRequest *self);

/**
 * @}
 * @defgroup AtmosListAccessTokensResponse AtmosListAccessTokensResponse
 * @brief This module contains the AtmosListAccessTokensResponse class that is
 * used to capture the list of access tokens from Atmos
 * @{
 */
typedef struct {
    AtmosResponse parent;
    // TODO: finish
} AtmosListAccessTokensResponse;

AtmosListAccessTokensResponse*
AtmosListAccessTokensResponse_init(AtmosListAccessTokensResponse *self);

void
AtmosListAccessTokensResponse_destroy(AtmosListAccessTokensResponse *self);

/**
 * @}
 * @defgroup AtmosGetAccessTokenInfoResponse AtmosGetAccessTokenInfoResponse
 * @brief This module contains the AtmosGetAccessTokenInfoResponse class that is
 * used to capture the get access token info response from Atmos
 * @{
 */

typedef struct {
    AtmosResponse response;
    // TODO: finish
} AtmosGetAccessTokenInfoResponse;

AtmosGetAccessTokenInfoResponse*
AtmosGetAccessTokenInfoResponse_init(AtmosGetAccessTokenInfoResponse *self);

void
AtmosGetAccessTokenInfoResponse_destroy(AtmosGetAccessTokenInfoResponse *self);

#endif /* ATMOS_ACCESS_TOKENS_H_ */
