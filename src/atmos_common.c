/*
 * atmos_common.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */
#include "atmos.h"

const char const *ATMOS_SYSTEM_META_NAMES[] = {ATMOS_SYSTEM_META_ATIME,
ATMOS_SYSTEM_META_CTIME, ATMOS_SYSTEM_META_GID, ATMOS_SYSTEM_META_ITIME,
ATMOS_SYSTEM_META_MTIME, ATMOS_SYSTEM_META_NLINK, ATMOS_SYSTEM_META_OBJECTID,
ATMOS_SYSTEM_META_OBJNAME, ATMOS_SYSTEM_META_POLICYNAME, ATMOS_SYSTEM_META_SIZE,
ATMOS_SYSTEM_META_TYPE, ATMOS_SYSTEM_META_UID, ATMOS_SYSTEM_META_WSCHECKSUM};
const int ATMOS_SYSTEM_META_NAME_COUNT = 13;

AtmosResponse*
AtmosResponse_init(AtmosResponse *self) {
    RestResponse_init((RestResponse*)self);
    self->atmos_error = 0;
    self->atmos_error_message[0] = 0;

    return self;
}

void
AtmosResponse_destroy(AtmosResponse *self) {
    self->atmos_error = 0;
    self->atmos_error_message[0] = 0;
    RestResponse_destroy((RestResponse*)self);
}


