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
#include "atmos_token_policy.h"

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
    /** Optional. Policy specification for the access token */
    Atmos_policyType *policy;
} AtmosCreateAccessTokenRequest;

/**
 * Initializes a new AtmosCreateAccessTokenRequest object.
 * @param self the AtmosCreateAccessTokenRequest object to initialize.
 * @return the AtmosCreateAccessTokenRequest object (same as 'self')
 */
AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init(AtmosCreateAccessTokenRequest *self);

/**
 * Initializes a new AtmosCreateAccessTokenRequest object.  This version can
 * be used download or update an existing ObjectID.
 * @param self the AtmosCreateAccessTokenRequest to initialize.
 * @param object_id the Atmos ObjectID to associate with the token.
 * @return the AtmosCreateAccessTokenRequest object (same as 'self')
 */
AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_obj(AtmosCreateAccessTokenRequest *self,
        const char *object_id);

/**
 * Initializes a new AtmosCreateAccessTokenRequest object.  This version can
 * be used download or create/update an Atmos object by namespace path.
 * @param self the AtmosCreateAccessTokenRequest object to initialize.
 * @param path the Atmos namespace path to associate with the token.
 * @return the AtmosCreateAccessTokenRequest object (same as 'self')
 */
AtmosCreateAccessTokenRequest*
AtmosCreateAccessTokenRequest_init_ns(AtmosCreateAccessTokenRequest *self,
        const char *path);

/**
 * Destroys an AtmosCreateAccessTokenRequest object.
 * @param self the AtmosCreateAccessTokenRequest object to destroy.
 */
void
AtmosCreateAccessTokenRequest_destroy(AtmosCreateAccessTokenRequest *self);

/**
 * @}
 * @defgroup AtmosCreateAccessTokenResponse AtmosCreateAccessTokenResponse
 * @brief This module contains the AtmosCreateAccessTokenResponse class that is
 * used to capture the create access token response from Atmos
 * @{
 */

/**
 * Contains the response from the create access token request.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /** Access token ID that was created */
    char token_id[ATMOS_SIMPLE_HEADER_MAX];
} AtmosCreateAccessTokenResponse;

/**
 * Initializes a new AtmosCreateAccessTokenResponse object.
 * @param self the AtmosCreateAccessTokenResponse object to initialize.
 * @return the AtmosCreateAccessTokenResponse object (same as 'self')
 */
AtmosCreateAccessTokenResponse*
AtmosCreateAccessTokenResponse_init(AtmosCreateAccessTokenResponse *self);

/**
 * Destroys an AtmosCreateAccessTokenResponse object.
 * @param self the AtmosCreateAccessTokenResponse object to destroy.
 */
void
AtmosCreateAccessTokenResponse_destroy(AtmosCreateAccessTokenResponse *self);

/**
 * @}
 * @defgroup AtmosListAccessTokensRequest AtmosListAccessTokensRequest
 * @brief This module contains the AtmosListAccessTokensRequest class that is
 * used to list access tokens from Atmos
 * @{
 */

/** Contains the parameters for a list access tokens request */
typedef struct {
    /** Parent class */
    AtmosPaginatedRequest parent;
} AtmosListAccessTokensRequest;

/**
 * Initializes a new AtmosListAccessTokensRequest object.
 * @param self the AtmosListAccessTokensRequest object to initialize.
 * @return the AtmosListAccessTokensRequest object (same as 'self')
 */
AtmosListAccessTokensRequest*
AtmosListAccessTokensRequest_init(AtmosListAccessTokensRequest *self);

/**
 * Destroys an AtmosListAccessTokensRequest object.
 * @param self the AtmosListAccessTokensRequest object to destroy.
 */
void
AtmosListAccessTokensRequest_destroy(AtmosListAccessTokensRequest *self);

/**
 * @}
 * @defgroup AtmosListAccessTokensResponse AtmosListAccessTokensResponse
 * @brief This module contains the AtmosListAccessTokensResponse class that is
 * used to capture the list of access tokens from Atmos
 * @{
 */

/**
 * Encapsulates the response for a list access tokens request.
 */
typedef struct {
    /** Parent object */
    AtmosResponse parent;
    /**
     * Pagination token.  If not NULL, there are more tokens available to
     * list.  Issue another request with this token to get more results.
     */
    const char *token;
    /** The unmarshalled token list response */
    Atmos_listAccessTokenResultType *token_list;
} AtmosListAccessTokensResponse;

/**
 * Initializes a new AtmosListAccessTokensResponse object.
 * @param self the AtmosListAccessTokensResponse object to initialize.
 * @return the AtmosListAccessTokensResponse object (same as 'self')
 */
AtmosListAccessTokensResponse*
AtmosListAccessTokensResponse_init(AtmosListAccessTokensResponse *self);

/**
 * Destroys an AtmosListAccessTokensResponse object.
 * @param self the AtmosListAccessTokensResponse object to destroy.
 */
void
AtmosListAccessTokensResponse_destroy(AtmosListAccessTokensResponse *self);

/**
 * @}
 * @defgroup AtmosGetAccessTokenInfoResponse AtmosGetAccessTokenInfoResponse
 * @brief This module contains the AtmosGetAccessTokenInfoResponse class that is
 * used to capture the get access token info response from Atmos
 * @{
 */
/** Encapsulates a response from a "get access token info" operation */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /** The unmarshalled token info object */
    Atmos_accessTokenType *token_info;
} AtmosGetAccessTokenInfoResponse;

/**
 * Initializes a new AtmosGetAccessTokenInfoResponse object.
 * @param self the AtmosGetAccessTokenInfoResponse object to initialize.
 * @return the AtmosGetAccessTokenInfoResponse object (same as 'self')
 */
AtmosGetAccessTokenInfoResponse*
AtmosGetAccessTokenInfoResponse_init(AtmosGetAccessTokenInfoResponse *self);

/**
 * Destroys an AtmosGetAccessTokenInfoResponse object.
 * @param self the AtmosGetAccessTokenInfoResponse object to destroy.
 */
void
AtmosGetAccessTokenInfoResponse_destroy(AtmosGetAccessTokenInfoResponse *self);

#endif /* ATMOS_ACCESS_TOKENS_H_ */
