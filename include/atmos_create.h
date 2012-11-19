/*
 * create.h
 *
 *  Created on: Nov 10, 2012
 *      Author: cwikj
 */

#ifndef CREATE_H_
#define CREATE_H_

typedef struct {
    RestRequest parent;
    char path[ATMOS_PATH_MAX];
    AtmosMetadata meta[ATMOS_META_COUNT_MAX];
    int meta_count;
    AtmosMetadata listable_meta[ATMOS_META_COUNT_MAX];
    int listable_meta_count;
    AtmosAclEntry acl[ATMOS_ACL_COUNT_MAX];
    int acl_count;
} AtmosCreateObjectRequest;

typedef struct {
    AtmosResponse parent;
    char object_id[ATMOS_OID_LENGTH];
} AtmosCreateObjectResponse;

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init(AtmosCreateObjectRequest *self);

AtmosCreateObjectRequest*
AtmosCreateObjectRequest_init_ns(AtmosCreateObjectRequest *self,
        const char *path);

void
AtmosCreateObjectRequest_destroy(AtmosCreateObjectRequest *self);

void
AtmosCreateObjectRequest_add_metadata(AtmosCreateObjectRequest *self,
        const char *name, const char *value,
        int listable);

void
AtmosCreateObjectRequest_add_acl(AtmosCreateObjectRequest *self,
        const char *principal,
        enum atmos_acl_principal_type type,
        enum atmos_acl_permission permission);

AtmosCreateObjectResponse*
AtmosCreateObjectResponse_init(AtmosCreateObjectResponse *self);

void
AtmosCreateObjectResponse_destroy(AtmosCreateObjectResponse *self);


#endif /* CREATE_H_ */
