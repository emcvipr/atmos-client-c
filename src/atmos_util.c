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

int AtmosUtil_cstring_cmp(const void *a, const void *b)
{
    return strcmp(* (char * const *) a, * (char * const *) b);
}

char *AtmosUtil_cstring_append(char *buf, size_t *bufsz, const char *str) {
    size_t strsz = strlen(str);
    size_t bufpos = 0;
    if(buf != NULL) {
        bufpos = strlen(buf);
    }

    if(bufpos + strsz + 1 > *bufsz) {
        size_t newsize = *bufsz + strsz + 256;
        buf = realloc(buf, newsize);
        *bufsz = newsize;
    }

    strcpy(buf + bufpos, str);
    buf[bufpos + strsz] = 0;

    return buf;
}

void AtmosUtil_get_date(char *formated_time)
{
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

    for(i=0, j=0; i<sz; i++) {
        if(header[i] == ' ') {
            if(i+1 == sz || header[i+1] == ' ' || header[i+1] == ','
                    || (!sepfound && header[i+1] == ':')) {
                // Trim whitespace at the end of a string, surrounding the colon,
                // collapse it inside the value, and at the end of a metadata
                // element before a comma.
            } else {
                header[j++] = header[i];
            }
        } else if(header[i] == ':' && !sepfound) {
            sepfound = 1;

            header[j++] = header[i];

            // Trim any whitespace following the first colon.
            while(i+1<sz && header[i+1] == ' ') {
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
    for( ; s[i] != ':'; i++)
	s[i] = tolower(s[i]);
}

void AtmosUtil_lowercase(char *s) {
    int i = 0;
    for( ; s[i] != '\0'; i++)
	s[i] = tolower(s[i]);
}


char *AtmosUtil_base64decode(const char *base64encoded, size_t length)
{
    BIO *b64, *bmem;
    char *buffer = (char *)malloc(length); // This really should be 3/4 of length

    memset(buffer, 0, length);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf((void*)base64encoded, (int)length);
    bmem = BIO_push(b64, bmem);
    BIO_read(bmem, buffer, (int)length);
    BIO_free_all(bmem);

    b64= NULL;

    return buffer;
}

char *AtmosUtil_base64encode(const char *normal, size_t length)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    char *buff = NULL;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, normal, (int)length);
    if(BIO_flush(b64) != 1) {
        return NULL;
    }

    BIO_get_mem_ptr(b64, &bptr);

    buff = (char *)malloc(bptr->length);
    memcpy(buff, bptr->data, bptr->length-1);
    buff[bptr->length-1] = 0;

    BIO_free_all(b64);
    b64= NULL;
    return buff;
}

char *AtmosUtil_HMACSHA1(const char *hash_string, const char *key, size_t key_len) {
    const EVP_MD *evp_md = EVP_sha1();
    unsigned int md_len;
    unsigned char md[EVP_MAX_MD_SIZE];
    char *newkey = AtmosUtil_base64decode(key,key_len);
    int new_key_len = strlen(newkey);

    HMAC(evp_md, newkey, new_key_len, (const unsigned char*)hash_string,
            strlen(hash_string), md, &md_len);
    free(newkey);
    return AtmosUtil_base64encode((char*)md, md_len);

}


xmlChar *AtmosUtil_select_single_node_value(xmlDocPtr doc, xmlChar *selector, int use_cos_ns) {
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;
    xmlNodePtr xmlNode;
    xmlChar *value;

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        return NULL;
    }

    if(use_cos_ns) {
        if(xmlXPathRegisterNs(xpathCtx, BAD_CAST "cos",
                BAD_CAST "http://www.emc.com/cos/")) {
            fprintf(stderr,"Error: unable to register cos namespace\n");
            xmlXPathFreeContext(xpathCtx);
            return NULL;

        }
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(selector, xpathCtx);
    if(xpathObj == NULL) {
        fprintf(stderr,
                "Error: unable to evaluate xpath expression \"%s\"\n",
                selector);
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    xpathNodeSet = xpathObj->nodesetval;
    if(!xpathNodeSet || xpathNodeSet->nodeNr == 0) {
        fprintf(stderr,"Error: No nodes returned from \"%s\"\n", selector);
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    if(xpathNodeSet->nodeNr > 1) {
        fprintf(stderr,
                "Error: Multiple (%d) nodes returned from \"%s\"\n",
                xpathNodeSet->nodeNr, selector);
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    xmlNode = xpathNodeSet->nodeTab[0];
    value = xmlNodeGetContent(xmlNode);

    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);

    return value;
}


void
AtmosUtil_set_metadata_header(AtmosMetadata *meta, int meta_count,
        int listable, int utf8, RestRequest *request) {
    char *meta_header = NULL;
    size_t meta_header_sz = 0;
    int i;
    CURL *curl = NULL;

    if(meta_count < 1) {
        return;
    }

    if(!listable) {
        meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                ATMOS_HEADER_META);
    } else {
        meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                ATMOS_HEADER_LISTABLE_META);
    }
    meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
            ": ");

    for(i=0; i<meta_count; i++) {
        if(!utf8) {
            // Sanity check name and value.
            if(AtmosUtil_meta_char_check(meta[i].name)
                    || AtmosUtil_meta_char_check(meta[i].value)) {
                ATMOS_WARN("Invalid character in metadata %s:%s - "
                        "use utf8 mode\n", meta[i].name, meta[i].value);
                continue;
            }
        }

        if(i>0) {
            meta_header = AtmosUtil_cstring_append(meta_header,
                    &meta_header_sz, ",");
        }

        if(utf8) {
            char *enc;

            if(!curl) {
                // for some reason, we need a curl handle for curl_easy_escape
                curl = curl_easy_init();
            }
            // URLEncode the name and value.
            enc = curl_easy_escape(curl, meta[i].name, 0);
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    enc);
            curl_free(enc);
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    "=");
            enc = curl_easy_escape(curl, meta[i].value, 0);
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    enc);
            curl_free(enc);
        } else {
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    meta[i].name);
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    "=");
            meta_header = AtmosUtil_cstring_append(meta_header, &meta_header_sz,
                    meta[i].value);
        }
    }

    RestRequest_add_header(request, meta_header);

    free(meta_header);
    if(curl) {
        curl_easy_cleanup(curl);
    }
}
void
AtmosUtil_set_acl_header(AtmosAclEntry *acl, int acl_count,
        RestRequest *request) {
    char *user_acl_header = NULL;
    size_t user_acl_header_sz = 0;
    char *group_acl_header = NULL;
    size_t group_acl_header_sz = 0;
    int i;

    if(acl_count < 1) {
        return;
    }

    for(i=0; i<acl_count; i++) {

    }

    if(user_acl_header_sz > 0) {
        RestRequest_add_header(request, user_acl_header);
        free(user_acl_header);
    }

    if(group_acl_header_sz > 0) {
        RestRequest_add_header(request, group_acl_header);
        free(group_acl_header);
    }

}

void
AtmosUtil_log(const char *level, const char *file, const int line, const char *fmt, ...) {
    char msg[1024];
    char stime[255];
    time_t t;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 1024, fmt, args);
    va_end(args);

    // Get the current timestamp
    t = time(NULL);
    ctime_r(&t, msg);

    fprintf(stderr, "%s - %s:%d: %s", stime, file, line, msg);
}

int AtmosUtil_meta_char_check(const char *str) {
    int len = strlen(str);
    int i;

    for(i=0; i<len; i++) {
        switch(str[i]) {
        case ',':
        case '\n':
        case '=':
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], str[i]);
            return 1;
        default:
            break;
        }

        if(str[i] < 32) {
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], str[i]);
            return 1;
        }
        if(str[i] & 0x80) {
            ATMOS_DEBUG("Invalid char %c(0x%x)\n", str[i], str[i]);
            return 1;
        }
    }

    return 0;
}

