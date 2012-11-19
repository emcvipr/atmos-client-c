/*
 * atmos_constants.h
 *
 *  Created on: Nov 10, 2012
 *      Author: cwikj
 */

#ifndef ATMOS_CONSTANTS_H_
#define ATMOS_CONSTANTS_H_

#define ATMOS_HEADER_UID "x-emc-uid"
#define ATMOS_HEADER_SIGNATURE "x-emc-signature"
#define ATMOS_HEADER_META "x-emc-meta"
#define ATMOS_HEADER_LISTABLE_META "x-emc-listable-meta"
#define ATMOS_HEADER_SUPPORT_UTF8 "x-emc-support-utf8"
#define ATMOS_HEADER_USER_ACL "x-emc-useracl"
#define ATMOS_HEADER_GROUP_ACL "x-emc-groupacl"

#define ATMOS_UID_MAX 255
#define ATMOS_SECRET_MAX 64
#define ATMOS_ERROR_MAX 1024
#define ATMOS_PATH_MAX 1024
#define ATMOS_META_VALUE_MAX 1024
#define ATMOS_META_NAME_MAX 255
#define ATMOS_META_COUNT_MAX 127
#define ATMOS_ACL_COUNT_MAX 64

// e.g. 4d773b6ca10574f404d773bd3bedfc04d776693243b8
#define ATMOS_OID_LENGTH 45 // 44+1

#define ATMOS_SIMPLE_HEADER_MAX 255

#define ATMOS_ERROR_ROOT_NODE "Error"
#define ATMOS_ERROR_CODE_NODE "Code"
#define ATMOS_ERROR_MESSAGE_NODE "Message"
#define ATMOS_ERROR_CODE_XPATH "/Error/Code"
#define ATMOS_ERROR_MESSAGE_XPATH "/Error/Message"

#define ATMOS_ACL_PERMISSION_NONE "NONE"
#define ATMOS_ACL_PERMISSION_READ "READ"
#define ATMOS_ACL_PERMISSION_READWRITE "WRITE"
#define ATMOS_ACL_PERMISSION_FULL "FULL_CONTROL"
#define ATMOS_ACL_GROUP_OTHER "other"


#define CLASS_ATMOS_CLIENT "AtmosClient"
#define CLASS_ATMOS_CREATE_OBJECT_REQUEST "AtmosCreateObjectRequest"
#define CLASS_ATMOS_CREATE_OBJECT_RESPONSE "AtmosCreateObjectResponse"

#define ATMOS_OID_LOCATION_PREFIX "/rest/objects/"
#define ATMOS_OID_LOCATION_PREFIX_SIZE 14

#endif /* ATMOS_CONSTANTS_H_ */
