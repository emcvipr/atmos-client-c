/*
 * atmos_service_info.h
 *
 *  Created on: Nov 14, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_SERVICE_INFO_H_
#define ATMOS_SERVICE_INFO_H_

/**
 * @file atmos_service_info.h
 * @brief This file contains the objects for the get servince information
 * operation.
 */

#include "atmos.h"

/**
 * @defgroup AtmosServiceInfoResponse AtmosServiceInfoResponse
 * @brief This module contains the AtmosServiceInfoResponse class that is
 * used to return information about the Atmos version the client is connected
 * to.
 * @{
 */
/** Contains the response from a get service information request */
typedef struct {
    /** Parent class */
    AtmosResponse parent;
    /** Atmos version string, e.g. "2.1.0" */
    char version[64];
    /**
     * If nonzero, this version of Atmos supports UTF-8 encoded metadata and
     * other headers. (Generally, =1.4.2, >=2.0.1).
     */
    int utf8_metadata_supported;
} AtmosServiceInfoResponse;

/**
 * Initializes a new AtmosServiceInfoResponse object.
 * @param self the AtmosServiceInfoResponse object to initialize.
 * @return the AtmosServiceInfoResponse object (same as 'self')
 */
AtmosServiceInfoResponse*
AtmosServiceInfoResponse_init(AtmosServiceInfoResponse *self);

/**
 * Destroys an AtmosServiceInfoResponse object.
 */
void
AtmosServiceInfoResponse_destroy(AtmosServiceInfoResponse *self);

/**
 * @}
 */

#endif /* ATMOS_SERVICE_INFO_H_ */
