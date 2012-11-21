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

/**
 * Convenience method to read user metadata from the response.
 * @param self the AtmosReadObjectResponse to search
 * @param name the name of the metadata item
 * @param listable set to 1 to search listable metadata, 0 to search regular.
 * @return the value of the metadata, or NULL if the metadata was not found.
 */
const char *
AtmosReadObjectResponse_get_metadata_value(AtmosReadObjectResponse *self,
        const char *name, int listable);

/**
 * Convenience method to read the ACL permission granted to a principal.
 * @param self the AtmosReadObjectResponse to search
 * @param principal the principal name to look for
 * @param principal_type the type of principal (ATMOS_USER or ATMOS_GROUP)
 * @return the permissions granted to the principal.  If the principal is not
 * found in the ACL, the permission NONE is returned.
 */
enum atmos_acl_permission
AtmosReadObjectResponse_get_acl_permission(AtmosReadObjectResponse *self,
        const char *principal, enum atmos_acl_principal_type principal_type);

#endif /* ATMOS_READ_H_ */
