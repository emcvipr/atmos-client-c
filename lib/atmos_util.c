#include "config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <curl/curl.h>

#include "atmos_util.h"
#include "atmos_private.h"

void AtmosUtil_strlcpy(char *dest, const char *src, size_t destsz) {
    size_t i;
    for(i=0; i<destsz-1 && *src; i++, src++, dest++) {
        *dest = *src;
    }
    *dest=0;
}

int AtmosUtil_cstring_cmp(const void *a, const void *b) {
    return strcmp(*(char * const *) a, *(char * const *) b);
}

char *AtmosUtil_cstring_append(char *buf, size_t *bufsz, const char *str) {
    size_t strsz = strlen(str);
    size_t bufpos = 0;
    char *buf2;
    
    if (buf != NULL) {
        bufpos = strlen(buf);
    }

    if (buf == NULL || bufpos + strsz + 1 > *bufsz) {
        size_t newsize = *bufsz + strsz + 256;
        buf2 = realloc(buf, newsize);
        if(!buf2) {
            ATMOS_ERROR("Could not reallocate %ld bytes", newsize);
            return buf;
        } else {
            buf = buf2;
        }
        *bufsz = newsize;
    }

    strcpy(buf + bufpos, str);
    buf[bufpos + strsz] = 0;

    return buf;
}

char *AtmosUtil_cstring_append_utf8(char *buf, size_t *bufsz,
        const char *str, CURL *curl) {
    size_t strsz = strlen(str);
    size_t bufpos = 0;
    char *buf2;
    char *encoded;

    encoded = curl_easy_escape(curl, str, (int)strsz);
    if(!encoded) {
        ATMOS_ERROR("Failed to encode value: %s\n", str);
        return buf;
    }
    strsz = strlen(encoded);

    if (buf != NULL) {
        bufpos = strlen(buf);
    }

    if (buf == NULL || bufpos + strsz + 1 > *bufsz) {
        size_t newsize = *bufsz + strsz + 256;
        buf2 = realloc(buf, newsize);
        if(!buf2) {
            ATMOS_ERROR("Could not reallocate %ld bytes", newsize);
            curl_free(encoded);
            return buf;
        } else {
            buf = buf2;
        }
        *bufsz = newsize;
    }

    strcpy(buf + bufpos, encoded);
    buf[bufpos + strsz] = 0;
    curl_free(encoded);

    return buf;
}

void AtmosUtil_get_date(char *formated_time) {
    //strftime adds a leading 0 to the day...
    time_t t = time(NULL);
    struct tm *a = gmtime(&t);

    strftime(formated_time, 256, "%a, %d %b %Y %H:%M:%S GMT", a);

}

void AtmosUtil_normalize_whitespace(char *header) {
    size_t sz = strlen(header);
    unsigned int i;
    unsigned int j;
    int sepfound = 0;

    for (i = 0, j = 0; i < sz; i++) {
        if (header[i] == ' ') {
            if (i + 1 == sz || header[i + 1] == ' ' || header[i + 1] == ','
                    || (!sepfound && header[i + 1] == ':')) {
                // Trim whitespace at the end of a string, surrounding the colon,
                // collapse it inside the value, and at the end of a metadata
                // element before a comma.
            } else {
                header[j++] = header[i];
            }
        } else if (header[i] == ':' && !sepfound) {
            sepfound = 1;

            header[j++] = header[i];

            // Trim any whitespace following the first colon.
            while (i + 1 < sz && header[i + 1] == ' ') {
                i++;
            }
        } else {
            header[j++] = header[i];
        }
    }
    header[j] = 0;
}

void AtmosUtil_lowercaseheader(char *s) {
    int i = 0;
    for (; s[i] != ':'; i++)
        s[i] = tolower(s[i]);
}

void AtmosUtil_lowercase(char *s) {
    int i = 0;
    for (; s[i] != '\0'; i++)
        s[i] = tolower(s[i]);
}

char *AtmosUtil_base64decode(const char *base64encoded, size_t length, size_t *decoded_len) {
    BIO *b64, *bmem;
    char *buffer = (char *) malloc(length); // This really should be 3/4 of length

    memset(buffer, 0, length);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf((void*) base64encoded, (int) length);
    bmem = BIO_push(b64, bmem);
    *decoded_len = BIO_read(bmem, buffer, (int) length);
    BIO_free_all(bmem);

    b64 = NULL;

    return buffer;
}

char *AtmosUtil_base64encode(const char *normal, size_t length) {
    long memlength = 0;
    char *membuff;

    BIO *bmem, *b64;
    char *buff = NULL;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, normal, (int) length);
    if (BIO_flush(b64) != 1) {
        return NULL;
    }
    memlength = BIO_get_mem_data(bmem, &membuff);

    buff = (char *) malloc(memlength);

    memcpy(buff, membuff, memlength);
    buff[memlength-1] = 0;

    BIO_free_all(b64);
    b64 = NULL;
    return buff;
}

char *AtmosUtil_HMACSHA1(const char *hash_string, const char *key,
        size_t key_len) {
    const EVP_MD *evp_md = EVP_sha1();
    unsigned int md_len;
    unsigned char md[EVP_MAX_MD_SIZE];
    size_t decoded_len;
    char *newkey = AtmosUtil_base64decode(key, key_len, &decoded_len);

    HMAC(evp_md, newkey, (int)decoded_len, (const unsigned char*) hash_string,
            strlen(hash_string), md, &md_len);
    free(newkey);
    return AtmosUtil_base64encode((char*) md, md_len);

}

xmlXPathObjectPtr AtmosUtil_select_nodes(xmlDocPtr doc, xmlChar *selector, int use_cos_ns) {
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        ATMOS_ERROR("Error: unable to create new XPath context: %s\n",
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        return NULL;
    }

    if (use_cos_ns) {
        if (xmlXPathRegisterNs(xpathCtx, BAD_CAST "cos",
                BAD_CAST "http://www.emc.com/cos/")) {
            ATMOS_ERROR("Error: unable to register cos namespace: %s\n",
                    xmlGetLastError()?xmlGetLastError()->message:"(null)");
            xmlXPathFreeContext(xpathCtx);
            return NULL;

        }
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(selector, xpathCtx);
    if (xpathObj == NULL) {
        ATMOS_ERROR("Error: unable to evaluate xpath expression \"%s\": %s\n",
                selector,
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    xpathNodeSet = xpathObj->nodesetval;
    if (!xpathNodeSet || xpathNodeSet->nodeNr == 0) {
        //ATMOS_ERROR("Error: No nodes returned from \"%s\"\n", selector);
        xmlXPathFreeContext(xpathCtx);
        xmlXPathFreeObject(xpathObj);
        return NULL;
    }

    /* Cleanup */
    xmlXPathFreeContext(xpathCtx);

    return xpathObj;
}

int AtmosUtil_count_nodes(xmlDocPtr doc, xmlChar *selector, int use_cos_ns) {
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;
    int count;

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        ATMOS_ERROR("Error: unable to create new XPath context: %s\n",
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        return 0;
    }

    if (use_cos_ns) {
        if (xmlXPathRegisterNs(xpathCtx, BAD_CAST "cos",
                BAD_CAST "http://www.emc.com/cos/")) {
            ATMOS_ERROR("Error: unable to register cos namespace: %s\n",
                    xmlGetLastError()?xmlGetLastError()->message:"(null)");
            xmlXPathFreeContext(xpathCtx);
            return 0;

        }
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(selector, xpathCtx);
    if (xpathObj == NULL) {
        ATMOS_ERROR("Error: unable to evaluate xpath expression \"%s\": %s\n",
                selector,
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        xmlXPathFreeContext(xpathCtx);
        return 0;
    }
    xpathNodeSet = xpathObj->nodesetval;
    if (!xpathNodeSet || xpathNodeSet->nodeNr == 0) {
        xmlXPathFreeContext(xpathCtx);
        xmlXPathFreeObject(xpathObj);
        return 0;
    }
    count = xpathNodeSet->nodeNr;

    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);

    return count;
}


xmlChar *AtmosUtil_select_single_node_value(xmlDocPtr doc, xmlChar *selector,
        int use_cos_ns) {
    xmlNodePtr xmlNode;
    xmlChar *value;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;

    xpathObj = AtmosUtil_select_nodes(doc, selector, use_cos_ns);
    if(!xpathObj) {
        return NULL;
    }
    xpathNodeSet = xpathObj->nodesetval;

    if (xpathNodeSet->nodeNr > 1) {
        fprintf(stderr, "Error: Multiple (%d) nodes returned from \"%s\"\n",
                xpathNodeSet->nodeNr, selector);
        xmlXPathFreeObject(xpathObj);
        return NULL;
    }
    xmlNode = xpathNodeSet->nodeTab[0];
    value = xmlNodeGetContent(xmlNode);

    xmlXPathFreeObject(xpathObj);
    return value;
}

void AtmosUtil_set_metadata_header(AtmosMetadata *meta, int meta_count,
        int listable, int utf8, RestRequest *request) {
    char *meta_header = NULL;
    size_t meta_header_sz = 0;
    int i;
    CURL *curl = NULL;

    if (meta_count < 1) {
        return;
    }

    if (!listable) {
        meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                ATMOS_HEADER_META);
    } else {
        meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                ATMOS_HEADER_LISTABLE_META);
    }
    meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz, ": ");

    for (i = 0; i < meta_count; i++) {
        if (!utf8) {
            // Sanity check name and value.
            if (AtmosUtil_meta_char_check(meta[i].name)
                    || AtmosUtil_meta_char_check(meta[i].value)) {
                ATMOS_WARN("Invalid character in metadata %s:%s - "
                        "use utf8 mode\n", meta[i].name, meta[i].value);
                continue;
            }
        }

        if (i > 0) {
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, ",");
        }

        if (utf8) {
            char *enc;

            if (!curl) {
                // for some reason, we need a curl handle for curl_easy_escape
                curl = curl_easy_init();
            }
            // URLEncode the name and value.
            enc = curl_easy_escape(curl, meta[i].name, 0);
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, enc);
            curl_free(enc);
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, "=");
            enc = curl_easy_escape(curl, meta[i].value, 0);
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, enc);
            curl_free(enc);
        } else {
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, meta[i].name);
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, "=");
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, meta[i].value);
        }
    }

    RestRequest_add_header(request, meta_header);

    free(meta_header);
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

const char *
AtmosUtil_get_permission_string(enum atmos_acl_permission perm) {
    switch (perm) {
    case ATMOS_PERM_FULL:
        return ATMOS_ACL_PERMISSION_FULL;
    case ATMOS_PERM_READ:
        return ATMOS_ACL_PERMISSION_READ;
    case ATMOS_PERM_READ_WRITE:
        return ATMOS_ACL_PERMISSION_READWRITE;
    case ATMOS_PERM_NONE:
        return ATMOS_ACL_PERMISSION_NONE;
    default:
        ATMOS_WARN("Invalid acl permission type %d\n", perm);
        return NULL;
    }
}

void AtmosUtil_set_acl_header(AtmosAclEntry *acl, int acl_count,
        RestRequest *request) {
    char *user_acl_header = NULL;
    size_t user_acl_header_sz = 0;
    char *group_acl_header = NULL;
    size_t group_acl_header_sz = 0;
    int i;
    int first_group = 1;
    int first_user = 1;

    if (acl_count < 1) {
        return;
    }

    for (i = 0; i < acl_count; i++) {
        if (acl[i].type == ATMOS_GROUP) {
            // Currently, the principal must be 'other'
            if (strcmp(ATMOS_ACL_GROUP_OTHER, acl[i].principal)) {
                ATMOS_WARN("Invalid ACL group '%s'\n", acl[i].principal);
                continue;
            }
            if (!first_group) {
                group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                        &group_acl_header_sz, ", ");
            } else {
                group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                        &group_acl_header_sz, ATMOS_HEADER_GROUP_ACL);
                group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                        &group_acl_header_sz, ": ");
                first_group = 0;
            }
            group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                    &group_acl_header_sz, acl[i].principal);
            group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                    &group_acl_header_sz, "=");
            group_acl_header = AtmosUtil_cstring_append(group_acl_header,
                    &group_acl_header_sz,
                    AtmosUtil_get_permission_string(acl[i].permission));
        } else if (acl[i].type == ATMOS_USER) {
            if (!first_user) {
                user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                        &user_acl_header_sz, ", ");
            } else {
                user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                        &user_acl_header_sz, ATMOS_HEADER_USER_ACL);
                user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                        &user_acl_header_sz, ": ");
                first_user = 0;
            }
            user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                    &user_acl_header_sz, acl[i].principal);
            user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                    &user_acl_header_sz, "=");
            user_acl_header = AtmosUtil_cstring_append(user_acl_header,
                    &user_acl_header_sz,
                    AtmosUtil_get_permission_string(acl[i].permission));
        } else {
            ATMOS_WARN("Invalid ACL entry type %d\n", acl[i].type);
            continue;
        }
    }

    if (user_acl_header_sz > 0) {
        RestRequest_add_header(request, user_acl_header);
        free(user_acl_header);
    }

    if (group_acl_header_sz > 0) {
        RestRequest_add_header(request, group_acl_header);
        free(group_acl_header);
    }

}

void AtmosUtil_log(const char *level, const char *file, const int line,
        const char *fmt, ...) {
    char msg[1024];
    char stime[255];
    time_t t;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 1024, fmt, args);
    va_end(args);

    // Get the current timestamp
    t = time(NULL);
    ctime_r(&t, stime);
    // ctime_r puts a newline at the end.
    stime[strlen(stime) - 1] = 0;

    fprintf(stderr, "%s - %s %s:%d: %s", stime, level, file, line, msg);
}

int AtmosUtil_meta_char_check(const char *str) {
    size_t len = strlen(str);
    int i;

    for (i = 0; i < len; i++) {
        switch (str[i]) {
        case ',':
        case '\n':
        case '=':
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], (int)str[i]);
            return 1;
        default:
            break;
        }

        if (str[i] < 32) {
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], (int)str[i]);
            return 1;
        }
        if (str[i] & 0x80) {
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], (int)str[i]);
            return 1;
        }
    }

    return 0;
}

int AtmosUtil_is_system_meta_name(const char *name) {
    int i;
    for (i = 0; i < ATMOS_SYSTEM_META_NAME_COUNT; i++) {
        if (!strcmp(ATMOS_SYSTEM_META_NAMES[i], name)) {
            return 1;
        }
    }
    return 0;
}

static void AtmosUtil_parse_meta_header_entry(const char *entry, char *name,
        char *value) {
    const char *delimiter;
    const char *valpos;
    // strip leading spaces
    while(*entry == ' ') {
        entry++;
    }

    // Split on the equal sign.
    delimiter = strstr(entry, "=");
    if (!delimiter) {
        ATMOS_WARN("Name-value delimter not found in metadata entry %s\n", value);
        // Just use it as the name.
        strcpy(name, entry);
    } else {
        size_t namesz = delimiter - entry;
        AtmosUtil_strlcpy(name, entry, namesz+1);
        valpos = entry+namesz+1;
        // Strip leading spaces
        while(*valpos == ' ') {
            valpos++;
        }
        strcpy(value, valpos);
    }
}

static void
AtmosUtil_strlcpy_utf8(char *dest, const char *src, size_t destsz,
        int utf8, CURL *curl) {
    char *decoded;

    if(utf8) {
        decoded = curl_easy_unescape(curl, src, 0, NULL);
        if(!decoded) {
            ATMOS_WARN("Could not unescape string %s\n", src);
            return;
        }
        AtmosUtil_strlcpy(dest, decoded, destsz);
        curl_free(decoded);
    } else {
        AtmosUtil_strlcpy(dest, src, destsz);
    }
}

static void AtmosUtil_parse_meta_header_string(const char *value,
        AtmosMetadata *meta, int *meta_count, int utf8) {
    char entry[ATMOS_META_NAME_MAX*3 + 1 + ATMOS_META_VALUE_MAX*3];
    char entry_name[ATMOS_META_NAME_MAX*3];
    char entry_value[ATMOS_META_VALUE_MAX*3];
    CURL *curl = NULL;

    if(utf8) {
        curl = curl_easy_init();
    }

    while (value) {
        const char *value_end = strstr(value, ",");
        entry_name[0] = 0;
        entry_value[0] = 0;

        if (!value_end) {
            // End of string reached.
            AtmosUtil_parse_meta_header_entry(value, entry_name, entry_value);
        } else {
            size_t valuesz = value_end - value;
            AtmosUtil_strlcpy(entry, value, valuesz+1);
            AtmosUtil_parse_meta_header_entry(entry, entry_name, entry_value);
        }

        if(AtmosUtil_is_system_meta_name(entry_name)) {
            // Skip
            if(value_end) {
                value = value_end+1;
            } else {
                value = NULL;
            }
            continue;
        }

        AtmosUtil_strlcpy_utf8(meta[*meta_count].name, entry_name,
                ATMOS_META_VALUE_MAX, utf8, curl);
        AtmosUtil_strlcpy_utf8(meta[*meta_count].value, entry_value,
                ATMOS_META_VALUE_MAX, utf8, curl);
        (*meta_count)++;

        if(value_end) {
            value = value_end+1;
        } else {
            value = NULL;
        }
    }

    if(curl) {
        curl_easy_cleanup(curl);
    }
}

void AtmosUtil_parse_user_meta_headers(RestResponse *response,
        AtmosMetadata *meta, int *meta_count, AtmosMetadata *listable_meta,
        int *listable_meta_count) {
    const char *value;
    int utf8 = 0;

    // Check for utf8
    value = RestResponse_get_header_value(response, ATMOS_HEADER_UTF8);
    if(value) {
        if(!strcmp("true", value)) {
            utf8 = 1;
        }
    }

    value = RestResponse_get_header_value(response, ATMOS_HEADER_META);
    if (value) {
        AtmosUtil_parse_meta_header_string(value, meta, meta_count, utf8);
    }

    value = RestResponse_get_header_value(response, ATMOS_HEADER_LISTABLE_META);
    if (value) {
        AtmosUtil_parse_meta_header_string(value, listable_meta,
                listable_meta_count, utf8);
    }
}


time_t
AtmosUtil_parse_xml_datetime(const char *value) {
    struct tm t;
    time_t tt;
    char buffer[ATMOS_SIMPLE_HEADER_MAX];

    memset(&t, 0, sizeof(struct tm));

    // strptime doesn't accept "Z" as GMT.
    AtmosUtil_strlcpy(buffer, value, ATMOS_SIMPLE_HEADER_MAX);
    buffer[strlen(buffer)-1] = 0;
    strcat(buffer, "GMT");

    if(!strptime(buffer, "%FT%T%Z", &t)) {
        ATMOS_WARN("Could not parse dateTime value %s\n", buffer);
        return 0;
    }

    tt = mktime(&t);
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
    /* BSD uses this member in mktime, so it's already TZ corrected to GMT */
#elif HAVE_STRUCT_TM___TM_GMTOFF
    /* Linux does not apply this member in mktime */
    /* Correct the time_t to be in GMT, not localtime. */
    tt -= timezone;
#else
#error Need either struct tm.tm_gmtoff or tm.__tm_gmtoff
#endif
    return tt;
}

void AtmosUtil_set_system_meta_entry(AtmosSystemMetadata *system_meta,
        const char *entry_name, const char *entry_value, int utf8, CURL *curl) {
    if (!strcmp(ATMOS_SYSTEM_META_ATIME, entry_name)) {
        system_meta->atime = AtmosUtil_parse_xml_datetime(entry_value);
    } else if (!strcmp(ATMOS_SYSTEM_META_CTIME, entry_name)) {
        system_meta->ctime = AtmosUtil_parse_xml_datetime(entry_value);
    } else if (!strcmp(ATMOS_SYSTEM_META_GID, entry_name)) {
        AtmosUtil_strlcpy(system_meta->gid, entry_value, ATMOS_UID_MAX);
    } else if (!strcmp(ATMOS_SYSTEM_META_ITIME, entry_name)) {
        system_meta->itime = AtmosUtil_parse_xml_datetime(entry_value);
    } else if (!strcmp(ATMOS_SYSTEM_META_MTIME, entry_name)) {
        system_meta->mtime = AtmosUtil_parse_xml_datetime(entry_value);
    } else if (!strcmp(ATMOS_SYSTEM_META_NLINK, entry_name)) {
        system_meta->nlink = (int)strtol(entry_value, NULL, 10);
    } else if (!strcmp(ATMOS_SYSTEM_META_OBJECTID, entry_name)) {
        AtmosUtil_strlcpy(system_meta->object_id, entry_value, ATMOS_OID_LENGTH);
    } else if (!strcmp(ATMOS_SYSTEM_META_OBJNAME, entry_name)) {
        AtmosUtil_strlcpy_utf8(system_meta->objname, entry_value,
                ATMOS_PATH_MAX, utf8, curl);
    } else if (!strcmp(ATMOS_SYSTEM_META_POLICYNAME, entry_name)) {
        AtmosUtil_strlcpy(system_meta->policyname, entry_value, ATMOS_UID_MAX);
    } else if (!strcmp(ATMOS_SYSTEM_META_SIZE, entry_name)) {
        system_meta->size = strtoll(entry_value, NULL, 10);
    } else if (!strcmp(ATMOS_SYSTEM_META_TYPE, entry_name)) {
        if(!strcmp(ATMOS_TYPE_DIRECTORY, entry_value)) {
            system_meta->type = ATMOS_TYPE_DIRECTORY;
        } else if(!strcmp(ATMOS_TYPE_REGULAR, entry_value)) {
            system_meta->type = ATMOS_TYPE_REGULAR;
        }
    } else if (!strcmp(ATMOS_SYSTEM_META_UID, entry_name)) {
        AtmosUtil_strlcpy(system_meta->uid, entry_value, ATMOS_UID_MAX);
    } else if (!strcmp(ATMOS_SYSTEM_META_WSCHECKSUM, entry_name)) {
        AtmosUtil_strlcpy(system_meta->wschecksum, entry_value, ATMOS_CHECKSUM_MAX);
    }

}

void AtmosUtil_parse_system_meta_header(RestResponse *response,
        AtmosSystemMetadata *system_meta) {
    const char *value;
    char entry[ATMOS_META_NAME_MAX*3 + 1 + ATMOS_META_VALUE_MAX*3];
    char entry_name[ATMOS_META_NAME_MAX*3];
    char entry_value[ATMOS_META_VALUE_MAX*3];\
    int utf8 = 0;
    CURL *curl = NULL;

    // Check for utf8
    value = RestResponse_get_header_value(response, ATMOS_HEADER_UTF8);
    if(value) {
        if(!strcmp("true", value)) {
            utf8 = 1;
        }
    }


    if(utf8) {
        curl = curl_easy_init();
    }

    value = RestResponse_get_header_value(response, ATMOS_HEADER_META);
    if (!value) {
        if(curl) {
            curl_easy_cleanup(curl);
        }
        return;
    }

    // Iterate through the items and look for the system names.
    while (value) {
        entry_name[0] = 0;
        entry_value[0] = 0;
        const char *value_end = strstr(value, ",");
        if (!value_end) {
            // End of string reached.
            AtmosUtil_parse_meta_header_entry(value, entry_name, entry_value);
        } else {
            size_t valuesz = value_end - value;
            AtmosUtil_strlcpy(entry, value, valuesz+1);
            AtmosUtil_parse_meta_header_entry(entry, entry_name, entry_value);
        }
        if(value_end) {
            value = value_end+1;
        } else {
            value = NULL;
        }
        AtmosUtil_set_system_meta_entry(system_meta, entry_name, entry_value,
                utf8, curl);
    }

    if(curl) {
        curl_easy_cleanup(curl);
    }

}

static void
AtmosUtil_parse_acl(const char *value, AtmosAclEntry *acl, int *acl_count,
        enum atmos_acl_principal_type type) {
    const char *value_end;
    char entry[ATMOS_UID_MAX*2+1];
    char entry_name[ATMOS_UID_MAX];
    char entry_value[ATMOS_UID_MAX];

    // Iterate through the items and look for the system names.
    while (value) {
        entry_name[0] = 0;
        entry_value[0] = 0;
        value_end = strstr(value, ",");
        if (!value_end) {
            // End of string reached.
            AtmosUtil_parse_meta_header_entry(value, entry_name, entry_value);
        } else {
            size_t valuesz = value_end - value;
            AtmosUtil_strlcpy(entry, value, valuesz+1);
            AtmosUtil_parse_meta_header_entry(entry, entry_name, entry_value);
        }
        if(value_end) {
            value = value_end+1;
        } else {
            value = NULL;
        }

        acl[*acl_count].type = type;
        AtmosUtil_strlcpy(acl[*acl_count].principal, entry_name, ATMOS_UID_MAX);
        if(!strcmp(ATMOS_ACL_PERMISSION_FULL, entry_value)) {
            acl[*acl_count].permission = ATMOS_PERM_FULL;
        } else if(!strcmp(ATMOS_ACL_PERMISSION_NONE, entry_value)) {
            acl[*acl_count].permission = ATMOS_PERM_NONE;
        } else if(!strcmp(ATMOS_ACL_PERMISSION_READ, entry_value)) {
            acl[*acl_count].permission = ATMOS_PERM_READ;
        } else if(!strcmp(ATMOS_ACL_PERMISSION_READWRITE, entry_value)) {
            acl[*acl_count].permission = ATMOS_PERM_READ_WRITE;
        } else {
            ATMOS_WARN("Invalid ACL permission '%s'\n", entry_value);
        }
        (*acl_count)++;
    }

}

void
AtmosUtil_parse_acl_headers(RestResponse *response,
        AtmosAclEntry *acl, int *acl_count) {
    const char *value;

    value = RestResponse_get_header_value(response, ATMOS_HEADER_USER_ACL);
    if(value) {
        AtmosUtil_parse_acl(value, acl, acl_count, ATMOS_USER);
    }
    value = RestResponse_get_header_value(response, ATMOS_HEADER_GROUP_ACL);
    if(value) {
        AtmosUtil_parse_acl(value, acl, acl_count, ATMOS_GROUP);
    }
}


const char *
AtmosUtil_get_metadata_value(const char *name, AtmosMetadata *meta, int meta_count) {
    int i;
    for(i=0; i<meta_count; i++) {
        if(!strcmp(name, meta[i].name)) {
            return meta[i].value;
        }
    }
    return NULL;
}

enum atmos_acl_permission
AtmosUtil_get_acl_permission(AtmosAclEntry *acl, int acl_count,
        const char *principal, enum atmos_acl_principal_type principal_type) {
    int i;

    for(i=0; i<acl_count; i++) {
        if(!strcmp(principal, acl[i].principal)
                && acl[i].type == principal_type) {
            return acl[i].permission;
        }
    }
    return ATMOS_PERM_NONE;
}

void
AtmosUtil_set_tags_header(RestRequest *request,
        char tags[][ATMOS_META_NAME_MAX], int tag_count, int utf8) {
    char *header = NULL;
    size_t header_size = 0;
    int i;
    CURL *curl = NULL;
    char *encoded_tag;

    if(tags == NULL || tag_count < 1) {
        return;
    }

    header = AtmosUtil_cstring_append(header, &header_size,
            ATMOS_HEADER_TAGS ": ");

    for(i=0; i<tag_count; i++) {
        if(i>0) {
            header = AtmosUtil_cstring_append(header, &header_size, ", ");
        }
        if(utf8) {
            if(!curl) {
                curl = curl_easy_init();
            }
            // Need to encode the tag name in case it's UTF-8.
            encoded_tag = curl_easy_escape(curl, tags[i], (int)strlen(tags[i]));
            if(!encoded_tag) {
                ATMOS_ERROR("Unable to encode tag %s\n", tags[i]);
                continue;
            }
            header = AtmosUtil_cstring_append(header, &header_size, encoded_tag);
            curl_free(encoded_tag);
        } else {
            header = AtmosUtil_cstring_append(header, &header_size, tags[i]);
        }
    }

    RestRequest_add_header(request, header);

    if(curl) {
        curl_easy_cleanup(curl);
    }

    free(header);
}

void
AtmosUtil_set_tags_header2(RestRequest *request,
        const char **tags, int tag_count, int utf8) {
    char *header = NULL;
    size_t header_size = 0;
    int i;
    CURL *curl = NULL;
    char *encoded_tag;

    if(tags==NULL || tag_count < 1) {
        return;
    }

    header = AtmosUtil_cstring_append(header, &header_size,
            ATMOS_HEADER_TAGS ": ");

    for(i=0; i<tag_count; i++) {
        if(i>0) {
            header = AtmosUtil_cstring_append(header, &header_size, ", ");
        }
        if(utf8) {
            if(!curl) {
                curl = curl_easy_init();
            }
            // Need to encode the tag name in case it's UTF-8.
            encoded_tag = curl_easy_escape(curl, tags[i], (int)strlen(tags[i]));
            if(!encoded_tag) {
                ATMOS_ERROR("Unable to encode tag %s\n", tags[i]);
                continue;
            }
            header = AtmosUtil_cstring_append(header, &header_size, encoded_tag);
            curl_free(encoded_tag);
        } else {
            header = AtmosUtil_cstring_append(header, &header_size, tags[i]);
        }
    }

    RestRequest_add_header(request, header);

    if(curl) {
        curl_easy_cleanup(curl);
    }

    free(header);
}

void
AtmosUtil_parse_meta(xmlNode *metadata, char *name, char *value, int *listable) {
    xmlNode *child;
    xmlChar *xvalue;

    for(child = metadata->children; child; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;
        }
        if(!strcmp((char*)child->name, DIR_NODE_NAME)) {
            xvalue = xmlNodeGetContent(child);
            if(!xvalue) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(name, (char*)xvalue, ATMOS_META_NAME_MAX);
                xmlFree(xvalue);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_VALUE)) {
            xvalue = xmlNodeGetContent(child);
            if(!xvalue) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                strncpy(value, (char*)xvalue, ATMOS_META_VALUE_MAX);
                xmlFree(xvalue);
            }
        } else if(!strcmp((char*)child->name, DIR_NODE_LISTABLE)) {
            xvalue = xmlNodeGetContent(child);
            if(!xvalue) {
                ATMOS_WARN("No value found for %s\n", child->name);
            } else {
                if(!strcmp("true", (char*)xvalue)) {
                    *listable = 1;
                } else {
                    *listable = 0;
                }
                xmlFree(xvalue);
            }
        } else {
            ATMOS_WARN("Unknown node %s found inside %s\n", metadata->name,
                    child->name);
        }
    }
}

