#ifndef _ATMOS_UTIL_H_
#define _ATMOS_UTIL_H_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "atmos.h"
#include "rest_client.h" // For decl of http_method enum

void AtmosUtil_strlcpy(char *dest, const char *src, size_t destsz);

void AtmosUtil_get_date(char *formated);
void AtmosUtil_lowercase(char *s);
void AtmosUtil_lowercaseheader(char *s);
int build_hash_string(char *hash_string, enum http_method method,
        const char *content_type, const char *range, const char *date,
        const char *uri, char **emc_sorted_headers, const int header_count);

char *AtmosUtil_base64decode(const char *base64encoded, size_t length, size_t *decoded_len);
char *AtmosUtil_base64encode(const char *normal, size_t length);
char *AtmosUtil_HMACSHA1(const char *hash_string, const char *key,
        size_t key_len);
int AtmosUtil_cstring_cmp(const void *a, const void *b);
char *AtmosUtil_cstring_append(char *buf, size_t *bufsz, const char *str);
char *
AtmosUtil_cstring_append_utf8(char *buf, size_t *bufsz,
        const char *str, CURL *curl);

void AtmosUtil_normalize_whitespace(char *header);
void AtmosUtil_set_metadata_header(AtmosMetadata *meta, int meta_count,
        int listable, int utf8, RestRequest *request);
void AtmosUtil_set_acl_header(AtmosAclEntry *acl, int acl_count,
        RestRequest *request);
int AtmosUtil_meta_char_check(const char *str);

void AtmosUtil_parse_meta(xmlNode *metadata, char *name, char *value,
        int *listable);


xmlXPathObjectPtr AtmosUtil_select_nodes(xmlDocPtr doc, xmlChar *selector, int use_cos_ns);

int AtmosUtil_count_nodes(xmlDocPtr doc, xmlChar *selector, int use_cos_ns);

xmlChar *AtmosUtil_select_single_node_value(xmlDocPtr doc, xmlChar *selector,
        int use_cos_ns);

void
AtmosUtil_parse_user_meta_headers(RestResponse *response,
        AtmosMetadata *meta, int *meta_count,
        AtmosMetadata *listable_meta, int *listable_meta_count);

void
AtmosUtil_set_system_meta_entry(AtmosSystemMetadata *system_meta,
        const char *entry_name, const char *entry_value, int utf8, CURL *curl);

void
AtmosUtil_parse_system_meta_header(RestResponse *response,
        AtmosSystemMetadata *system_meta);

void
AtmosUtil_parse_acl_headers(RestResponse *response,
        AtmosAclEntry *acl, int *acl_count);

int
AtmosUtil_is_system_meta_name(const char *name);

time_t
AtmosUtil_parse_xml_datetime(const char *value);

const char *
AtmosUtil_get_metadata_value(const char *name, AtmosMetadata *meta,
        int meta_count);

enum atmos_acl_permission
AtmosUtil_get_acl_permission(AtmosAclEntry *acl, int acl_count,
        const char *principal, enum atmos_acl_principal_type principal_type);

void
AtmosUtil_set_tags_header(RestRequest *request,
        char tags[][ATMOS_META_NAME_MAX], int tag_count, int utf8);

void
AtmosUtil_set_tags_header2(RestRequest *request,
        const char **tags, int tag_count, int utf8);

// Debugging
void
AtmosUtil_log(const char *level, const char *file, const int line, const char *fmt, ...);
#define ATMOS_DEBUG(fmt, ...) do{ AtmosUtil_log("DEBUG", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)
#define ATMOS_WARN(fmt, ...) do{ AtmosUtil_log("WARN", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)
#define ATMOS_ERROR(fmt, ...) do{ AtmosUtil_log("ERROR", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)

#if WIN32
#define snprintf sprintf_s
#define strtok_r strtok_s
#endif
#endif
