#ifndef __AOL_INIT__
#define __AOL_INIT__

/**
 * @file atmos_rest.h
 */

#include "transport.h"
#include "stdio.h"  /*FILE*/
#include "unistd.h" /*off_t*/

typedef struct requestval {
    credentials c;
    char ** emc_headers;
    char * path;
    char * method;
    char * range;
    char * content_type;
    char * date;
    char * uid;
    char * signature;
} request;

#define ACL_GROUP_PUBLIC "other"
#define ACL_READ "READ"
#define ACL_WRITE "WRITE"
#define ACL_FULL "FULL_CONTROL"
#define ACL_NONE "NONE"

/**
 * Linked list that defines ACL entries
 */
typedef struct {
    char user[512]; /* The user (or group) name */
    char permission[32]; /* READ, WRITE, FULL_CONTROL, or NONE */
    char is_group; /* If true, this is a group ACL, not a user ACL */
    void *next;
} acl;


typedef struct sys_info {
    char version[1024];
} sys_info;

#define TIMESIZE 40
#define UIDSIZE  44
#define POLICYSIZE  44
#define GIDSIZE  44
#define OBJECTIDSIZE  45

/* Size to allocate for serializing ACL headers */
#define ACL_SIZE 1024

typedef struct System_meta {
    char atime[TIMESIZE];
    char mtime[TIMESIZE];
    char ctime[TIMESIZE];
    char itime[TIMESIZE];
    char type[1024];
    char uid[UIDSIZE];
    char gid[GIDSIZE];
    char objectid[OBJECTIDSIZE];
    char objname[1024];
    unsigned long long size;
    int nlink;
    char policyname[POLICYSIZE];
} system_meta;


//metavalues are max size 1k
typedef struct Metaval {
    char key[1024];
    char value[1024];
    int listable;
    void *next;
} user_meta;

typedef struct listing {
    char name[256];
    char type[128];
} listing;

extern const char *EMC_META_HDR_STR;
extern const char *EMC_USER_HDR_STR;
//static const char *EMC_GROUPACL_HDR_STR = "x-emc-groupacl";
//static const char *EMC_USERACL_HDR_STR = "x-emc-useracl";
extern const char *EMC_LISTABLE_META_HDR_STR;

extern const char* atime;
extern const char* mtime;
extern const char* itime;
extern const char* emc_ctime;
extern const char* type;
extern const char* uid;
extern const char* gid;
extern const char* objectid;
extern const char* objname;
extern const char* size;
extern const char* nlink;
extern const char* policyname;

//Object - CRUD
int create_obj(credentials *c, char *obj_id, const char *content_type, acl *acl,user_meta *meta, ws_result *ws);
int read_obj(credentials *c, char *object_id, postdata* d, int limit, ws_result* ws);
int update_obj(credentials *c, char *object_id, const char* content_type, acl* acl, postdata* data, user_meta* meta, ws_result *ws);
int delete_obj(credentials *c, char *object_id, ws_result *ws);

/**
 * Renames an object in the Atmos namespace.
 * @param path The Atmos namespace path.  This should be the relative path of
 * the object, e.g. /foo/bar.txt (not /rest/foo/bar.txt).
 * @param new_path the new Atmos namespace path for the object.
 * @param force if nonzero, the rename will overwrite an existing file if it
 * exists.  Note that overwrite operations are not synchronous and there may
 * be a slight delay before the rename is complete.
 * @param ws result information from the operation, including the response
 * body (if any).  Be sure to call init_ws on the object before calling and
 * free_ws on the object after you're done with it.
 * @return the HTTP response code from the operation.
 */
int rename_ns(credentials *c, const char *path, const char *new_path,
		int force, ws_result *ws);

/**
 * Creates a new object in the Atmos namespace
 * @param c the Atmos credentials
 * @param path The Atmos namespace path.  This should be the relative path of
 * the object, e.g. /foo/bar.txt (not /rest/foo/bar.txt).
 * @param data content for the object.  See the postdata structure for more
 * information.  If NULL, an empty object will be created.
 * @param content_type the MIME type of the object.  If NULL, the type will
 * default to application/octet-stream.
 * @param acl the ACL to apply to the object.  May be NULL.
 * @param user_meta user metadata to apply to the object.  May be NULL.
 * @param ws result information from the operation, including the response
 * body (if any).  Be sure to call init_ws on the object before calling and
 * free_ws on the object after you're done with it.
 * @return the HTTP response code from the operation.
 */
int create_ns(credentials *c, const char * uri, const char *content_type, acl *acl,
		postdata *data, user_meta *meta, ws_result *ws);


/**
 * Lists the contents of a directory in the Atmos namespace.
 * @param c the Atmos credentials
 * @param path The Atmos namespace path.  This should be the relative path of
 * the directory, e.g. /foo/bar.txt (not /rest/foo/).  Note that Atmos
 * directory objects always end in a slash.
 * @param token if non-null, the x-emc-token to continue a directory listing
 * with.
 * @param limit if greater than zero, the maximum number of objects to return
 * in a listing.
 * @param include_meta if nonzero, metdata will be fetched with entries.
 * @param user_tag_list if include_meta is set, you may optionally provide a
 * list of comma-separated user metadata tags to fetch.
 * @param system_tag_list if include_meta is set, you may optionally provide
 * a list of comma-separated system metadata tags to fetch.
 * @param ws result information from the operation, including the response
 * body (if any).  Be sure to call init_ws on the object before calling and
 * free_ws on the object after you're done with it.  Also be sure to check
 * the value of the x-emc-token response header.  If it is not null, you need
 * to call list_ns again with that token to continue your listing.
 * @return the HTTP response code from the operation.
 */
int list_ns(credentials *c, const char * uri, const char *token, int limit, int include_meta,
        char *user_tag_list, char *system_tag_list, ws_result *ws);

/**
 * Updates an object in the Atmos namespace
 * @param c the Atmos credentials
 * @param path The Atmos namespace path.  This should be the relative path of
 * the object, e.g. /foo/bar.txt (not /rest/foo/bar.txt).
 * @param data content for the object.  See the postdata structure for more
 * information.  If NULL, an empty object will be created.
 * @param content_type the MIME type of the object.  If NULL, the type will
 * default to application/octet-stream.
 * @param acl the ACL to apply to the object.  May be NULL.
 * @param user_meta user metadata to apply to the object.  May be NULL.
 * @param ws result information from the operation, including the response
 * body (if any).  Be sure to call init_ws on the object before calling and
 * free_ws on the object after you're done with it.
 * @return the HTTP response code from the operation.
 */
int update_ns(credentials *c, const char * uri, const char *content_type, acl *acl,
		postdata* data, user_meta *meta, ws_result *ws);


int read_obj_ns(credentials *c, const char *uri, postdata* d, ws_result* ws);

int delete_ns(credentials *c, const char *object_id, ws_result *ws);

/**
 * Gets object metadata.  This issues a HEAD call against the object name and
 * fetches the object's metadata.  On success, pass the ws_result to
 * parse_headers if you want to extract the metadata.
 * @param c The Atmos credentials
 * @param object_name the object's path in the namespace
 * @param ws result information from the operation, including the response
 * body (if any).  Be sure to call init_ws on the object before calling and
 * free_ws on the object after you're done with it.
 * @return the HTTP response code from the operation.
 */
int get_meta_ns(credentials *c,const char *object_name, ws_result *ws);

//namespace metadata
int user_meta_ns(credentials *c, const char *uri, const char * content_type, user_meta *meta, ws_result* ws);

//int set_acl_obj(credentials *c, const char *objectid, acl* acl, ws_result *ws);
//int get_acl_obj(credentials *c, const char *objectid, acl* acl, ws_result *ws);
//int get_info_obj(credentials *c, char *objectid, ws_result* ws);//returns object details - replica's etc
//int list_obj(credentials *c, char* objectid,char *tag, ws_result * ws); //retrieves all obj id's indexed by tag

//int set_meta_obj(credentials *c, const char *object_name, const char *key, const char *val);
//int get_meta_obj(credentials *c,const char *object_name);


//versionining - new
//int create_version();
//int delete_version();
//int list_versions();
//int restore_version();

/**
 * Gets extended information about an object including replica info, retention,
 * and expiration policies.
 * @param c the Atmos credentials
 * @param object_id the Atmos Object ID to inspect
 * @param ws the result of the operation.  The object information will be
 * returned in ws->response_body as an XML blob.  The response is freed by
 * calling result_deinit().
 */
void get_object_info(credentials *c, const char *object_id, ws_result *ws);

/**
 * Gets extended information about an object including replica info, retention,
 * and expiration policies.
 * @param c the Atmos credentials
 * @param object_id the Atmos object path (namespace) to inspect
 * @param ws the result of the operation.  The object information will be
 * returned in ws->response_body as an XML blob.  The response is freed by
 * calling result_deinit().
 */
void get_object_info_ns(credentials *c, const char *object_path, ws_result *ws);


//Take a ws_result and break its headers into system and user meta structs
void parse_headers(ws_result*, system_meta*, user_meta**);

//atmos specific helpers
credentials* init_ws(const char *user_id, const char *key, const char *endpoint);
void free_ws(credentials *creds);

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
		const char *proxy_user, const char *proxy_pass);

//user meta data helpers
user_meta* new_user_meta(const char *key, const char *value, int listable);
void add_user_meta(user_meta *head, const char *key, const char *value, int listable);
void free_user_meta(user_meta *um);

/**
 * Searches the given user_meta list for a value with the given name.
 */
user_meta* find_user_meta(user_meta *um, const char *name);

void get_service_info(credentials *c, ws_result *result) ;

void get_object_id(char** headers, int count, char *obj);


#endif
