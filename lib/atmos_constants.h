/*

 Copyright (c) 2012, EMC Corporation

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of the EMC Corporation nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ATMOS_CONSTANTS_H_
#define ATMOS_CONSTANTS_H_

#define ATMOS_HEADER_UID "x-emc-uid"
#define ATMOS_HEADER_SIGNATURE "x-emc-signature"
#define ATMOS_HEADER_META "x-emc-meta"
#define ATMOS_HEADER_LISTABLE_META "x-emc-listable-meta"
#define ATMOS_HEADER_SUPPORT_UTF8 "x-emc-support-utf8"
#define ATMOS_HEADER_UTF8 "x-emc-utf8"
#define ATMOS_HEADER_USER_ACL "x-emc-useracl"
#define ATMOS_HEADER_GROUP_ACL "x-emc-groupacl"
#define ATMOS_HEADER_TAGS "x-emc-tags"

#define ATMOS_UID_MAX 255
#define ATMOS_SECRET_MAX 64
#define ATMOS_ERROR_MAX 1024
#define ATMOS_PATH_MAX 1024
#define ATMOS_META_VALUE_MAX 1024
#define ATMOS_META_NAME_MAX 255
#define ATMOS_META_COUNT_MAX 127
#define ATMOS_SYSTEM_META_COUNT_MAX 20
#define ATMOS_ACL_COUNT_MAX 64
#define ATMOS_CHECKSUM_MAX 128

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
#define CLASS_ATMOS_READ_OBJECT_REQUEST "AtmosReadObjectRequest"
#define CLASS_ATMOS_READ_OBJECT_RESPONSE "AtmosReadObjectResponse"
#define CLASS_ATMOS_GET_USER_META_REQUEST "AtmosGetUserMetaRequest"
#define CLASS_ATMOS_GET_USER_META_RESPONSE "AtmosGetUserMetaResponse"

#define ATMOS_OID_LOCATION_PREFIX "/rest/objects/"
#define ATMOS_OID_LOCATION_PREFIX_SIZE 14

// System metadata names
#define ATMOS_SYSTEM_META_ATIME "atime"
#define ATMOS_SYSTEM_META_CTIME "ctime"
#define ATMOS_SYSTEM_META_GID "gid"
#define ATMOS_SYSTEM_META_ITIME "itime"
#define ATMOS_SYSTEM_META_MTIME "mtime"
#define ATMOS_SYSTEM_META_NLINK "nlink"
#define ATMOS_SYSTEM_META_OBJECTID "objectid"
#define ATMOS_SYSTEM_META_OBJNAME "objname"
#define ATMOS_SYSTEM_META_POLICYNAME "policyname"
#define ATMOS_SYSTEM_META_SIZE "size"
#define ATMOS_SYSTEM_META_TYPE "type"
#define ATMOS_SYSTEM_META_UID "uid"
#define ATMOS_SYSTEM_META_WSCHECKSUM "x-emc-wschecksum"

// Values for ATMOS_SYSTEM_META_TYPE
#define ATMOS_TYPE_REGULAR "regular"
#define ATMOS_TYPE_DIRECTORY "directory"

#endif /* ATMOS_CONSTANTS_H_ */
