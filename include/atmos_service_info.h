/*
 * atmos_service_info.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_SERVICE_INFO_H_
#define ATMOS_SERVICE_INFO_H_

#include "atmos.h"


typedef struct {
    AtmosResponse parent;
    char version[64];
    int utf8_metadata_supported;
} AtmosServiceInfoResponse;

AtmosServiceInfoResponse*
AtmosServiceInfoResponse_init(AtmosServiceInfoResponse *self);

void
AtmosServiceInfoResponse_destroy(AtmosServiceInfoResponse *self);

#endif /* ATMOS_SERVICE_INFO_H_ */
