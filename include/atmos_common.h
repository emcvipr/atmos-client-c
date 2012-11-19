/*
 * atmos_common.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_COMMON_H_
#define ATMOS_COMMON_H_

typedef struct {
    RestResponse parent;
    int atmos_error;
    char atmos_error_message[ATMOS_ERROR_MAX];
} AtmosResponse;

enum atmos_acl_principal_type {
    ATMOS_USER, ATMOS_GROUP
};

enum atmos_acl_permission {
    NONE, READ, READ_WRITE, FULL
};

typedef struct {
    char principal[ATMOS_UID_MAX];
    enum atmos_acl_principal_type type;
    enum atmos_acl_permission permission;
} AtmosAclEntry;


typedef struct {
    char name[ATMOS_META_NAME_MAX];
    char value[ATMOS_META_VALUE_MAX];
} AtmosMetadata;

AtmosResponse*
AtmosResponse_init(AtmosResponse *self);

void
AtmosResponse_destroy(AtmosResponse *self);


#endif /* ATMOS_COMMON_H_ */
