#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "atmos_util.h"
#include "atmos_rest.h"

/**
 * Initialize constants here.
 */
const char *EMC_META_HDR_STR = "x-emc-meta: ";
const char *EMC_USER_HDR_STR = "x-emc-user: ";
const char *EMC_LISTABLE_META_HDR_STR = "x-emc-listable-meta: ";

const char* atime = "atime";
const char* mtime = "mtime";
const char* itime = "itime";
const char* emc_ctime = "ctime";
const char* type = "type";
const char* uid = "uid";
const char* gid = "gid";
const char* objectid = "objectid";
const char* objname = "objname";
const char* size = "size";
const char* nlink = "nlink";
const char* policyname = "policyname";

credentials* init_ws(const char *user_id, const char *key, const char *endpoint) {

    credentials *c = malloc(sizeof(credentials));
    memset(c, 0, sizeof(credentials));
    strncpy(c->tokenid, user_id, CREDS_SIZE);
    strncpy(c->secret, key, CREDS_SIZE);
    strncpy(c->accesspoint, endpoint, CREDS_SIZE);
    c->curl_verbose = 0;
    c->proxy_port = -1;
    return c;
}

void free_ws(credentials *c) {
	if(c->proxy_host) {
		free(c->proxy_host);
	}
	if(c->proxy_user) {
		free(c->proxy_user);
	}
	if(c->proxy_pass) {
		free(c->proxy_pass);
	}
    free(c);
}

/**
 * Configures the given connection to use a proxy.
 * @param c the Atmos credentials object
 * @param proxy_host the host to proxy through.  Set to NULL to disable proxy
 * support.
 * @param proxy_port the proxy host.  Set to -1 to use the default port.
 * @param proxy_user the user to authenticate against the proxy.  Set to NULL
 * to disable proxy authentication.
 * @param proxy_pass the password for the proxy user.
 */
void config_proxy(credentials *c, const char *proxy_host, int proxy_port,
		const char *proxy_user, const char *proxy_pass) {
	// Clear any existing proxy info first
	if(c->proxy_host) {
		free(c->proxy_host);
		c->proxy_host = NULL;
	}
	if(c->proxy_user) {
		free(c->proxy_user);
		c->proxy_user = NULL;
	}
	if(c->proxy_pass) {
		free(c->proxy_pass);
		c->proxy_pass = NULL;
	}

	if(proxy_host) {
		c->proxy_host = strdup(proxy_host);
	}
	c->proxy_port = proxy_port;
	if(proxy_user) {
		c->proxy_user = strdup(proxy_user);
	}
	if(proxy_pass) {
		c->proxy_pass = strdup(proxy_pass);
	}

}


int rename_ns(credentials *c, const char * uri, const char *new_uri, int force,
        ws_result *ws) {

    static const char* rename = "?rename";
    static const char* PATH_HEADER = "x-emc-path:";

    char *rename_uri = (char*) malloc(strlen(uri) + strlen(rename) + 1);
    http_method method = POST;
    char *path = NULL;
    char **headers = (char**) calloc(20, sizeof(char*));
    int header_count = 0;
    char theader[] = "x-emc-force:true";
    char fheader[] = "x-emc-force:false";
    sprintf(rename_uri, "%s%s", uri, rename);

    if (force)
        headers[header_count++] = theader;
    else
        headers[header_count++] = fheader;

    path = (char*) malloc(strlen(new_uri) + strlen(PATH_HEADER) + 1);
    sprintf(path, "%s%s", PATH_HEADER, new_uri);
    headers[header_count++] = path;
    http_request_ns(c, method, rename_uri, NULL, headers, header_count, NULL,
            ws);

    free(path);
    free(headers);
    free(rename_uri);

    return ws->return_code;
}

/**
 * Parses the acllist and populates the user and group ACLs if needed.
 * @param user_acl [inout] The user ACL header.  If not populated, will be set
 * to NULL on return.  If not NULL, pass to free() when done.
 * @param group_acl [inout] The group ACL header.  If not populated, will be set
 * to NULL on return.  If not NULL, pass to free() when done.
 * @param acllist [in] the list of ACL elements.  May be NULL to use default
 * ACL (uid=FULL_CONTROL,other=NONE).
 */
void add_acl_headers(char **user_acl, char **group_acl, acl *acllist) {
    int user_offset = 0;
    int group_offset = 0;

    *user_acl = NULL;
    *group_acl = NULL;

    if (!acllist)
        return;

    //end up with :
    //x-emc-useracl: name=perm,name=perm

    while (acllist) {
//        printf("Adding ACL: %s=%s, group: %d\n", acllist->user, acllist->permission, (int)acllist->is_group);

        if(acllist->is_group) {
            if(!(*group_acl)) {
                // Init header for first element
                *group_acl = malloc(ACL_SIZE);
                group_offset = sprintf(*group_acl, "x-emc-groupacl:%s=%s",
                        acllist->user,
                        acllist->permission);
            } else {
                group_offset += sprintf((*group_acl) + group_offset, ",%s=%s",
                        acllist->user,
                        acllist->permission);
            }
        } else {
            if(!(*user_acl)) {
                // Init header for first element
                *user_acl = malloc(ACL_SIZE);
                user_offset = sprintf(*user_acl, "x-emc-useracl:%s=%s",
                        acllist->user,
                        acllist->permission);
            } else {
                user_offset += sprintf((*user_acl) + user_offset, ",%s=%s",
                        acllist->user,
                        acllist->permission);
            }
        }
        acllist = acllist->next;
    }

//    if(*user_acl) {
//        printf("User ACL: %s\n", *user_acl);
//    }
//    if(*group_acl) {
//        printf("Group ACL: %s\n", *group_acl);
//    }

}

void add_meta_headers(char **emc_listable, char **emc_meta, user_meta *meta) {
    user_meta *index = meta;
    int emc_meta_loc = 0;
    int emc_listable_loc = 0;
    for (; index != NULL; index = index->next) {
        char *value, *key;
        value = index->value;
        key = index->key;

        while (*key == ' ')
            key++;
        while (*value == ' ')
            value++;

        if (index->listable == false) {
            if (!*emc_meta) {
                *emc_meta = malloc(8096);
                memset(*emc_meta, 0, 8096);
            }
            if (emc_meta_loc > 0) {
                emc_meta_loc += sprintf((*emc_meta) + emc_meta_loc, ",%s=%s",
                        key, value);
            } else {
                emc_meta_loc += sprintf((*emc_meta) + emc_meta_loc,
                        "X-Emc-Meta:%s=%s", key, value);
            }
        } else if (index->listable == true) {
            if (!*emc_listable) {
                *emc_listable = malloc(8096);
                memset(*emc_listable, 0, 8096);
            }

            if (emc_listable_loc > 0) {
                emc_listable_loc += sprintf((*emc_listable) + emc_listable_loc,
                        ",%s=%s", key, value);
            } else {
                emc_listable_loc += sprintf((*emc_listable) + emc_listable_loc,
                        "X-Emc-Listable-meta:%s=%s", key, value);
            }

        }
    }

}

int create_ns(credentials *c, const char * uri, const char *content_type, acl *acl,
        postdata *data, user_meta *meta, ws_result *ws) {

    http_method method = POST;
    char **headers = calloc(20, sizeof(char*));
    int header_count = 0;
    char *meta_listable_header = NULL;
    char *meta_header = NULL;
    char *user_acl = NULL;
    char *group_acl = NULL;

    if (data) {
        // No range on create requests
        data->offset = -1;
    }

    if (acl) {
        add_acl_headers(&user_acl, &group_acl, acl);
        if(user_acl) {
            headers[header_count++] = user_acl;
        }
        if(group_acl) {
            headers[header_count++] = group_acl;
        }
    }

    if (meta) {
        add_meta_headers(&meta_listable_header, &meta_header, meta);
        if (meta_listable_header) {
            headers[header_count++] = meta_listable_header;
        }
        if (meta_header) {
            headers[header_count++] = meta_header;
        }
    }

    http_request_ns(c, method, uri, content_type, headers, header_count, data,
            ws);

    if(user_acl)
        free(user_acl);
    if(group_acl)
        free(group_acl);
    if (meta_listable_header)
        free(meta_listable_header);
    if (meta_header)
        free(meta_header);

    free(headers);

    return ws->return_code;
}

int update_ns(credentials *c, const char * uri, const char *content_type, acl *acl,
        postdata *data, user_meta *meta, ws_result *ws) {

    char **headers = calloc(20, sizeof(char*));
    http_method method = PUT;
    char *meta_listable_header = NULL;
    char *meta_header = NULL;
    char *user_acl = NULL;
    char *group_acl = NULL;

    int header_count = 0;
    if (acl) {
        add_acl_headers(&user_acl, &group_acl, acl);
        if(user_acl) {
            headers[header_count++] = user_acl;
        }
        if(group_acl) {
            headers[header_count++] = group_acl;
        }
    }
    if (meta) {
        add_meta_headers(&meta_listable_header, &meta_header, meta);
        if (meta_listable_header) {
            headers[header_count++] = meta_listable_header;
        }
        if (meta_header) {
            headers[header_count++] = meta_header;
        }
    }

    http_request_ns(c, method, uri, content_type, headers, header_count, data,
            ws);

    if(user_acl)
        free(user_acl);
    if(group_acl)
        free(group_acl);
    if (meta_listable_header)
        free(meta_listable_header);
    if (meta_header)
        free(meta_header);
    free(headers);

    return ws->return_code;
}

int list_ns(credentials *c, const char * uri, const char *token, int limit,
        int include_meta, char *user_tag_list, char *system_tag_list,
        ws_result *ws) {

    char **headers;
    int count;

    char header_limit[256];
    char header_token[256];
    char header_include[64];
    char header_user[1024];
    char header_system[1024];

    http_method method = GET;
    headers = (char**) calloc(20, sizeof(char*));
    count = 0;
    if (limit>0) {
        snprintf(header_limit, 255, "x-emc-limit:%d", limit);
        headers[count++] = header_limit;
    }
    if(token) {
        snprintf(header_token, 255, "x-emc-token:%s", token);
        headers[count++] = header_token;
    }
    if(include_meta) {
        snprintf(header_include, 63, "x-emc-include-meta:%d", include_meta);
        headers[count++] = header_include;
        if(user_tag_list) {
            snprintf(header_user, 1023, "x-emc-user-tags:%s", user_tag_list);
            headers[count++] = header_user;
        }
            snprintf(header_system, 1023, "x-emc-system-tags:%s", system_tag_list);
        if(system_tag_list) {
            headers[count++] = header_system;
        }
    }
    http_request_ns(c, method, uri, NULL, headers, count, NULL, ws);

    free(headers);

    return ws->return_code;
}

int delete_ns(credentials *c, const char *uri, ws_result *ws) {
    http_method method = aDELETE;
    char **headers = calloc(20, sizeof(char*));
    http_request_ns(c, method, uri, NULL, headers, 0, NULL, ws);
    free(headers);
    return ws->return_code;
}

void parse_headers(ws_result* ws, system_meta* sm, user_meta** head_ptr_um) {
    int i = 0;
    user_meta *ptr_um = NULL;
    for (; i < ws->header_count; i++) {
        if (0
                == strncmp(ws->headers[i], EMC_META_HDR_STR,
                        strlen(EMC_META_HDR_STR))) {
            char *meta_ptr = ws->headers[i] + strlen(EMC_META_HDR_STR);
            char delims[] = ",";
            char *result = NULL;
            char *hdr_context;
            result = strtok_r(meta_ptr, delims, &hdr_context);
            while (result != NULL) {

                //trim leading whitespace
                int offset = 0;
                for (; result[offset] == ' '; offset++)
                    ;
                result += offset;

                if (0 == strncmp(result, atime, strlen(atime))) {
                    strcpy(sm->atime, result + strlen(atime) + 1);
                } else if (0 == strncmp(result, mtime, strlen(mtime))) {
                    strcpy(sm->mtime, result + strlen(mtime) + 1);
                } else if (0 == strncmp(result, emc_ctime, strlen(emc_ctime))) {
                    strcpy(sm->ctime, result + strlen(emc_ctime) + 1);
                } else if (0 == strncmp(result, itime, strlen(itime))) {
                    strcpy(sm->itime, result + strlen(itime) + 1);
                } else if (0 == strncmp(result, type, strlen(type))) {
                    strcpy(sm->type, result + strlen(type) + 1);
                } else if (0 == strncmp(result, uid, strlen(uid))) {
                    strcpy(sm->uid, result + strlen(uid) + 1);
                } else if (0 == strncmp(result, gid, strlen(gid))) {
                    strcpy(sm->gid, result + strlen(gid) + 1);
                } else if (0 == strncmp(result, objectid, strlen(objectid))) {
                    strcpy(sm->objectid, result + strlen(objectid) + 1);
                } else if (0 == strncmp(result, objname, strlen(objname))) {
                    strcpy(sm->objname, result + strlen(objname) + 1);
                } else if (0 == strncmp(result, size, strlen(size))) {
                    sm->size = atoi(result + strlen(size) + 1);
                } else if (0 == strncmp(result, nlink, strlen(nlink))) {
                    sm->nlink = atoi(result + strlen(nlink) + 1);
                } else if (0
                        == strncmp(result, policyname, strlen(policyname))) {
                    strcpy(sm->policyname, result + strlen(policyname) + 1);
                } else {
                    char um_delims[] = "=";
                    char *meta_context;
                    char *um_result = NULL;
                    int meta_index = 0;
                    if (ptr_um) {
                        ptr_um->next = malloc(sizeof(user_meta));
                        memset(ptr_um->next, 0, sizeof(user_meta));
                        ptr_um = ptr_um->next;
                    } else {
                        *head_ptr_um = malloc(sizeof(user_meta));
                        memset(*head_ptr_um, 0, sizeof(user_meta));
                        ptr_um = *head_ptr_um;
                    }
                    memset(ptr_um, 0, sizeof(user_meta));
                    ptr_um->listable = false;
                    um_result = strtok_r(result, um_delims, &meta_context);
                    while (um_result != NULL) {
                        if (0 == meta_index) {
                            strcpy(ptr_um->key, um_result);
                        } else {
                            strcpy(ptr_um->value, um_result);
                        }
                        meta_index++;
                        um_result = strtok_r(NULL, um_delims, &meta_context);
                    }
                }
                result = strtok_r(NULL, delims, &hdr_context);
            }
        } else if (0
                == strncmp(ws->headers[i], EMC_USER_HDR_STR,
                        strlen(EMC_USER_HDR_STR))) {
            ;

        } else if (0
                == strncmp(ws->headers[i], EMC_LISTABLE_META_HDR_STR,
                        strlen(EMC_LISTABLE_META_HDR_STR))) {
            //listable x-emc-listable-meta: meta_test=meta_pass
            char hdr_delims[] = ",";
            char *hdr_result = NULL;
            char *hdr_context = NULL;
            hdr_result = strtok_r(
                    ws->headers[i] + strlen(EMC_LISTABLE_META_HDR_STR),
                    hdr_delims, &hdr_context);
            while (hdr_result != NULL) {
                char delims[] = "=";
                char *result = NULL;
                char *inner_context;
                int meta_index = 0;
                if (ptr_um) {
                    ptr_um->next = malloc(sizeof(user_meta));
                    memset(ptr_um->next, 0, sizeof(user_meta));
                    ptr_um = ptr_um->next;
                } else {
                    *head_ptr_um = malloc(sizeof(user_meta));
                    memset(*head_ptr_um, 0, sizeof(user_meta));
                    ptr_um = *head_ptr_um;
                }
                memset(ptr_um, 0, sizeof(user_meta));

                ptr_um->listable = true;
                result = strtok_r(hdr_result, delims, &inner_context);
                while (*result == ' ')
                    result++;
                while (result != NULL) {
                    if (0 == meta_index) {
                        strcpy(ptr_um->key, result);
                    } else {
                        strcpy(ptr_um->value, result);
                    }
                    meta_index++;
                    result = strtok_r(NULL, delims, &inner_context);
                }
                hdr_result = strtok_r(NULL, hdr_delims, &hdr_context);
            }
        } else if (0
                == strncmp(ws->headers[i], EMC_META_HDR_STR,
                        strlen(EMC_META_HDR_STR))) {
            ptr_um->listable = false;

        }
    }
}

int user_meta_ns(credentials *c, const char *uri, const char * content_type,
        user_meta *meta, ws_result * ws) {
    if (meta) {
        static const char* user_meta_uri = "?metadata/user";
        char *meta_uri = (char*) malloc(
                strlen(uri) + strlen(user_meta_uri) + 1);
        char **headers = calloc(20, sizeof(char*));
        int header_count = 0;
        char *unlistable_meta = NULL, *listable_meta = NULL;
        sprintf(meta_uri, "%s%s", uri, user_meta_uri);

        add_meta_headers(&listable_meta, &unlistable_meta, meta);

        if (listable_meta) {
        	headers[header_count++] = listable_meta;
        }
        if (unlistable_meta) {
        	headers[header_count++] = unlistable_meta;
        }

        http_request_ns(c, POST, meta_uri, content_type, headers, header_count,
                NULL, ws);

        if (listable_meta)
            free(listable_meta);
        if (unlistable_meta)
            free(unlistable_meta);

        free(meta_uri);
        free(headers);
    }
    return ws->return_code;
}

int get_meta_ns(credentials *c, const char *object_name, ws_result *ws) {
    char **headers = (char**) calloc(20, sizeof(char*));
    int header_count = 0;
    http_request_ns(c, HEAD, object_name, NULL, headers, header_count, NULL,
            ws);

    free(headers);
    return ws->return_code;
}

//int object_get_listable_meta(const char *object_name) 
//{

//}

user_meta* new_user_meta(const char *key, const char *value, int listable) {
    user_meta *um = malloc(sizeof(user_meta));
    memset(um, 0, sizeof(user_meta));
    strcpy(um->key, key);
    strcpy(um->value, value);
    um->listable = listable;
    um->next = NULL;
    return um;
}
void add_user_meta(user_meta *head, const char *key, const char *value,
        int listable) {
    user_meta *um = malloc(sizeof(user_meta));
    memset(um, 0, sizeof(user_meta));
    strcpy(um->key, key);
    strcpy(um->value, value);
    um->next = NULL;
    um->listable = listable;

    if (head->next == NULL) {
        head->next = um;
    } else {
        user_meta *current = head->next;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = um;
    }

}

void free_user_meta(user_meta *um) {
    user_meta *idx = um;
    while (idx != NULL) {
        user_meta *f = idx;
        idx = idx->next;
        free(f);
    }
}

user_meta* find_user_meta(user_meta *um, const char *name) {
    while (um != NULL) {
        if (!strcmp(name, um->key)) {
            return um;
        }
        um = um->next;
    }

    return NULL;
}

void get_service_info(credentials *c, ws_result *result) {
    char **headers = calloc(20, sizeof(char*));
    int header_count = 0;
    char *sys_info = "/rest/service";
    http_request(c, GET, sys_info, NULL, headers, header_count, NULL, result);
    free(headers);
}

int create_obj(credentials *c, char *obj_id, const char *content_type,
		acl *acl, user_meta *meta, ws_result *ws) {

    http_method method = POST;
    char **headers = calloc(20, sizeof(char*));
    int header_count = 0;
    char *meta_listable_header = NULL;
    char *meta_header = NULL;
    char *obj_uri = "/rest/objects";
    char *user_acl = NULL;
    char *group_acl = NULL;

    if (acl) {
        add_acl_headers(&user_acl, &group_acl, acl);
        if(user_acl) {
            headers[header_count++] = user_acl;
        }
        if(group_acl) {
            headers[header_count++] = group_acl;
        }
    }

    if (meta) {
        add_meta_headers(&meta_listable_header, &meta_header, meta);
        if (meta_listable_header) {
            headers[header_count++] = meta_listable_header;
        }
        if (meta_header) {
            headers[header_count++] = meta_header;
        }
    }
    http_request(c, method, obj_uri, content_type, headers, header_count, NULL,
            ws);
    get_object_id(ws->headers, ws->header_count, obj_id);

    if(user_acl)
        free(user_acl);
    if(group_acl)
        free(group_acl);
    if (meta_listable_header)
        free(meta_listable_header);
    if (meta_header)
        free(meta_header);
    free(headers);
    return 1;
}

int delete_obj(credentials *c, char *obj, ws_result *ws) {
    http_method method = aDELETE;
    char **headers = calloc(20, sizeof(char*));
    char *object_path = "/rest/objects/";
    char *obj_uri = (char*) malloc(strlen(object_path) + strlen(obj) + 1);
    sprintf(obj_uri, "%s%s", object_path, obj);
    http_request(c, method, obj_uri, NULL, headers, 0, NULL, ws);
    free(headers);
    return ws->return_code;
}

int read_obj(credentials *c, char *object_id, postdata* pd, int limit,
        ws_result* ws) {
    char **headers;
    int count;
    char *object_path = "/rest/objects/";
    char *obj_uri;

    http_method method = HEAD;
    if (pd)
        method = GET;
    headers = (char**) calloc(20, sizeof(char*));
    count = 0;
    if (limit) {
        sprintf(headers[count++], "x-emc-limit: %d", limit);
    }
    obj_uri = (char*) malloc(strlen(object_path) + strlen(object_id) + 1);
    sprintf(obj_uri, "%s%s", object_path, object_id);
    http_request(c, method, obj_uri, NULL, headers, count, pd, ws);
    free(obj_uri);
    free(headers);
    return ws->return_code;
}

int read_obj_ns(credentials *c, const char *object_id, postdata* pd, ws_result* ws) {
    char **headers;
    int count;
    char *object_path = "/rest/namespace/";
    char *obj_uri;

    http_method method = GET;
    headers = (char**) calloc(20, sizeof(char*));
    count = 0;
    obj_uri = (char*) malloc(strlen(object_path) + strlen(object_id) + 1);
    sprintf(obj_uri, "%s%s", object_path, object_id);
    http_request(c, method, obj_uri, NULL, headers, count, pd, ws);
    free(headers);
    free(obj_uri);
    return ws->return_code;
}


int update_obj(credentials *c, char *object_id, const char* content_type, acl* acl,
        postdata* data, user_meta* meta, ws_result *ws) {

    char **headers = calloc(20, sizeof(char*));
    http_method method = PUT;
    char *meta_listable_header = NULL;
    char *meta_header = NULL;
    char *object_path = "/rest/objects/";
    char *obj_uri;
    char *user_acl = NULL;
    char *group_acl = NULL;

    int header_count = 0;
    if (acl) {
        add_acl_headers(&user_acl, &group_acl, acl);
        if(user_acl) {
            headers[header_count++] = user_acl;
        }
        if(group_acl) {
            headers[header_count++] = group_acl;
        }
    }
    if (meta) {
        //add meta headers on create
    }
    obj_uri = (char*) malloc(strlen(object_path) + strlen(object_id) + 1);
    sprintf(obj_uri, "%s%s", object_path, object_id);

    http_request(c, method, obj_uri, content_type, headers, header_count, data,
            ws);

    if(user_acl)
        free(user_acl);
    if(group_acl)
        free(group_acl);
    if (meta_listable_header)
        free(meta_listable_header);
    if (meta_header)
        free(meta_header);
    free(headers);

    return 0;
}

void get_object_id(char** headers, int count, char *object_id) {

    int i;
    char *location_str = "location: /rest/objects/";
    for (i = 0; i < count; i++) {
        if (0 == strncmp(location_str, headers[i], strlen(location_str))) {
            strcpy(object_id, headers[i] + strlen(location_str));
        }
    }
}
