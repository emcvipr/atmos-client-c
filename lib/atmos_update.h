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
 * @file atmos_read.h
 * @brief This file contains the classes used to update objects in Atmos.
 */
#include "atmos.h"


typedef struct {
    RestRequest parent;
    char object_id[ATMOS_UID_MAX];
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
} AtmosUpdateObjectRequest;

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init(AtmosUpdateObjectRequest *self,
        const char *object_id);

AtmosUpdateObjectRequest*
AtmosUpdateObjectRequest_init_ns(AtmosUpdateObjectRequest *self,
        const char *path);

void
AtmosUpdateObjectRequest_destroy(AtmosUpdateObjectRequest *self);

typedef struct {
    RestRequest parent;
    char object_id[ATMOS_UID_MAX];
    char path[ATMOS_PATH_MAX];
    /** Metadata entries for the new object */
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    /** Number of metadata entries */
    int meta_count;
    /** Listable metadata entries for the new object */
    AtmosMetadata listable_meta[ATMOS_META_COUNT_MAX];
    /** Number of listable metadata entries */
    int listable_meta_count;
} AtmosSetUserMetaRequest;

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init(AtmosSetUserMetaRequest *self,
        const char *object_id);

AtmosSetUserMetaRequest*
AtmosSetUserMetaRequest_init_ns(AtmosSetUserMetaRequest *self,
        const char *path);

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

#endif /* ATMOS_UPDATE_H_ */
