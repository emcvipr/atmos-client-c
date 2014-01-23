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
#define ATMOS_HEADER_PATH "x-emc-path"
#define ATMOS_HEADER_FORCE "x-emc-force"
#define ATMOS_HEADER_TOKEN "x-emc-token"
#define ATMOS_HEADER_INCLUDE_META "x-emc-include-meta"
#define ATMOS_HEADER_USER_TAGS "x-emc-user-tags"
#define ATMOS_HEADER_SYSTEM_TAGS "x-emc-system-tags"
#define ATMOS_HEADER_LIMIT "x-emc-limit"
#define ATMOS_HEADER_LISTABLE_TAGS "x-emc-listable-tags"
#define ATMOS_HEADER_VERSION_OID "x-emc-version-oid"
#define ATMOS_HEADER_OBJECTID "x-emc-objectid"
#define ATMOS_HEADER_POOL "x-emc-pool"
#define ATMOS_HEADER_WSCHECKSUM "x-emc-wschecksum"
#define ATMOS_HEADER_GENERATE_CHECKSUM "x-emc-generate-checksum"
#define ATMOS_HEADER_CONTENT_CHECKSUM "x-emc-content-checksum"

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
#define ATMOS_TOKEN_MAX 1024

// e.g. 4d773b6ca10574f404d773bd3bedfc04d776693243b8
// ViPR extends this to 64
#define ATMOS_OID_LENGTH 65 // 64+1

#define ATMOS_SIMPLE_HEADER_MAX 255
#define ATMOS_CHECKSUM_LENGTH_MAX 129

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

#define ATMOS_CHECKSUM_ALG_SHA0 "sha0"
/**
 * Atmos 2.1+ only
 */
#define ATMOS_CHECKSUM_ALG_SHA1 "sha1"
/**
 * Atmos 2.1+ only
 */
#define ATMOS_CHECKSUM_ALG_MD5 "md5"

#define CLASS_ATMOS_CLIENT "AtmosClient"
#define CLASS_ATMOS_CREATE_OBJECT_REQUEST "AtmosCreateObjectRequest"
#define CLASS_ATMOS_CREATE_OBJECT_RESPONSE "AtmosCreateObjectResponse"
#define CLASS_ATMOS_READ_OBJECT_REQUEST "AtmosReadObjectRequest"
#define CLASS_ATMOS_READ_OBJECT_RESPONSE "AtmosReadObjectResponse"
#define CLASS_ATMOS_GET_USER_META_REQUEST "AtmosGetUserMetaRequest"
#define CLASS_ATMOS_GET_USER_META_RESPONSE "AtmosGetUserMetaResponse"
#define CLASS_ATMOS_SET_USER_META_REQUEST "AtmosSetUserMetaRequest"
#define CLASS_ATMOS_GET_SYSTEM_META_REQUEST "AtmosGetSystemMetaRequest"
#define CLASS_ATMOS_GET_SYSTEM_META_RESPONSE "AtmosGetSystemMetaResponse"
#define CLASS_ATMOS_GET_ACL_RESPONSE "AtmosGetAclResponse"
#define CLASS_ATMOS_UPDATE_OBJECT_REQUEST "AtmosUpdateObjectRequest"
#define CLASS_ATMOS_GET_OBJECT_INFO_RESPONSE "AtmosGetObjectInfoResponse"
#define CLASS_ATMOS_LIST_DIRECTORY_RESPONSE "AtmosListDirectoryResponse"
#define CLASS_ATMOS_DIRECTORY_ENTRY "AtmosDirectoryEntry"
#define CLASS_ATMOS_LIST_DIRECTORY_REQUEST "AtmosListDirectoryRequest"
#define CLASS_ATMOS_GET_LISTABLE_TAGS_REQUEST "AtmosGetListableTagsRequest"
#define CLASS_ATMOS_GET_LISTABLE_TAGS_RESPONSE "AtmosGetListableTagsResponse"
#define CLASS_ATMOS_PAGINATED_REQUEST "AtmosPaginatedRequest"
#define CLASS_ATMOS_OBJECT_METADATA "AtmosObjectMetadata"
#define CLASS_ATMOS_OBJECT_LISTING "AtmosObjectListing"
#define CLASS_ATMOS_LIST_OBJECTS_RESPONSE "AtmosListObjectsResponse"
#define CLASS_ATMOS_CREATE_VERSION_REQUEST "AtmosCreateVersionRequest"
#define CLASS_ATMOS_CREATE_VERSION_RESPONSE "AtmosCreateVersionResponse"
#define CLASS_ATMOS_LIST_VERSIONS_REQUEST "AtmosListVersionsRequest"
#define CLASS_ATMOS_LIST_VERSIONS_RESPONSE "AtmosListVersionsResponse"
#define CLASS_ATMOS_CREATE_ACCESS_TOKEN_REQUEST "AtmosCreateAccessTokenRequest"
#define CLASS_ATMOS_CREATE_ACCESS_TOKEN_RESPONSE "AtmosCreateAccessTokenResponse"
#define CLASS_ATMOS_LIST_ACCESS_TOKENS_REQUEST "AtmosListAccessTokensRequest"
#define CLASS_ATMOS_LIST_ACCESS_TOKENS_RESPONSE "AtmosListAccessTokensResponse"
#define CLASS_ATMOS_GET_ACCESS_TOKEN_INFO_RESPONSE "AtmosGetAccessTokenInfoResponse"
#define CLASS_ATMOS_WRITE_OBJECT_REQUEST "AtmosWriteObjectRequest"
#define CLASS_ATMOS_CHECKSUM "AtmosChecksum"

#define ATMOS_OID_LOCATION_PREFIX "/rest/objects/"
#define ATMOS_OID_LOCATION_PREFIX_SIZE 14

#define ATMOS_ACCESS_TOKEN_LOCATION_PREFIX "/rest/accesstokens/"

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

// Used for parsing directory listings
#define DIR_NODE_OBJECT_ID "ObjectID"
#define DIR_NODE_FILE_TYPE "FileType"
#define DIR_NODE_FILE_NAME "Filename"
#define DIR_NODE_SYSTEM_METADATA_LIST "SystemMetadataList"
#define DIR_NODE_METADATA "Metadata"
#define DIR_NODE_USER_METADATA_LIST "UserMetadataList"
#define DIR_NODE_NAME "Name"
#define DIR_NODE_VALUE "Value"
#define DIR_NODE_LISTABLE "Listable"

// Used for parsing version listings
#define VER_NODE_OID "OID"
#define VER_NODE_VER_NUM "VerNum"
#define VER_NODE_ITIME "itime"

// Internal API errors set as Atmos error codes
#define ATMOS_ERROR_API_FTELL_NOT_SUPPORTED 601
#define ATMOS_ERROR_API_FTELL_NOT_SUPPORTED_STR "A file handle was used that does not support ftell() but it is required for wschecksum."
#define ATMOS_ERROR_API_FSEEK_NOT_SUPPORTED 602
#define ATMOS_ERROR_API_FSEEK_NOT_SUPPORTED_STR "A file handle was used that does not support fseek() but it is required for wschecksum."
#define ATMOS_ERROR_API_CHECKSUM_FAILED 603
#define ATMOS_ERROR_API_CHECKSUM_FAILED_STR "Checksum calculation failed"

#endif /* ATMOS_CONSTANTS_H_ */
