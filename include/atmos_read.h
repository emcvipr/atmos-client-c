/*
 * atmos_read.h
 *
 *  Created on: Nov 19, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_READ_H_
#define ATMOS_READ_H_

#include "atmos.h"

typedef struct {
    RestRequest parent;
    char path[ATMOS_PATH_MAX];
    char object_id[ATMOS_OID_LENGTH];
    int64_t range_start;
    int64_t range_end;
} AtmosReadObjectRequest;

AtmosReadObjectRequest*
AtmosReadObjectRequest_init(AtmosReadObjectRequest *self,
        const char *object_id);

AtmosReadObjectRequest*
AtmosReadObjectRequest_init_ns(AtmosReadObjectRequest *self, const char *path);

void
AtmosReadObjectRequest_set_range(AtmosReadObjectRequest *self,
        int64_t range_start, int64_t range_end);

void
AtmosReadObjectRequest_destroy(AtmosReadObjectRequest *self);

/**
 * A convenience function that allows you to specify the extent using offset
 * and size.
 */
void
AtmosReadObjectRequest_set_range_offset_size(AtmosReadObjectRequest *self,
        int64_t offset, int64_t size);

typedef struct {
    AtmosResponse parent;
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    int meta_count;
    AtmosMetadata listable_metadata[ATMOS_META_COUNT_MAX];
    int listable_meta_count;
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    int acl_count;
    AtmosSystemMetadata system_metadata;
} AtmosReadObjectResponse;

AtmosReadObjectResponse*
AtmosReadObjectResponse_init(AtmosReadObjectResponse *self);

void
AtmosReadObjectResponse_destroy(AtmosReadObjectResponse *self);

#endif /* ATMOS_READ_H_ */
