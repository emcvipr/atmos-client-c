/*
 * atmos.h
 *
 *  Created on: Oct 19, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_H_
#define ATMOS_H_

#ifdef __APPLE__
/* Apple has deprecated openssl in favor of Common Crypto.  Ignore for now. */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "rest_client.h"

#include "atmos_constants.h"
#include "atmos_common.h"
#include "atmos_create.h"
#include "atmos_service_info.h"
#include "atmos_read.h"
#include "atmos_client.h"

#endif /* ATMOS_H_ */
