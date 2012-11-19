/*
 * atmos_common.c
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */
#include "atmos.h"

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


