/******************************************************************************
 * NOTE: This file contains UTF-8 characters.  If your compiler does not accept
 * this encoding (most should), comment out:
 * - test_meta_generator_utf8
 ******************************************************************************/
/*
 * test_atmos.c
 *
 *  Created on: Oct 19, 2012
 *      Author: cwikj
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/rand.h>

#include "seatest.h"
#include "test.h"
#include "atmos.h"
#include "atmos_util.h"
#include "atmos_private.h" // To test private methods

#define TEST_DIR "atmos-c-unittest"
#define TEST_KEYPOOL "atmos-c-unittest-pool"

static char user_id[255];
static char key[64];
static char endpoint[255];

// For testing ACLs
char uid1[64];
char uid2[64];

// Proxy support
char proxy_host[256];
char proxy_user[256];
char proxy_pass[256];
int proxy_port;

int atmos_major;
int atmos_minor;
int atmos_sp;

static const char
        *filechars =
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~!$&\"()*+;:";

static void random_file(char *name, int count) {
    int i = 0;
    size_t num_filechars = strlen(filechars);
    for (; i < count; i++) {
        name[i] = filechars[random() % num_filechars];
    }
    name[i] = 0;
}

/** Generate a 20-byte key and base64 encode it. */
char *random_key() {
    unsigned char key[20];
    char *bkey,*k2;
    RAND_pseudo_bytes(key, 20);
    bkey = AtmosUtil_base64encode((char*)key, 20);

    // Make it url safe
    for(k2 = bkey; *k2; k2++) {
        if(*k2 == '/') {
            *k2 = '_';
        } else if(*k2 == '+') {
            *k2 = '-';
        }
    }
    return bkey;
}

void check_error(AtmosResponse *response) {
    RestResponse *rest_response = (RestResponse*) response;
    if (rest_response->http_code == 0) {
        // Transport-level error
        printf("Connection error %d: %s\n", rest_response->curl_error,
                rest_response->curl_error_message);
    } else if (rest_response->http_code > 399) {
        printf("HTTP error %d: %s\n", rest_response->http_code,
                rest_response->http_status);
    }
    if (response->atmos_error > 0) {
        printf("Atmos error %d: %s\n", response->atmos_error,
                response->atmos_error_message);
    }
}

static const char *string_to_sign = "POST\n"
    "application/octet-stream\n"
    "\n"
    "Thu, 05 Jun 2008 16:38:19 GMT\n"
    "/rest/objects\n"
    "x-emc-date:Thu, 05 Jun 2008 16:38:19 GMT\n"
    "x-emc-groupacl:other=NONE\n"
    "x-emc-listable-meta:part4/part7/part8=quick\n"
    "x-emc-meta:part1=buy\n"
    "x-emc-uid:6039ac182f194e15b9261d73ce044939/user1\n"
    "x-emc-useracl:john=FULL_CONTROL,mary=WRITE";

void get_atmos_client(AtmosClient *atmos) {
    AtmosClient_init(atmos, endpoint, -1, user_id, key);
    //RestClient_add_curl_config_handler((RestClient*) atmos, rest_verbose_config);

    if (strlen(proxy_host) > 0) {
        if (strlen(proxy_user) > 0) {
            RestClient_set_proxy((RestClient*) atmos, proxy_host, proxy_port,
                    proxy_user, proxy_pass);
        } else {
            RestClient_set_proxy((RestClient*) atmos, proxy_host, proxy_port,
                    NULL, NULL);
        }
    }
}

void do_test_whitespace(const char *input, const char *expected_output) {
    char teststr[255];

    strcpy(teststr, input);
    AtmosUtil_normalize_whitespace(teststr);
    assert_string_equal(expected_output, teststr);
}

void test_normalize_whitespace() {
    do_test_whitespace("x-emc-meta:title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta: title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta : title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:  title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:   title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:title=Moutain  Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:title=Moutain   Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:title=Moutain Dew ",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace("x-emc-meta:title=Moutain Dew, type=White Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain Dew,type=White Out",
            "x-emc-meta:title=Moutain Dew,type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain Dew , type=White Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain Dew, type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain  Dew, type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain  Dew , type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title=Moutain   Dew  , type=White  Out  ",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace("x-emc-meta:title= Moutain   Dew  , type= White  Out  ",
            "x-emc-meta:title= Moutain Dew, type= White Out");

    do_test_whitespace(
            "x-emc-meta:title = Moutain   Dew  , type = White  Out  ",
            "x-emc-meta:title = Moutain Dew, type = White Out");
}

void test_sign_string() {
    AtmosClient atmos;

    // Create a dummy client to sign with
    AtmosClient_init(&atmos, "doesntmatter", 80,
            "6039ac182f194e15b9261d73ce044939/user1",
            "LJLuryj6zs8ste6Y3jTGQp71xq0=");

    char *signature = AtmosClient_sign(&atmos, string_to_sign);
    assert_string_equal("WHJo1MFevMnK4jCthJ974L3YHoo=", signature);
    free(signature);

    AtmosClient_destroy(&atmos);
}

void test_sign_request() {
    RestRequest request;
    AtmosClient atmos;

    RestRequest_init(&request, "/rest/objects", HTTP_POST);
    RestRequest_add_header(&request, "Content-Type: application/octet-stream");
    RestRequest_add_header(&request, "Date: Thu, 05 Jun 2008 16:38:19 GMT");
    RestRequest_add_header(&request, "x-emc-date:Thu, 05 Jun 2008 16:38:19 GMT");
    RestRequest_add_header(&request,
            "x-emc-uid:6039ac182f194e15b9261d73ce044939/user1");
    RestRequest_add_header(&request, "x-emc-meta:part1=buy");
    RestRequest_add_header(&request,
            "x-emc-listable-meta:part4/part7/part8=quick");
    RestRequest_add_header(&request,
            "x-emc-useracl:john=FULL_CONTROL,mary=WRITE");
    RestRequest_add_header(&request, "x-emc-groupacl:other=NONE");
    RestRequest_add_header(&request, "Accept: */*"); // This header is not signed.
    RestRequest_add_header(&request, "x-some-other-header: bogus"); // Ditto

    // Create a dummy client to sign with
    AtmosClient_init(&atmos, "doesntmatter", 80,
            "6039ac182f194e15b9261d73ce044939/user1",
            "LJLuryj6zs8ste6Y3jTGQp71xq0=");

    // Test canonicalization.
    char *canonicalized = AtmosClient_canonicalize_request(&atmos, &request);
    assert_string_equal(string_to_sign, canonicalized);
    free(canonicalized);

    // Test signing a request object.
    char *signature = AtmosClient_sign_request(&atmos, &request);
    assert_string_equal("WHJo1MFevMnK4jCthJ974L3YHoo=", signature);
    free(signature);

    AtmosClient_destroy(&atmos);
    RestRequest_destroy(&request);
}

void test_get_service_info() {
    AtmosClient atmos;
    AtmosServiceInfoResponse res;

    get_atmos_client(&atmos);

    AtmosServiceInfoResponse_init(&res);

    AtmosClient_get_service_information(&atmos, &res);

    check_error((AtmosResponse*) &res);
    assert_int_equal(200, res.parent.parent.http_code);
    assert_true(strlen(res.version) > 0);
    printf("Atmos Version %s\n", res.version);

    if (res.version[0] >= 50) { // '2'
        // Atmos >= 2.x should support this
        assert_int_equal(1, res.utf8_metadata_supported);
    }
    sscanf(res.version, "%d.%d.%d", &atmos_major, &atmos_minor, &atmos_sp);

    AtmosServiceInfoResponse_destroy(&res);

    AtmosClient_destroy(&atmos);
}

void test_parse_error() {
    AtmosClient atmos;
    AtmosServiceInfoResponse res;

    get_atmos_client(&atmos);

    // Intentionally change the secret key to produce an error
    strcpy(atmos.secret, "LJLuryj6zs8ste6Y3jTGQp71xq0=");

    AtmosServiceInfoResponse_init(&res);

    AtmosClient_get_service_information(&atmos, &res);

    assert_int_equal(403, res.parent.parent.http_code);
    assert_int_equal(1032, res.parent.atmos_error);
    assert_string_equal("There was a mismatch between the signature "
            "in the request and the signature computed by the server.",
            res.parent.atmos_error_message);
    AtmosServiceInfoResponse_destroy(&res);

    AtmosClient_destroy(&atmos);

}

/**
 * Test the function that generates ACL headers.
 */
void test_acl_generator() {
    AtmosAclEntry acl[4];
    RestRequest request;
    const char *header_value;

    acl[0].type = ATMOS_GROUP;
    strcpy(acl[0].principal, ATMOS_ACL_GROUP_OTHER);
    acl[0].permission = ATMOS_PERM_READ;

    acl[1].type = ATMOS_USER;
    strcpy(acl[1].principal, "alice");
    acl[1].permission = ATMOS_PERM_FULL;

    acl[2].type = ATMOS_USER;
    strcpy(acl[2].principal, "bob");
    acl[2].permission = ATMOS_PERM_READ_WRITE;

    acl[3].type = ATMOS_USER;
    strcpy(acl[3].principal, "charlie");
    acl[3].permission = ATMOS_PERM_NONE;

    RestRequest_init(&request, "http://www.google.com", HTTP_GET);
    AtmosUtil_set_acl_header(acl, 4, &request);

    header_value = RestRequest_get_header_value(&request,
            ATMOS_HEADER_GROUP_ACL);
    assert_string_equal("other=READ", header_value);

    header_value
            = RestRequest_get_header_value(&request, ATMOS_HEADER_USER_ACL);
    assert_string_equal("alice=FULL_CONTROL, bob=WRITE, charlie=NONE",
            header_value);

    RestRequest_destroy(&request);
}

/**
 * Test the function that generates metadata headers.
 */
void test_meta_generator() {
    AtmosMetadata meta[4];
    RestRequest request;
    const char *header_value;

    printf("Ignore any warnings about invalid characters\n");

    strcpy(meta[0].name, "name1");
    strcpy(meta[0].value, "value1");

    strcpy(meta[1].name, "name 2");
    strcpy(meta[1].value, "Value with space");

    strcpy(meta[2].name, "name_3");
    strcpy(meta[2].value, "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><");

    strcpy(meta[3].name, "name4");
    strcpy(meta[3].value, "invalid character test ,=");

    RestRequest_init(&request, "http://www.google.com", HTTP_GET);
    AtmosUtil_set_metadata_header(meta, 4, 0, 0, &request);
    header_value = RestRequest_get_header_value(&request, ATMOS_HEADER_META);
    assert_string_equal("name1=value1,name 2=Value with space,"
            "name_3=character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            header_value);
    AtmosUtil_set_metadata_header(meta, 4, 1, 0, &request);
    header_value = RestRequest_get_header_value(&request,
            ATMOS_HEADER_LISTABLE_META);
    assert_string_equal("name1=value1,name 2=Value with space,"
            "name_3=character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            header_value);
    RestRequest_destroy(&request);
}

/**
 * Test the function that generates metadata headers in UTF-8 mode.
 */
void test_meta_generator_utf8() {
    AtmosMetadata meta[4];
    RestRequest request;
    const char *header_value;

    strcpy(meta[0].name, "cryllic");
    strcpy(meta[0].value, "спасибо");

    strcpy(meta[1].name, "Japanese");
    strcpy(meta[1].value, "どうもありがとう");

    strcpy(meta[2].name, "Composed accents");
    strcpy(meta[2].value, "éêëè");

    strcpy(meta[3].name, "Special Characters");
    strcpy(meta[3].value, "invalid character test ,=");

    RestRequest_init(&request, "http://www.google.com", HTTP_GET);
    AtmosUtil_set_metadata_header(meta, 4, 1, 1, &request);
    header_value = RestRequest_get_header_value(&request,
            ATMOS_HEADER_LISTABLE_META);
    assert_string_equal("cryllic=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE,"
            "Japanese=%E3%81%A9%E3%81%86%E3%82%82%E3%81%82%E3%82%8A%E3%81%8C%E3%81%A8%E3%81%86,"
            "Composed%20accents=%C3%A9%C3%AA%C3%AB%C3%A8,"
            "Special%20Characters=invalid%20character%20test%20%2C%3D",
            header_value);
    RestRequest_destroy(&request);
}

// Test the metadata parser
void test_parse_meta() {
    AtmosMetadata meta[4], listable_meta[4];
    int meta_count = 0, listable_meta_count = 0;

    RestResponse response;

    RestResponse_init(&response);

    RestResponse_add_header(&response, ATMOS_HEADER_META ":"
    "name1=value1,name 2=Value with space,"
    "name_3=character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><");
    RestResponse_add_header(&response, ATMOS_HEADER_LISTABLE_META ":"
    "name1=value1,name 2=Value with space,"
    "name_3=character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><");
    AtmosUtil_parse_user_meta_headers(&response, meta, &meta_count,
            listable_meta, &listable_meta_count);

    assert_int_equal(3, meta_count);
    assert_int_equal(3, listable_meta_count);
    assert_string_equal("name1", meta[0].name);
    assert_string_equal("name 2", meta[1].name);
    assert_string_equal("name_3", meta[2].name);
    assert_string_equal("value1", meta[0].value);
    assert_string_equal("Value with space", meta[1].value);
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", meta[2].value);
    assert_string_equal("name1", listable_meta[0].name);
    assert_string_equal("name 2", listable_meta[1].name);
    assert_string_equal("name_3", listable_meta[2].name);
    assert_string_equal("value1", listable_meta[0].value);
    assert_string_equal("Value with space", listable_meta[1].value);
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", listable_meta[2].value);

    RestResponse_destroy(&response);
}

// Test the metadata parser in UTF-8 mode
void test_parse_meta_utf8() {
    AtmosMetadata meta[4], listable_meta[4];
    int meta_count = 0, listable_meta_count = 0;

    RestResponse response;

    RestResponse_init(&response);

    RestResponse_add_header(
            &response,
            ATMOS_HEADER_META ":"
            "%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE,"
            "Japanese=%E3%81%A9%E3%81%86%E3%82%82%E3%81%82%E3%82%8A%E3%81%8C%E3%81%A8%E3%81%86,"
            "Composed%20accents=%C3%A9%C3%AA%C3%AB%C3%A8,"
            "Special%20Characters=invalid%20character%20test%20%2C%3D");
    RestResponse_add_header(
            &response,
            ATMOS_HEADER_LISTABLE_META ":"
            "%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE,"
            "Japanese=%E3%81%A9%E3%81%86%E3%82%82%E3%81%82%E3%82%8A%E3%81%8C%E3%81%A8%E3%81%86,"
            "Composed%20accents=%C3%A9%C3%AA%C3%AB%C3%A8,"
            "Special%20Characters=invalid%20character%20test%20%2C%3D");
    // Indicate that the response was encoded.
    RestResponse_add_header(&response, ATMOS_HEADER_UTF8 ":true");

    AtmosUtil_parse_user_meta_headers(&response, meta, &meta_count,
            listable_meta, &listable_meta_count);
    assert_int_equal(4, meta_count);
    assert_int_equal(4, listable_meta_count);
    assert_string_equal("русский", meta[0].name);
    assert_string_equal("Japanese", meta[1].name);
    assert_string_equal("Composed accents", meta[2].name);
    assert_string_equal("Special Characters", meta[3].name);
    assert_string_equal("спасибо", meta[0].value);
    assert_string_equal("どうもありがとう", meta[1].value);
    assert_string_equal("éêëè", meta[2].value);
    assert_string_equal("invalid character test ,=", meta[3].value);
    assert_string_equal("русский", listable_meta[0].name);
    assert_string_equal("Japanese", listable_meta[1].name);
    assert_string_equal("Composed accents", listable_meta[2].name);
    assert_string_equal("Special Characters", listable_meta[3].name);
    assert_string_equal("спасибо", listable_meta[0].value);
    assert_string_equal("どうもありがとう", listable_meta[1].value);
    assert_string_equal("éêëè", listable_meta[2].value);
    assert_string_equal("invalid character test ,=", listable_meta[3].value);

    RestResponse_destroy(&response);
}

void test_parse_system_meta() {
    AtmosMetadata meta[4], listable_meta[4];
    int meta_count = 0, listable_meta_count = 0;
    AtmosSystemMetadata system_meta;
    RestResponse response;

    memset(&system_meta, 0, sizeof(AtmosSystemMetadata));

    RestResponse_init(&response);

    RestResponse_add_header(&response, ATMOS_HEADER_META ":"
    "atime=2009-02-18T16:27:24Z, mtime=2009-02-18T16:03:52Z,"
    "ctime=2009-02-18T16:27:24Z, itime=2009-02-18T16:03:52Z,"
    "type=regular, uid=user1, gid=apache,"
    "objectid=499ad542a1a8bc200499ad5a6b05580499c3168560a4, objname=,"
    "size=211, nlink=0, policyname=default,"
    "x-emc-wschecksum=sha0/1037/87hn7kkdd9d982f031qwe9ab224abjd6h1276nj9");

    AtmosUtil_parse_user_meta_headers(&response, meta, &meta_count,
            listable_meta, &listable_meta_count);
    assert_int_equal(0, meta_count);
    assert_int_equal(0, listable_meta_count);

    AtmosUtil_parse_system_meta_header(&response, &system_meta);
    assert_int64t_equal(1234974444, system_meta.atime);
    assert_int64t_equal(1234974444, system_meta.ctime);
    assert_string_equal("apache", system_meta.gid);
    assert_int64t_equal(1234973032, system_meta.itime);
    assert_int64t_equal(1234973032, system_meta.mtime);
    assert_int_equal(0, system_meta.nlink);
    assert_string_equal("499ad542a1a8bc200499ad5a6b05580499c3168560a4", system_meta.object_id);
    assert_string_equal("", system_meta.objname);
    assert_string_equal("default", system_meta.policyname);
    assert_int64t_equal(211, system_meta.size);
    assert_string_equal("regular", system_meta.type);
    assert_string_equal("user1", system_meta.uid);
    assert_string_equal("sha0/1037/87hn7kkdd9d982f031qwe9ab224abjd6h1276nj9", system_meta.wschecksum);

    RestResponse_destroy(&response);
}

void test_parse_system_meta_utf8() {
    AtmosMetadata meta[4], listable_meta[4];
    int meta_count = 0, listable_meta_count = 0;
    AtmosSystemMetadata system_meta;
    RestResponse response;

    memset(&system_meta, 0, sizeof(AtmosSystemMetadata));

    RestResponse_init(&response);

    RestResponse_add_header(&response, ATMOS_HEADER_META ":"
    "atime=2012-01-06T16:16:00Z, mtime=2012-01-06T15:59:28Z,"
    "ctime=2012-01-06T16:16:00Z, itime=2012-01-06T15:59:27Z,"
    "type=regular, uid=user1, gid=apache,"
    "objectid=4ef49feaa106904c04ef4a066e778104f071a5ff0c85,"
    "objname=%cf%85%cf%80%ce%bf%ce%bb%ce%bf%ce%b3%ce%b9%cf%83%cf%84%ce%b7%cc%81.jpg,"
    "size=459, nlink=1, policyname=default");
    RestResponse_add_header(&response, ATMOS_HEADER_UTF8 ": true");

    AtmosUtil_parse_user_meta_headers(&response, meta, &meta_count,
            listable_meta, &listable_meta_count);
    assert_int_equal(0, meta_count);
    assert_int_equal(0, listable_meta_count);

    AtmosUtil_parse_system_meta_header(&response, &system_meta);
    assert_int64t_equal(1325866560, system_meta.atime);
    assert_int64t_equal(1325866560, system_meta.ctime);
    assert_string_equal("apache", system_meta.gid);
    assert_int64t_equal(1325865567, system_meta.itime);
    assert_int64t_equal(1325865568, system_meta.mtime);
    assert_int64t_equal(1, system_meta.nlink);
    assert_string_equal("4ef49feaa106904c04ef4a066e778104f071a5ff0c85", system_meta.object_id);
    assert_string_equal("υπολογιστή.jpg", system_meta.objname);
    assert_string_equal("default", system_meta.policyname);
    assert_int64t_equal(459, system_meta.size);
    assert_string_equal("regular", system_meta.type);
    assert_string_equal("user1", system_meta.uid);
    assert_string_equal("", system_meta.wschecksum);

    RestResponse_destroy(&response);
}

void test_parse_acl() {
    RestResponse response;
    AtmosAclEntry acl[6];
    int acl_count=0;

    RestResponse_init(&response);

    RestResponse_add_header(&response, ATMOS_HEADER_USER_ACL ": "
            "fred=FULL_CONTROL, john=FULL_CONTROL,mary=READ,"
            "user1=WRITE");
    RestResponse_add_header(&response, ATMOS_HEADER_GROUP_ACL ": "
            "other=NONE");

    AtmosUtil_parse_acl_headers(&response, acl, &acl_count);

    assert_int_equal(5, acl_count);
    assert_string_equal("fred", acl[0].principal);
    assert_int_equal(ATMOS_USER, acl[0].type);
    assert_int_equal(ATMOS_PERM_FULL, acl[0].permission);
    assert_string_equal("john", acl[1].principal);
    assert_int_equal(ATMOS_USER, acl[1].type);
    assert_int_equal(ATMOS_PERM_FULL, acl[1].permission);
    assert_string_equal("mary", acl[2].principal);
    assert_int_equal(ATMOS_USER, acl[2].type);
    assert_int_equal(ATMOS_PERM_READ, acl[2].permission);
    assert_string_equal("user1", acl[3].principal);
    assert_int_equal(ATMOS_USER, acl[3].type);
    assert_int_equal(ATMOS_PERM_READ_WRITE, acl[3].permission);
    assert_string_equal("other", acl[4].principal);
    assert_int_equal(ATMOS_GROUP, acl[4].type);
    assert_int_equal(ATMOS_PERM_NONE, acl[4].permission);

    RestResponse_destroy(&response);
}

void test_parse_object_info() {
    AtmosGetObjectInfoResponse info;
    const char *xml =
            "<?xml version='1.0' encoding='UTF-8'?>\n"
            "<GetObjectInfoResponse xmlns='http://www.emc.com/cos/'>\n"
            "    <objectId>4b00fffea12059c104b00ffca1f8e804b040c4d911c9</objectId>\n"
            "    <selection>geographic</selection>\n"
            "    <numReplicas>2</numReplicas>\n"
            "    <replicas>\n"
            "        <replica>\n"
            "            <id>3</id>\n"
            "            <type>sync</type>\n"
            "            <current>true</current>\n"
            "            <location>Boston</location>\n"
            "            <storageType>Normal</storageType>\n"
            "        </replica>\n"
            "        <replica>\n"
            "            <id>5</id>\n"
            "            <type>async</type>\n"
            "            <current>false</current>\n"
            "            <location>New York</location>\n"
            "            <storageType>Normal</storageType>\n"
            "        </replica>\n"
            "    </replicas>\n"
            "    <retention>\n"
            "        <enabled>false</enabled>\n"
            "        <endAt></endAt>\n"
            "    </retention>\n"
            "    <expiration>\n"
            "        <enabled>true</enabled>\n"
            "        <endAt>1970-01-01T00:00:05Z</endAt>\n"
            "    </expiration>\n"
            "</GetObjectInfoResponse>\n";

    AtmosGetObjectInfoResponse_init(&info);

    AtmosGetObjectInfoResponse_parse(&info, xml, strlen(xml));

    assert_string_equal("4b00fffea12059c104b00ffca1f8e804b040c4d911c9", info.object_id);
    assert_string_equal("geographic", info.selection);
    assert_int_equal(2, info.replica_count);
    assert_int_equal(3, info.replicas[0].id);
    assert_string_equal("sync", info.replicas[0].type);
    assert_int_equal(1, info.replicas[0].current);
    assert_string_equal("Boston", info.replicas[0].location);
    assert_string_equal("Normal", info.replicas[0].storage_type);
    assert_int_equal(5, info.replicas[1].id);
    assert_string_equal("async", info.replicas[1].type);
    assert_int_equal(0, info.replicas[1].current);
    assert_string_equal("New York", info.replicas[1].location);
    assert_string_equal("Normal", info.replicas[1].storage_type);
    assert_int_equal(0, info.retention_enabled);
    assert_int64t_equal(0, info.retention_end);
    assert_int_equal(1, info.expiration_enabled);
    assert_int64t_equal(5, info.expiration_end);

    AtmosGetObjectInfoResponse_destroy(&info);
}

void test_parse_dirlist() {
    char *xml;
    FILE *f;
    size_t size;
    AtmosListDirectoryResponse dir;

    f = fopen("dirlist.xml", "r");
    if(!f) {
        assert_fail("Missing dirlist.xml");
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    xml = malloc(size);

    if(!fread(xml, size, 1, f)) {
        assert_fail("Failed to read dirlist.xml");
        return;
    }


    AtmosListDirectoryResponse_init(&dir);
    AtmosListDirectoryResponse_parse(&dir, xml, size);

    // Check
    assert_int_equal(62, dir.entry_count);

    assert_string_equal("4ee696e4a11f549604f0b753939ef204f7b5ea28199f",
            dir.entries[1].object_id);
    assert_string_equal("regular", dir.entries[1].type);
    assert_string_equal("AtmosSync.jar", dir.entries[1].filename);
    assert_int64t_equal(1333485224, dir.entries[1].parent.system_metadata.atime);
    assert_int64t_equal(1333485223, dir.entries[1].parent.system_metadata.ctime);
    assert_string_equal("apache", dir.entries[1].parent.system_metadata.gid);
    assert_int64t_equal(1333485218, dir.entries[1].parent.system_metadata.itime);
    assert_int64t_equal(1333485223, dir.entries[1].parent.system_metadata.mtime);
    assert_int_equal(1, dir.entries[1].parent.system_metadata.nlink);
    assert_string_equal("4ee696e4a11f549604f0b753939ef204f7b5ea28199f", dir.entries[1].parent.system_metadata.object_id);
    assert_string_equal("AtmosSync.jar", dir.entries[1].parent.system_metadata.objname);
    assert_string_equal("default", dir.entries[1].parent.system_metadata.policyname);
    assert_int64t_equal(2236828, dir.entries[1].parent.system_metadata.size);
    assert_string_equal("regular", dir.entries[1].parent.system_metadata.type);
    assert_string_equal("A130672722730429efbb", dir.entries[1].parent.system_metadata.uid);
    assert_string_equal("", dir.entries[1].parent.system_metadata.wschecksum);

    // User metadata on entries 22 and 23
    assert_string_equal("ec", dir.entries[22].parent.meta[0].name);
    assert_string_equal("true", dir.entries[22].parent.meta[0].value);
    assert_string_equal("ec", dir.entries[23].parent.meta[0].name);
    assert_string_equal("true", dir.entries[23].parent.meta[0].value);
    assert_string_equal("project_x", dir.entries[23].parent.listable_meta[0].name);
    assert_string_equal("", dir.entries[23].parent.listable_meta[0].value);

    AtmosListDirectoryResponse_destroy(&dir);
    free(xml);
    fclose(f);
}

void test_create_object() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_create_object_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    //atmos.signature_debug = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain",
            &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_create_object_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);
    //atmos.signature_debug = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key,
            "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


/**
 * Test creating an object with metadata.
 */
void test_create_object_meta() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

/**
 * Test creating an object with UTF8 metadata.
 */
void test_create_object_meta_utf8() {
    AtmosClient atmos;
    AtmosServiceInfoResponse service_info;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;

    get_atmos_client(&atmos);
    AtmosServiceInfoResponse_init(&service_info);
    AtmosClient_get_service_information(&atmos, &service_info);
    if (!service_info.utf8_metadata_supported) {
        printf("...skipped (UTF8 metadata not supported)\n");
        AtmosServiceInfoResponse_destroy(&service_info);
        AtmosClient_destroy(&atmos);
        return;
    }
    AtmosServiceInfoResponse_destroy(&service_info);

    // Turn on UTF-8 metadata handling
    atmos.enable_utf8_metadata = 1;

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    AtmosCreateObjectRequest_add_metadata(&request, "русский", "спасибо", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "日本語", "どうもありがとう", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "Composed accents", "éêëè",
            0);
    AtmosCreateObjectRequest_add_metadata(&request, "Special Characters",
            "invalid character test ,=", 0);

    AtmosClient_create_object(&atmos, &request, &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

/**
 * Test creating an object with an ACL
 */
void test_create_object_acl() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // Build an ACL
    AtmosCreateObjectRequest_add_acl(&request, uid1, ATMOS_USER,
            ATMOS_PERM_FULL);
    AtmosCreateObjectRequest_add_acl(&request, uid2, ATMOS_USER,
            ATMOS_PERM_READ_WRITE);
    AtmosCreateObjectRequest_add_acl(&request, ATMOS_ACL_GROUP_OTHER,
            ATMOS_GROUP, ATMOS_PERM_NONE);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_read_object() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain; charset=US-ASCII", &response);

    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Read back.
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(4, read_response.parent.parent.content_length);
    assert_string_equal("text/plain; charset=US-ASCII", read_response.parent.parent.content_type);
    assert_string_equal("test", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_read_object_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    //atmos.signature_debug = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain; charset=US-ASCII",
            &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_ns(&atmos, path, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(4, read_response.parent.parent.content_length);
    assert_string_equal("text/plain; charset=US-ASCII", read_response.parent.parent.content_type);
    assert_string_equal("test", read_response.parent.parent.body);
    // Look for the ObjectID
    assert_string_equal(response.object_id,
            read_response.system_metadata.object_id);
    AtmosReadObjectResponse_destroy(&read_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_read_object_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);
    //atmos.signature_debug = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "test", 4, "text/plain; charset=US-ASCII",
            &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_keypool(&atmos, TEST_KEYPOOL, key, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(4, read_response.parent.parent.content_length);
    assert_string_equal("text/plain; charset=US-ASCII", read_response.parent.parent.content_type);
    assert_string_equal("test", read_response.parent.parent.body);
    // Look for the ObjectID
    assert_string_equal(response.object_id,
            read_response.system_metadata.object_id);
    AtmosReadObjectResponse_destroy(&read_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


void test_read_object_with_meta_and_acl() {
    AtmosClient atmos;
    AtmosServiceInfoResponse service_info;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosReadObjectResponse read_response;
    int utf8;

    get_atmos_client(&atmos);
    AtmosServiceInfoResponse_init(&service_info);
    AtmosClient_get_service_information(&atmos, &service_info);
    if (!service_info.utf8_metadata_supported) {
        printf("... skipping UTF8 tests\n");
        utf8=0;
    } else {
        utf8=1;
        atmos.enable_utf8_metadata=1;
    }
    AtmosServiceInfoResponse_destroy(&service_info);

    // Turn on UTF-8 metadata handling
    if(utf8) {
        atmos.enable_utf8_metadata = 1;
    }

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // Put a bunch of metadata on the object.
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);
    if(utf8) {
        // If UTF-8 is available, test that too.
        AtmosCreateObjectRequest_add_metadata(&request, "русский", "спасибо", 0);
        AtmosCreateObjectRequest_add_metadata(&request, "日本語", "どうもありがとう", 0);
        AtmosCreateObjectRequest_add_metadata(&request, "Composed accents", "éêëè",
                0);
        AtmosCreateObjectRequest_add_metadata(&request, "Special Characters",
                "invalid\ncharacter test ,=AAA", 0);
    }

    // Put an ACL on the object.
    AtmosCreateObjectRequest_add_acl(&request, uid1, ATMOS_USER,
            ATMOS_PERM_FULL);
    AtmosCreateObjectRequest_add_acl(&request, uid2, ATMOS_USER,
            ATMOS_PERM_READ_WRITE);
    AtmosCreateObjectRequest_add_acl(&request, ATMOS_ACL_GROUP_OTHER,
            ATMOS_GROUP, ATMOS_PERM_NONE);

    // Set the object body content
    RestRequest_set_array_body((RestRequest*)&request, "test", 4, "text/plain; charset=iso-8859-1");

    // Create
    AtmosClient_create_object(&atmos, &request, &response);

    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Read it back.
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);

    // Check
    assert_string_equal("Value  with   spaces",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "meta2", 0));
    assert_string_equal("value",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "name  with   spaces", 0));
    assert_string_equal("",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "empty value", 0));
    assert_string_equal("value1",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "listable1", 1));
    assert_string_equal("",
            AtmosReadObjectResponse_get_metadata_value(&read_response,
                    "listable2", 1));
    if(utf8) {
        assert_string_equal("спасибо",
                AtmosReadObjectResponse_get_metadata_value(&read_response,
                        "русский", 0));
        assert_string_equal("どうもありがとう",
                AtmosReadObjectResponse_get_metadata_value(&read_response,
                        "日本語", 0));
        assert_string_equal("éêëè",
                AtmosReadObjectResponse_get_metadata_value(&read_response,
                        "Composed accents", 0));
        assert_string_equal("invalid\ncharacter test ,=AAA",
                AtmosReadObjectResponse_get_metadata_value(&read_response,
                        "Special Characters", 0));
    }
    assert_int64t_equal(4, read_response.parent.parent.content_length);
    assert_string_equal("text/plain; charset=iso-8859-1", read_response.parent.parent.content_type);
    assert_string_equal("test", read_response.parent.parent.body);

    // Check system metadata
    assert_true(0 < read_response.system_metadata.atime);
    assert_true(0 < read_response.system_metadata.ctime);
    assert_string_equal("apache", read_response.system_metadata.gid);
    assert_true(0 < read_response.system_metadata.itime);
    assert_true(0 < read_response.system_metadata.mtime);
    assert_true(0 == read_response.system_metadata.nlink);
    assert_string_equal(response.object_id, read_response.system_metadata.object_id);
    assert_string_equal("", read_response.system_metadata.objname);
    assert_string_equal("default", read_response.system_metadata.policyname);
    assert_int64t_equal(4, read_response.system_metadata.size);
    assert_string_equal("regular", read_response.system_metadata.type);
    assert_string_equal(uid1, read_response.system_metadata.uid);

    // Check ACL
    assert_int_equal(ATMOS_PERM_FULL,
            AtmosReadObjectResponse_get_acl_permission(&read_response,
                    uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ_WRITE,
            AtmosReadObjectResponse_get_acl_permission(&read_response,
                    uid2, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_NONE,
            AtmosReadObjectResponse_get_acl_permission(&read_response,
                    ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    // This one doesn't exist and should be 'None'
    assert_int_equal(ATMOS_PERM_NONE,
            AtmosReadObjectResponse_get_acl_permission(&read_response,
                    "some_user_that_does_not_exist", ATMOS_USER));

    AtmosReadObjectResponse_destroy(&read_response);

    // Delete it
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosCreateObjectRequest_destroy(&request);
    AtmosClient_destroy(&atmos);
}

void test_read_object_range() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosReadObjectResponse read_response;
    AtmosReadObjectRequest read_request;
    RestResponse delete_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "0123456789", 10, "text/plain", &response);

    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Read back.

    // 0-0 should return "0"
    AtmosReadObjectRequest_init(&read_request, response.object_id);
    AtmosReadObjectResponse_init(&read_response);
    AtmosReadObjectRequest_set_range(&read_request, 0, 0);
    AtmosClient_read_object(&atmos, &read_request, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(206, read_response.parent.parent.http_code);
    assert_string_equal("0", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);
    AtmosReadObjectRequest_destroy(&read_request);

    // 0- should return everything
    AtmosReadObjectRequest_init(&read_request, response.object_id);
    AtmosReadObjectResponse_init(&read_response);
    AtmosReadObjectRequest_set_range(&read_request, 0, -1);
    AtmosClient_read_object(&atmos, &read_request, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(206, read_response.parent.parent.http_code);
    assert_string_equal("0123456789", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);
    AtmosReadObjectRequest_destroy(&read_request);

    // -1 should return "9"
    AtmosReadObjectRequest_init(&read_request, response.object_id);
    AtmosReadObjectResponse_init(&read_response);
    AtmosReadObjectRequest_set_range(&read_request, -1, 1);
    AtmosClient_read_object(&atmos, &read_request, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(206, read_response.parent.parent.http_code);
    assert_string_equal("9", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);
    AtmosReadObjectRequest_destroy(&read_request);

    // 5-7 should return "567"
    AtmosReadObjectRequest_init(&read_request, response.object_id);
    AtmosReadObjectResponse_init(&read_response);
    AtmosReadObjectRequest_set_range(&read_request, 5, 7);
    AtmosClient_read_object(&atmos, &read_request, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(206, read_response.parent.parent.http_code);
    assert_string_equal("567", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);
    AtmosReadObjectRequest_destroy(&read_request);

    // 5 with length 3 should return "567"
    AtmosReadObjectRequest_init(&read_request, response.object_id);
    AtmosReadObjectResponse_init(&read_response);
    AtmosReadObjectRequest_set_range_offset_size(&read_request, 5, 3);
    AtmosClient_read_object(&atmos, &read_request, &read_response);

    check_error((AtmosResponse*)&read_response);
    assert_int_equal(206, read_response.parent.parent.http_code);
    assert_string_equal("567", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);
    AtmosReadObjectRequest_destroy(&read_request);

    // Delete
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_read_object_file() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;
    FILE *f1;
    FILE *f2;
    char buffer[256];
    char buffer2[256];
    char buffer3[256];
    int i, j;

    // Build input data.
    f1 = tmpfile();
    f2 = tmpfile();

    for(i=0; i<256; i++) {
        buffer[i] = (char)i;
    }

    // Write 2MB of data
    for(i=0; i<8192; i++) {
        fwrite(buffer, 256, 1, f1);
    }

    // rewind
    fseek(f1, 0, SEEK_SET);

    // Create the object.
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);
    RestRequest_set_file_body((RestRequest*)&request, f1, 1024*1024*2,
            "application/octet-stream");

    AtmosClient_create_object(&atmos, &request, &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Read it back into f2.
    AtmosReadObjectResponse_init(&read_response);
    RestResponse_use_file((RestResponse*)&read_response, f2);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(1024*1024*2, read_response.parent.parent.content_length);
    assert_int64t_equal(1024*1024*2, ftell(f2));

    // Check data in f2.
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);

    for(i=0; i<8192; i++) {
        if(!fread(buffer, 256, 1, f1)) {
            // ignore
        }
        if(!fread(buffer2, 256, 1, f2)) {
            // ignore
        }

        for(j=0; j<256; j++) {
            if(buffer[j] != buffer2[j]) {
                sprintf(buffer3, "Failure at offset %d (%d != %d)", i*256+j, buffer[j], buffer2[j]);
                assert_fail(buffer3)
                i=8192;
                break;
            }
        }
    }
    AtmosReadObjectResponse_destroy(&read_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
    fclose(f1);
    fclose(f2);
}

void test_get_user_meta() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaRequest meta_request;
    AtmosGetUserMetaResponse meta_response;

    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple(&atmos, response.object_id, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal("value1",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Test advanced
    AtmosGetUserMetaRequest_init(&meta_request, response.object_id);
    AtmosGetUserMetaRequest_add_tag(&meta_request, "meta1");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "name  with   spaces");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "listable2");
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);
    AtmosGetUserMetaRequest_destroy(&meta_request);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_get_user_meta_ns() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaRequest meta_request;
    AtmosGetUserMetaResponse meta_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];
    RestResponse delete_response;

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_ns(&request, path);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_ns(&atmos, path, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal("value1",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Test advanced
    AtmosGetUserMetaRequest_init_ns(&meta_request, path);
    AtmosGetUserMetaRequest_add_tag(&meta_request, "meta1");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "name  with   spaces");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "listable2");
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);
    AtmosGetUserMetaRequest_destroy(&meta_request);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_get_user_meta_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaRequest meta_request;
    AtmosGetUserMetaResponse meta_response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_keypool(&request, TEST_KEYPOOL, key);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_keypool(&atmos, TEST_KEYPOOL, key, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal("value1",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Test advanced
    AtmosGetUserMetaRequest_init_keypool(&meta_request, TEST_KEYPOOL, key);
    AtmosGetUserMetaRequest_add_tag(&meta_request, "meta1");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "name  with   spaces");
    AtmosGetUserMetaRequest_add_tag(&meta_request, "listable2");
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);
    AtmosGetUserMetaRequest_destroy(&meta_request);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
    free(key);
}

void test_delete_user_meta() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;

    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Delete some items
    RestResponse_init(&delete_response);
    const char *names[3] = {"meta2", "empty value", "listable1"};
    AtmosClient_delete_user_meta(&atmos, response.object_id, names, 3,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple(&atmos, response.object_id, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_delete_user_meta_ns() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];
    RestResponse delete_response;

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_ns(&request, path);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Delete some items
    RestResponse_init(&delete_response);
    const char *names[3] = {"meta2", "empty value", "listable1"};
    AtmosClient_delete_user_meta_ns(&atmos, path, names, 3,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_ns(&atmos, path, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_delete_user_meta_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_keypool(&request, TEST_KEYPOOL, key);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Delete some items
    RestResponse_init(&delete_response);
    const char *names[3] = {"meta2", "empty value", "listable1"};
    AtmosClient_delete_user_meta_keypool(&atmos, TEST_KEYPOOL, key, names, 3,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Test basic
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_keypool(&atmos, TEST_KEYPOOL, key, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("Value  with   spaces",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "empty value", 0));
    assert_string_equal(NULL,
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable1", 1));
    assert_string_equal("",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "listable2", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
    free(key);
}


void test_set_user_meta() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;
    AtmosSetUserMetaRequest set_request;
    AtmosResponse set_response;

    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Set some items
    AtmosSetUserMetaRequest_init(&set_request, response.object_id);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "meta1", "changed value", 0);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "new meta", "new value", 1);
    AtmosResponse_init(&set_response);

    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
    check_error(&set_response);
    assert_int_equal(200, set_response.parent.http_code);

    AtmosSetUserMetaRequest_destroy(&set_request);
    AtmosResponse_destroy(&set_response);

    // Read back and check.
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple(&atmos, response.object_id, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("changed value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("new value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "new meta", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}

void test_set_user_meta_ns() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;
    AtmosSetUserMetaRequest set_request;
    AtmosResponse set_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];
    RestResponse delete_response;

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_ns(&request, path);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Set some items
    AtmosSetUserMetaRequest_init_ns(&set_request, path);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "meta1", "changed value", 0);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "new meta", "new value", 1);
    AtmosResponse_init(&set_response);

    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
    check_error(&set_response);
    assert_int_equal(200, set_response.parent.http_code);

    AtmosSetUserMetaRequest_destroy(&set_request);
    AtmosResponse_destroy(&set_response);

    // Read back and check.
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_ns(&atmos, path, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("changed value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("new value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "new meta", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);

}

void test_set_user_meta_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetUserMetaResponse meta_response;
    AtmosSetUserMetaRequest set_request;
    AtmosResponse set_response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init_keypool(&request, TEST_KEYPOOL, key);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // Set some items
    AtmosSetUserMetaRequest_init_keypool(&set_request, TEST_KEYPOOL, key);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "meta1", "changed value", 0);
    AtmosSetUserMetaRequest_add_metadata(&set_request, "new meta", "new value", 1);
    AtmosResponse_init(&set_response);

    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
    check_error(&set_response);
    assert_int_equal(200, set_response.parent.http_code);

    AtmosSetUserMetaRequest_destroy(&set_request);
    AtmosResponse_destroy(&set_response);

    // Read back and check.
    AtmosGetUserMetaResponse_init(&meta_response);
    AtmosClient_get_user_meta_simple_keypool(&atmos, TEST_KEYPOOL, key, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_string_equal("changed value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "meta2", 0));
    assert_string_equal("new value",
            AtmosGetUserMetaResponse_get_metadata_value(&meta_response, "new meta", 1));

    AtmosGetUserMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
    free(key);
}


void test_get_system_meta() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosGetSystemMetaRequest meta_request;
    AtmosGetSystemMetaResponse meta_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Simple version
    AtmosGetSystemMetaResponse_init(&meta_response);
    AtmosClient_get_system_meta_simple(&atmos, response.object_id, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_true(0 < meta_response.system_metadata.atime);
    assert_true(0 < meta_response.system_metadata.ctime);
    assert_string_equal("apache", meta_response.system_metadata.gid);
    assert_true(0 < meta_response.system_metadata.itime);
    assert_true(0 < meta_response.system_metadata.mtime);
    assert_true(0 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal("", meta_response.system_metadata.objname);
    assert_string_equal("default", meta_response.system_metadata.policyname);
    assert_int64t_equal(4, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal(uid1, meta_response.system_metadata.uid);

    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Advanced version.  Only load type, object_id, and objname
    AtmosGetSystemMetaRequest_init(&meta_request, response.object_id);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_TYPE);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJECTID);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJNAME);
    AtmosGetSystemMetaResponse_init(&meta_response);

    AtmosClient_get_system_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);

    assert_int_equal(200, meta_response.parent.parent.http_code);
    assert_true(0 == meta_response.system_metadata.atime);
    assert_true(0 == meta_response.system_metadata.ctime);
    assert_string_equal("", meta_response.system_metadata.gid);
    assert_true(0 == meta_response.system_metadata.itime);
    assert_true(0 == meta_response.system_metadata.mtime);
    assert_true(0 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal("", meta_response.system_metadata.objname);
    assert_string_equal("", meta_response.system_metadata.policyname);
    assert_int64t_equal(0, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal("", meta_response.system_metadata.uid);

    AtmosGetSystemMetaRequest_destroy(&meta_request);
    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_get_system_meta_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosGetSystemMetaRequest meta_request;
    AtmosGetSystemMetaResponse meta_response;
    char path[ATMOS_PATH_MAX];
    char randfile[13];
    RestResponse delete_response;

    random_file(randfile, 8);
    strcat(randfile, ".txt");
    sprintf(path, "/atmos-c-unittest/%s", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Simple version
    AtmosGetSystemMetaResponse_init(&meta_response);
    AtmosClient_get_system_meta_simple_ns(&atmos, path, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_true(0 < meta_response.system_metadata.atime);
    assert_true(0 < meta_response.system_metadata.ctime);
    assert_string_equal("apache", meta_response.system_metadata.gid);
    assert_true(0 < meta_response.system_metadata.itime);
    assert_true(0 < meta_response.system_metadata.mtime);
    assert_true(1 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal(randfile, meta_response.system_metadata.objname);
    assert_string_equal("default", meta_response.system_metadata.policyname);
    assert_int64t_equal(4, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal(uid1, meta_response.system_metadata.uid);

    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Advanced version.  Only load type, object_id, and objname
    AtmosGetSystemMetaRequest_init_ns(&meta_request, path);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_TYPE);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJECTID);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJNAME);
    AtmosGetSystemMetaResponse_init(&meta_response);

    AtmosClient_get_system_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);

    assert_int_equal(200, meta_response.parent.parent.http_code);
    assert_true(0 == meta_response.system_metadata.atime);
    assert_true(0 == meta_response.system_metadata.ctime);
    assert_string_equal("", meta_response.system_metadata.gid);
    assert_true(0 == meta_response.system_metadata.itime);
    assert_true(0 == meta_response.system_metadata.mtime);
    assert_true(0 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal(randfile, meta_response.system_metadata.objname);
    assert_string_equal("", meta_response.system_metadata.policyname);
    assert_int64t_equal(0, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal("", meta_response.system_metadata.uid);

    AtmosGetSystemMetaRequest_destroy(&meta_request);
    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_get_system_meta_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosGetSystemMetaRequest meta_request;
    AtmosGetSystemMetaResponse meta_response;
    RestResponse delete_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);
    atmos.enable_utf8_metadata = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Simple version
    AtmosGetSystemMetaResponse_init(&meta_response);
    AtmosClient_get_system_meta_simple_keypool(&atmos, TEST_KEYPOOL, key, &meta_response);
    check_error((AtmosResponse*)&meta_response);
    assert_int_equal(200, meta_response.parent.parent.http_code);

    assert_true(0 < meta_response.system_metadata.atime);
    assert_true(0 < meta_response.system_metadata.ctime);
    assert_string_equal("apache", meta_response.system_metadata.gid);
    assert_true(0 < meta_response.system_metadata.itime);
    assert_true(0 < meta_response.system_metadata.mtime);
    assert_true(1 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal(key, meta_response.system_metadata.objname);
    assert_string_equal("default", meta_response.system_metadata.policyname);
    assert_int64t_equal(4, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal(uid1, meta_response.system_metadata.uid);

    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Advanced version.  Only load type, object_id, and objname
    AtmosGetSystemMetaRequest_init_keypool(&meta_request, TEST_KEYPOOL, key);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_TYPE);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJECTID);
    AtmosGetSystemMetaRequest_add_tag(&meta_request, ATMOS_SYSTEM_META_OBJNAME);
    AtmosGetSystemMetaResponse_init(&meta_response);

    AtmosClient_get_system_meta(&atmos, &meta_request, &meta_response);
    check_error((AtmosResponse*)&meta_response);

    assert_int_equal(200, meta_response.parent.parent.http_code);
    assert_true(0 == meta_response.system_metadata.atime);
    assert_true(0 == meta_response.system_metadata.ctime);
    assert_string_equal("", meta_response.system_metadata.gid);
    assert_true(0 == meta_response.system_metadata.itime);
    assert_true(0 == meta_response.system_metadata.mtime);
    assert_true(0 == meta_response.system_metadata.nlink);
    assert_string_equal(response.object_id, meta_response.system_metadata.object_id);
    assert_string_equal(key, meta_response.system_metadata.objname);
    assert_string_equal("", meta_response.system_metadata.policyname);
    assert_int64t_equal(0, meta_response.system_metadata.size);
    assert_string_equal("regular", meta_response.system_metadata.type);
    assert_string_equal("", meta_response.system_metadata.uid);

    AtmosGetSystemMetaRequest_destroy(&meta_request);
    AtmosGetSystemMetaResponse_destroy(&meta_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


void test_set_get_acl() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosResponse set_acl_response;
    AtmosGetAclResponse acl_response;
    AtmosAclEntry acl[3];

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Read ACL.  Should be default (uid=FULL,other=NONE).
    AtmosGetAclResponse_init(&acl_response);
    AtmosClient_get_acl(&atmos, response.object_id, &acl_response);
    check_error((AtmosResponse*)&acl_response);
    assert_int_equal(200, acl_response.parent.parent.http_code);
    assert_int_equal(2, acl_response.acl_count);
    assert_int_equal(ATMOS_PERM_FULL, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, "dummy_user_that_does_not_exist", ATMOS_USER));
    AtmosGetAclResponse_destroy(&acl_response);

    // Build a new ACL and set it on the object.
    acl[0].permission = ATMOS_PERM_FULL;
    acl[0].type = ATMOS_USER;
    strcpy(acl[0].principal, uid1);
    acl[1].permission = ATMOS_PERM_READ_WRITE;
    acl[1].type = ATMOS_USER;
    strcpy(acl[1].principal, uid2);
    acl[2].permission = ATMOS_PERM_READ;
    acl[2].type = ATMOS_GROUP;
    strcpy(acl[2].principal, ATMOS_ACL_GROUP_OTHER);
    AtmosResponse_init(&set_acl_response);
    AtmosClient_set_acl(&atmos, response.object_id, acl, 3, &set_acl_response);
    check_error(&set_acl_response);
    assert_int_equal(200, set_acl_response.parent.http_code);
    AtmosResponse_destroy(&set_acl_response);

    // Check new ACL
    AtmosGetAclResponse_init(&acl_response);
    AtmosClient_get_acl(&atmos, response.object_id, &acl_response);
    check_error((AtmosResponse*)&acl_response);
    assert_int_equal(200, acl_response.parent.parent.http_code);
    assert_int_equal(3, acl_response.acl_count);
    assert_int_equal(ATMOS_PERM_FULL, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ_WRITE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid2, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ, AtmosGetAclResponse_get_acl_permission(
            &acl_response, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, "dummy_user_that_does_not_exist", ATMOS_USER));
    AtmosGetAclResponse_destroy(&acl_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_set_get_acl_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosResponse set_acl_response;
    AtmosGetAclResponse acl_response;
    AtmosAclEntry acl[3];
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4,
            "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Read ACL.  Should be default (uid=FULL).
    AtmosGetAclResponse_init(&acl_response);
    AtmosClient_get_acl_ns(&atmos, path, &acl_response);
    check_error((AtmosResponse*)&acl_response);
    assert_int_equal(200, acl_response.parent.parent.http_code);
    assert_int_equal(2, acl_response.acl_count);
    assert_int_equal(ATMOS_PERM_FULL, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, "dummy_user_that_does_not_exist", ATMOS_USER));
    AtmosGetAclResponse_destroy(&acl_response);

    // Build a new ACL and set it on the object.
    acl[0].permission = ATMOS_PERM_FULL;
    acl[0].type = ATMOS_USER;
    strcpy(acl[0].principal, uid1);
    acl[1].permission = ATMOS_PERM_READ_WRITE;
    acl[1].type = ATMOS_USER;
    strcpy(acl[1].principal, uid2);
    acl[2].permission = ATMOS_PERM_READ;
    acl[2].type = ATMOS_GROUP;
    strcpy(acl[2].principal, ATMOS_ACL_GROUP_OTHER);
    AtmosResponse_init(&set_acl_response);
    AtmosClient_set_acl_ns(&atmos, path, acl, 3, &set_acl_response);
    check_error(&set_acl_response);
    assert_int_equal(200, set_acl_response.parent.http_code);
    AtmosResponse_destroy(&set_acl_response);

    // Check new ACL
    AtmosGetAclResponse_init(&acl_response);
    AtmosClient_get_acl_ns(&atmos, path, &acl_response);
    check_error((AtmosResponse*)&acl_response);
    assert_int_equal(200, acl_response.parent.parent.http_code);
    assert_int_equal(3, acl_response.acl_count);
    assert_int_equal(ATMOS_PERM_FULL, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ_WRITE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, uid2, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ, AtmosGetAclResponse_get_acl_permission(
            &acl_response, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    assert_int_equal(ATMOS_PERM_NONE, AtmosGetAclResponse_get_acl_permission(
            &acl_response, "dummy_user_that_does_not_exist", ATMOS_USER));
    AtmosGetAclResponse_destroy(&acl_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_update_object() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it.
    RestResponse_init(&update_response);
    AtmosClient_update_object_simple(&atmos, response.object_id, "hello", 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_update_object_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it.
    RestResponse_init(&update_response);
    AtmosClient_update_object_simple_ns(&atmos, path, "hello", 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_ns(&atmos, path, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_update_object_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it.
    RestResponse_init(&update_response);
    AtmosClient_update_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "hello", 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_keypool(&atmos, TEST_KEYPOOL, key, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


void test_update_object_file() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    FILE *f;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it with a file
    f = tmpfile();
    fwrite("hello", 5, 1, f);
    fseek(f, 0, SEEK_SET);
    RestResponse_init(&update_response);
    AtmosClient_update_object_file(&atmos, response.object_id, f, 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    fclose(f);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_update_object_file_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    FILE *f;
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it with a file
    f = tmpfile();
    fwrite("hello", 5, 1, f);
    fseek(f, 0, SEEK_SET);
    RestResponse_init(&update_response);
    AtmosClient_update_object_file_ns(&atmos, path, f, 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    fclose(f);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_ns(&atmos, path, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_update_object_file_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    FILE *f;
    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);
    char *key = random_key();

    printf("Creating key: %s\n", key);

    // Create the object
    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it with a file
    f = tmpfile();
    fwrite("hello", 5, 1, f);
    fseek(f, 0, SEEK_SET);
    RestResponse_init(&update_response);
    AtmosClient_update_object_file_keypool(&atmos, TEST_KEYPOOL, key, f, 5,
            "text/plain;charset=utf8", &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    fclose(f);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_keypool(&atmos, TEST_KEYPOOL, key, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


void test_update_object_range() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    AtmosUpdateObjectRequest update_request;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Create the object
    AtmosClient_create_object_simple(&atmos, "roll the dice", 13, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Update it.
    AtmosUpdateObjectRequest_init(&update_request, response.object_id);
    RestRequest_set_array_body((RestRequest*)&update_request, "two", 3, "text/plain");
    AtmosUpdateObjectRequest_set_range(&update_request, 5, 7);
    RestResponse_init(&update_response);
    AtmosClient_update_object(&atmos, &update_request, &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    AtmosUpdateObjectRequest_destroy(&update_request);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(13, read_response.parent.parent.content_length);
    assert_string_equal("roll two dice", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Append
    AtmosUpdateObjectRequest_init(&update_request, response.object_id);
    RestRequest_set_array_body((RestRequest*)&update_request, " twice", 6, "text/plain");
    AtmosUpdateObjectRequest_set_range_offset_size(&update_request, 13, 6);
    RestResponse_init(&update_response);
    AtmosClient_update_object(&atmos, &update_request, &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    AtmosUpdateObjectRequest_destroy(&update_request);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(19, read_response.parent.parent.content_length);
    assert_string_equal("roll two dice twice", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_update_object_range_file() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    AtmosUpdateObjectRequest update_request;
    FILE *f;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    // Common use case -- create an object and append to it from the same
    // file handle to simulate a 'chunked' upload.
    f = tmpfile();
    fwrite("roll two dice twice", 19, 1, f);
    fseek(f, 0, SEEK_SET);
    AtmosClient_create_object_file(&atmos, f, 13, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Append to it.
    AtmosUpdateObjectRequest_init(&update_request, response.object_id);
    RestRequest_set_file_body((RestRequest*)&update_request, f, 6, "text/plain");
    AtmosUpdateObjectRequest_set_range_offset_size(&update_request, 13, 6);
    RestResponse_init(&update_response);
    AtmosClient_update_object(&atmos, &update_request, &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    AtmosUpdateObjectRequest_destroy(&update_request);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(19, read_response.parent.parent.content_length);
    assert_string_equal("roll two dice twice", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_update_object_meta_acl() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    RestResponse update_response;
    AtmosReadObjectResponse read_response;
    AtmosUpdateObjectRequest update_request;

    get_atmos_client(&atmos);
    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // some test strings
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);
    AtmosCreateObjectRequest_destroy(&request);

    // Update it.
    AtmosUpdateObjectRequest_init(&update_request, response.object_id);
    RestRequest_set_array_body((RestRequest*)&update_request, "hello", 5,
            "text/plain;charset=utf8");
    RestResponse_init(&update_response);
    AtmosUpdateObjectRequest_add_metadata(&update_request, "meta1", "changed value", 0);
    AtmosUpdateObjectRequest_add_metadata(&update_request, "new meta", "new value", 1);

    AtmosUpdateObjectRequest_add_acl(&update_request, uid1, ATMOS_USER, ATMOS_PERM_FULL);
    AtmosUpdateObjectRequest_add_acl(&update_request, uid2, ATMOS_USER, ATMOS_PERM_READ_WRITE);
    AtmosUpdateObjectRequest_add_acl(&update_request, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP, ATMOS_PERM_READ);

    AtmosClient_update_object(&atmos, &update_request, &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);
    AtmosUpdateObjectRequest_destroy(&update_request);

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int64t_equal(5, read_response.parent.parent.content_length);
    assert_string_equal("hello", read_response.parent.parent.body);
    assert_string_equal("changed value",
            AtmosReadObjectResponse_get_metadata_value(&read_response, "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosReadObjectResponse_get_metadata_value(&read_response, "meta2", 0));
    assert_string_equal("new value",
            AtmosReadObjectResponse_get_metadata_value(&read_response, "new meta", 1));
    assert_int_equal(3, read_response.acl_count);
    assert_int_equal(ATMOS_PERM_FULL, AtmosReadObjectResponse_get_acl_permission(
            &read_response, uid1, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ_WRITE, AtmosReadObjectResponse_get_acl_permission(
            &read_response, uid2, ATMOS_USER));
    assert_int_equal(ATMOS_PERM_READ, AtmosReadObjectResponse_get_acl_permission(
            &read_response, ATMOS_ACL_GROUP_OTHER, ATMOS_GROUP));
    assert_int_equal(ATMOS_PERM_NONE, AtmosReadObjectResponse_get_acl_permission(
            &read_response, "dummy_user_that_does_not_exist", ATMOS_USER));
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_get_object_info() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosGetObjectInfoResponse info_response;
//    AtmosSetUserMetaRequest set_request;
//    AtmosResponse set_response;
    int i;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Get the replica info.
    AtmosGetObjectInfoResponse_init(&info_response);
    AtmosClient_get_object_info(&atmos, response.object_id, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);

    // These fields will vary depending on server configuration, so we can
    // really only do some small verification on them.
    assert_string_equal(response.object_id, info_response.object_id);
    assert_true(info_response.replica_count > 0);
    for(i=0; i<info_response.replica_count; i++) {
        assert_true(info_response.replicas[i].id > 0);
        assert_true(info_response.replicas[i].current < 2);
        assert_true(strcmp("sync", info_response.replicas[i].type) == 0
                || strcmp("async", info_response.replicas[i].type) == 0);
        assert_true(strlen(info_response.replicas[i].location) > 0);
        assert_true(strlen(info_response.replicas[i].storage_type) > 0);
    }
    // Selection is empty in 2.1
    //assert_true(strlen(info_response.selection) > 0);
    AtmosGetObjectInfoResponse_destroy(&info_response);

    // Expiration must be enabled by policy first before you can modify it.

//    // Manually enable expiraton on the object to check those
//    // features.
//    AtmosSetUserMetaRequest_init(&set_request, response.object_id);
//    AtmosSetUserMetaRequest_add_metadata(&set_request,
//            "user.maui.expirationEnable", "true", 0);
//    AtmosSetUserMetaRequest_add_metadata(&set_request,
//            "user.maui.expirationEnd", "2040-01-01T00:00:01Z", 0);
//    AtmosResponse_init(&set_response);
//    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
//    check_error(&set_response);
//    assert_int_equal(200, set_response.parent.http_code);
//    AtmosResponse_destroy(&set_response);
//    AtmosSetUserMetaRequest_destroy(&set_request);
//
//    // Get info again and check expiration/retention
//    AtmosGetObjectInfoResponse_init(&info_response);
//    AtmosClient_get_object_info(&atmos, response.object_id, &info_response);
//    check_error((AtmosResponse*)&info_response);
//    assert_int_equal(200, info_response.parent.parent.http_code);
//    assert_int_equal(1, info_response.expiration_enabled);
//    assert_int64t_equal(2208988801, info_response.expiration_end);
//    AtmosGetObjectInfoResponse_destroy(&info_response);


    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

void test_get_object_info_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosGetObjectInfoResponse info_response;
//    AtmosSetUserMetaRequest set_request;
//    AtmosResponse set_response;
    int i;
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Get the replica info.
    AtmosGetObjectInfoResponse_init(&info_response);
    AtmosClient_get_object_info_ns(&atmos, path, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);

    // These fields will vary depending on server configuration, so we can
    // really only do some small verification on them.
    assert_string_equal(response.object_id, info_response.object_id);
    assert_true(info_response.replica_count > 0);
    for(i=0; i<info_response.replica_count; i++) {
        assert_true(info_response.replicas[i].id > 0);
        assert_true(info_response.replicas[i].current < 2);
        assert_true(strcmp("sync", info_response.replicas[i].type) == 0
                || strcmp("async", info_response.replicas[i].type) == 0);
        assert_true(strlen(info_response.replicas[i].location) > 0);
        assert_true(strlen(info_response.replicas[i].storage_type) > 0);
    }
    // Selection is empty in 2.1
    //assert_true(strlen(info_response.selection) > 0);
    AtmosGetObjectInfoResponse_destroy(&info_response);

    // Expiration must be enabled by policy first before you can modify it.

//    // Manually enable expiraton on the object to check those
//    // features.
//    AtmosSetUserMetaRequest_init_ns(&set_request, path);
//    AtmosSetUserMetaRequest_add_metadata(&set_request,
//            "user.maui.expirationEnable", "true", 0);
//    AtmosSetUserMetaRequest_add_metadata(&set_request,
//            "user.maui.expirationEnd", "2040-01-01T00:00:01Z", 0);
//    AtmosResponse_init(&set_response);
//    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
//    check_error(&set_response);
//    assert_int_equal(200, set_response.parent.http_code);
//    AtmosResponse_destroy(&set_response);
//    AtmosSetUserMetaRequest_destroy(&set_request);
//
//    // Get info again and check expiration/retention
//    AtmosGetObjectInfoResponse_init(&info_response);
//    AtmosClient_get_object_info_ns(&atmos, path, &info_response);
//    check_error((AtmosResponse*)&info_response);
//    assert_int_equal(200, info_response.parent.parent.http_code);
//    assert_int_equal(1, info_response.expiration_enabled);
//    assert_int64t_equal(2208988801, info_response.expiration_end);
//    AtmosGetObjectInfoResponse_destroy(&info_response);


    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_get_object_info_keypool() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosGetObjectInfoResponse info_response;
    int i;
    char *key = random_key();

    printf("Creating key: %s\n", key);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_keypool(&atmos, TEST_KEYPOOL, key, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Get the replica info.
    AtmosGetObjectInfoResponse_init(&info_response);
    AtmosClient_get_object_info_keypool(&atmos, TEST_KEYPOOL, key, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);

    // These fields will vary depending on server configuration, so we can
    // really only do some small verification on them.
    assert_string_equal(response.object_id, info_response.object_id);
    assert_true(info_response.replica_count > 0);
    for(i=0; i<info_response.replica_count; i++) {
        assert_true(info_response.replicas[i].id > 0);
        assert_true(info_response.replicas[i].current < 2);
        assert_true(strcmp("sync", info_response.replicas[i].type) == 0
                || strcmp("async", info_response.replicas[i].type) == 0);
        assert_true(strlen(info_response.replicas[i].location) > 0);
        assert_true(strlen(info_response.replicas[i].storage_type) > 0);
    }
    // Selection is empty in 2.1
    //assert_true(strlen(info_response.selection) > 0);
    AtmosGetObjectInfoResponse_destroy(&info_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_keypool(&atmos, TEST_KEYPOOL, key, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    free(key);
}


void test_rename_object() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosResponse rename_response;
    AtmosReadObjectResponse read_response;

    char path[ATMOS_PATH_MAX];
    char path2[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    random_file(randfile, 8);
    sprintf(path2, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    atmos.enable_utf8_metadata = 1;

    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain",
            &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);
    AtmosCreateObjectResponse_destroy(&response);

    // Rename path1 to path2
    AtmosResponse_init(&rename_response);
    AtmosClient_rename_object(&atmos, path, path2, 0, &rename_response);
    check_error(&rename_response);
    assert_int_equal(200, rename_response.parent.http_code);

    // Check rename
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_ns(&atmos, path2, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_string_equal("test", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Recreate path1
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test2", 5, "text/plain",
            &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);
    AtmosCreateObjectResponse_destroy(&response);

    // Rename path1 to path2, overwriting it
    AtmosResponse_init(&rename_response);
    AtmosClient_rename_object(&atmos, path, path2, 1, &rename_response);
    check_error(&rename_response);
    assert_int_equal(200, rename_response.parent.http_code);

    // Check overwrite
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple_ns(&atmos, path2, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_string_equal("test2", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    // Should not exist!
    assert_int_equal(404, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path2, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosClient_destroy(&atmos);
}

static AtmosDirectoryEntry*
find_file_in_directory(AtmosListDirectoryResponse *dir, const char *name) {
    int i;
    for(i=0; i<dir->entry_count; i++) {
        if(!strcmp(name, dir->entries[i].filename)) {
            return &(dir->entries[i]);
        }
    }

    return NULL;
}


void test_list_directory() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    char path[ATMOS_PATH_MAX];
    char path2[ATMOS_PATH_MAX];
    char randfile1[13];
    char randfile2[13];
    int found_path1 = 0;
    int found_path2 = 0;
    int iterations = 0;
    char token[ATMOS_TOKEN_MAX];
    AtmosListDirectoryRequest list_request;
    AtmosListDirectoryResponse list_response;
    AtmosDirectoryEntry *entry;

    random_file(randfile1, 8);
    strcat(randfile1, ".txt");
    sprintf(path, "/" TEST_DIR "/%s", randfile1);
    random_file(randfile2, 8);
    strcat(randfile2, ".txt");
    sprintf(path2, "/" TEST_DIR "/%s", randfile2);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectRequest_init_ns(&request, path);
    AtmosCreateObjectResponse_init(&response);

    RestRequest_set_array_body((RestRequest*)&request, "test", 4,
            "text/plain; charset=utf-8");
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request ,&response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);
    AtmosCreateObjectRequest_destroy(&request);

    // List and make sure it comes back.
    AtmosListDirectoryRequest_init(&list_request, "/" TEST_DIR "/");
    AtmosListDirectoryResponse_init(&list_response);
    AtmosClient_list_directory(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.parent.http_code);
    assert_string_equal(TEST_DIR,
            list_response.parent.system_metadata.objname);
    assert_string_equal(ATMOS_TYPE_DIRECTORY,
            list_response.parent.system_metadata.type);
    entry = find_file_in_directory(&list_response, randfile1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }
    assert_string_equal(randfile1, entry->filename);
    assert_string_equal(response.object_id, entry->object_id);
    assert_string_equal(ATMOS_TYPE_REGULAR, entry->type);
    assert_int_equal(0, entry->parent.listable_meta_count);
    assert_int_equal(0, entry->parent.meta_count);
    assert_int64t_equal(0, entry->parent.system_metadata.itime); // should not be returned
    AtmosListDirectoryRequest_destroy(&list_request);
    AtmosListDirectoryResponse_destroy(&list_response);

    // Check metadata
    AtmosListDirectoryRequest_init(&list_request, "/" TEST_DIR "/");
    AtmosListDirectoryResponse_init(&list_response);
    list_request.include_meta = 1;
    AtmosClient_list_directory(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.parent.http_code);
    assert_string_equal(TEST_DIR,
            list_response.parent.system_metadata.objname);
    assert_string_equal(ATMOS_TYPE_DIRECTORY,
            list_response.parent.system_metadata.type);
    entry = find_file_in_directory(&list_response, randfile1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }
    assert_string_equal(randfile1, entry->filename);
    assert_string_equal(response.object_id, entry->object_id);
    assert_string_equal(ATMOS_TYPE_REGULAR, entry->type);
    assert_int_equal(2, entry->parent.listable_meta_count);
    assert_int_equal(4, entry->parent.meta_count);

    assert_true(0 < entry->parent.system_metadata.atime);
    assert_true(0 < entry->parent.system_metadata.ctime);
    assert_string_equal("apache", entry->parent.system_metadata.gid);
    assert_true(0 < entry->parent.system_metadata.itime);
    assert_true(0 < entry->parent.system_metadata.mtime);
    assert_true(1 == entry->parent.system_metadata.nlink);
    assert_string_equal(response.object_id, entry->parent.system_metadata.object_id);
    assert_string_equal(randfile1, entry->parent.system_metadata.objname);
    // policy might not be default
    assert_true(strlen(entry->parent.system_metadata.policyname) > 0);
    assert_int64t_equal(4, entry->parent.system_metadata.size);
    assert_string_equal("regular", entry->parent.system_metadata.type);
    assert_string_equal(uid1, entry->parent.system_metadata.uid);

    assert_string_equal("Value  with   spaces",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "meta2", 0));
    assert_string_equal("value",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "name  with   spaces", 0));
    assert_string_equal("",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "empty value", 0));
    assert_string_equal("value1",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "listable1", 1));
    assert_string_equal("",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "listable2", 1));
    AtmosListDirectoryRequest_destroy(&list_request);
    AtmosListDirectoryResponse_destroy(&list_response);

    // Check *some* metadata
    AtmosListDirectoryRequest_init(&list_request, "/" TEST_DIR "/");
    AtmosListDirectoryResponse_init(&list_response);
    list_request.include_meta = 1;
    AtmosListDirectoryRequest_add_user_tag(&list_request, "meta1");
    AtmosListDirectoryRequest_add_user_tag(&list_request, "name  with   spaces");
    AtmosListDirectoryRequest_add_user_tag(&list_request, "listable2");
    AtmosListDirectoryRequest_add_system_tag(&list_request, ATMOS_SYSTEM_META_SIZE);
    AtmosListDirectoryRequest_add_system_tag(&list_request, ATMOS_SYSTEM_META_OBJECTID);
    AtmosClient_list_directory(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.parent.http_code);
    assert_string_equal(TEST_DIR,
            list_response.parent.system_metadata.objname);
    assert_string_equal(ATMOS_TYPE_DIRECTORY,
            list_response.parent.system_metadata.type);
    entry = find_file_in_directory(&list_response, randfile1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }

    assert_string_equal(randfile1, entry->filename);
    assert_string_equal(response.object_id, entry->object_id);
    assert_string_equal(ATMOS_TYPE_REGULAR, entry->type);
    assert_int_equal(1, entry->parent.listable_meta_count);
    assert_int_equal(2, entry->parent.meta_count);

    assert_true(0 == entry->parent.system_metadata.atime);
    assert_true(0 == entry->parent.system_metadata.ctime);
    assert_string_equal("", entry->parent.system_metadata.gid);
    assert_true(0 == entry->parent.system_metadata.itime);
    assert_true(0 == entry->parent.system_metadata.mtime);
    assert_true(0 == entry->parent.system_metadata.nlink);
    assert_string_equal(response.object_id, entry->parent.system_metadata.object_id);
    assert_string_equal("", entry->parent.system_metadata.objname);
    // policy might not be default
    assert_true(strlen(entry->parent.system_metadata.policyname) == 0);
    assert_int64t_equal(4, entry->parent.system_metadata.size);
    assert_string_equal(NULL, entry->parent.system_metadata.type);
    assert_string_equal("", entry->parent.system_metadata.uid);

    assert_string_equal("Value  with   spaces",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "meta1", 0));
    assert_string_equal(NULL,
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "meta2", 0));
    assert_string_equal("value",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "empty value", 0));
    assert_string_equal(NULL,
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "listable1", 1));
    assert_string_equal("",
            AtmosDirectoryEntry_get_metadata_value(entry,
                    "listable2", 1));
    AtmosListDirectoryRequest_destroy(&list_request);
    AtmosListDirectoryResponse_destroy(&list_response);


    // Create a 2nd object
    AtmosCreateObjectResponse_destroy(&response);
    AtmosCreateObjectResponse_init(&response);
    AtmosClient_create_object_simple_ns(&atmos, path2, "test2", 5,
            "text/plain; charset=utf-8", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    // Check pagination
    found_path1 = 0;
    found_path2 = 0;
    token[0] = 0;
    iterations = 0;

    do {
        AtmosListDirectoryRequest_init(&list_request, "/" TEST_DIR "/");
        list_request.parent.limit = 1;
        strcpy(list_request.parent.token, token);
        AtmosListDirectoryResponse_init(&list_response);
        AtmosClient_list_directory(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.parent.http_code);
        assert_string_equal(TEST_DIR,
                list_response.parent.system_metadata.objname);
        assert_string_equal(ATMOS_TYPE_DIRECTORY,
                list_response.parent.system_metadata.type);
        if(find_file_in_directory(&list_response, randfile1)) {
            found_path1 = 1;
        }
        if(find_file_in_directory(&list_response, randfile2)) {
            found_path2 = 1;
        }

        // Check for continuation token.
        if(list_response.token) {
            strncpy(token, list_response.token, ATMOS_TOKEN_MAX);
        } else {
            token[0] = 0;
        }
        AtmosListDirectoryResponse_destroy(&list_response);
        AtmosListDirectoryRequest_destroy(&list_request);
        iterations++;
    } while(strlen(token)>0);

    assert_int_equal(1, found_path1);
    assert_int_equal(1, found_path2);
    assert_true(iterations >= 2);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path2, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);

}

int
list_contains_tag(char **list, size_t count, const char *tag) {
    size_t i;

    for(i=0; i<count; i++) {
        if(!strcmp(tag, list[i])) {
            return 1;
        }
    }

    return 0;
}

void test_get_listable_tags() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetListableTagsRequest list_request;
    AtmosGetListableTagsResponse list_response;
    int tag1_found;
    int tag2_found;
    int pages;
    int i;
    char token[ATMOS_TOKEN_MAX];
    char buffer[ATMOS_META_NAME_MAX];

    RestResponse delete_response;
    get_atmos_client(&atmos);

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // Create some listable tags
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    for(i=0; i<ATMOS_META_COUNT_MAX-2; i++) {
        snprintf(buffer, ATMOS_META_NAME_MAX, TEST_DIR "/listable%d", i);
        AtmosCreateObjectRequest_add_metadata(&request, buffer, "", 1);
    }


    AtmosClient_create_object(&atmos, &request, &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // List tags back and make sure they're present.
    AtmosGetListableTagsRequest_init(&list_request, NULL);
    AtmosGetListableTagsResponse_init(&list_response);
    AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);

    assert_true(list_response.tag_count > 3);
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable1"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable2"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, TEST_DIR));

    AtmosGetListableTagsRequest_destroy(&list_request);
    AtmosGetListableTagsResponse_destroy(&list_response);


    // Test subtags
    AtmosGetListableTagsRequest_init(&list_request, TEST_DIR);
    AtmosGetListableTagsResponse_init(&list_response);
    AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);

    assert_true(list_response.tag_count > 3);
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable1"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable2"));

    AtmosGetListableTagsRequest_destroy(&list_request);
    AtmosGetListableTagsResponse_destroy(&list_response);

    // Test pagination
    tag1_found = 0;
    tag2_found = 0;
    pages = 0;
    *token = 0;

    do {
        AtmosGetListableTagsRequest_init(&list_request, TEST_DIR);
        AtmosGetListableTagsResponse_init(&list_response);
        //list_request.parent.limit = 1; // no effect
        if(*token) {
            strcpy(list_request.parent.token, token);
        }
        AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.http_code);

        tag1_found |= list_contains_tag(list_response.tags,
                list_response.tag_count, "listable1");
        tag2_found |= list_contains_tag(list_response.tags,
                list_response.tag_count, "listable2");

        pages++;

        if(list_response.token) {
            strncpy(token, list_response.token, ATMOS_TOKEN_MAX);
        } else {
            *token = 0;
        }

        AtmosGetListableTagsRequest_destroy(&list_request);
        AtmosGetListableTagsResponse_destroy(&list_response);
    } while(*token);

    assert_true(tag1_found);
    assert_true(tag2_found);
    // The server always decides pagination, so we can't anticpate how many
    // pages are getting returned.
    //assert_true(pages > 1);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}


void test_get_listable_tags_utf8() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    AtmosGetListableTagsRequest list_request;
    AtmosGetListableTagsResponse list_response;
    int tag1_found;
    int tag2_found;
    int pages;
    int i;
    char token[ATMOS_TOKEN_MAX];
    char buffer[ATMOS_META_NAME_MAX];

    RestResponse delete_response;
    get_atmos_client(&atmos);

    atmos.enable_utf8_metadata = 1;

    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    // Create some listable tags
    AtmosCreateObjectRequest_add_metadata(&request, "listable1日本語", "", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2日本語", "", 1);

    for(i=0; i<ATMOS_META_COUNT_MAX-2; i++) {
        snprintf(buffer, ATMOS_META_NAME_MAX, TEST_DIR "/listable日本語%d", i);
        AtmosCreateObjectRequest_add_metadata(&request, buffer, "", 1);
    }


    AtmosClient_create_object(&atmos, &request, &response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);
    printf("Created object: %s\n", response.object_id);

    // List tags back and make sure they're present.
    AtmosGetListableTagsRequest_init(&list_request, NULL);
    AtmosGetListableTagsResponse_init(&list_response);
    AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);

    assert_true(list_response.tag_count > 3);
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable1日本語"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable2日本語"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, TEST_DIR));

    AtmosGetListableTagsRequest_destroy(&list_request);
    AtmosGetListableTagsResponse_destroy(&list_response);


    // Test subtags
    AtmosGetListableTagsRequest_init(&list_request, TEST_DIR);
    AtmosGetListableTagsResponse_init(&list_response);
    AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);

    assert_true(list_response.tag_count > 3);
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable日本語1"));
    assert_true(list_contains_tag(list_response.tags,
            list_response.tag_count, "listable日本語2"));

    AtmosGetListableTagsRequest_destroy(&list_request);
    AtmosGetListableTagsResponse_destroy(&list_response);

    // Test pagination
    tag1_found = 0;
    tag2_found = 0;
    pages = 0;
    *token = 0;

    do {
        AtmosGetListableTagsRequest_init(&list_request, TEST_DIR);
        AtmosGetListableTagsResponse_init(&list_response);
        //list_request.parent.limit = 1; // no effect
        if(*token) {
            strcpy(list_request.parent.token, token);
        }
        AtmosClient_get_listable_tags(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.http_code);

        tag1_found |= list_contains_tag(list_response.tags,
                list_response.tag_count, "listable日本語1");
        tag2_found |= list_contains_tag(list_response.tags,
                list_response.tag_count, "listable日本語2");

        pages++;

        if(list_response.token) {
            strncpy(token, list_response.token, ATMOS_TOKEN_MAX);
        } else {
            *token = 0;
        }

        AtmosGetListableTagsRequest_destroy(&list_request);
        AtmosGetListableTagsResponse_destroy(&list_response);
    } while(*token);

    assert_true(tag1_found);
    assert_true(tag2_found);
    // The server always decides pagination, so we can't anticpate how many
    // pages are getting returned.
    //assert_true(pages > 1);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
}


static AtmosObjectListing*
find_object_in_list(AtmosListObjectsResponse *list, const char *object_id) {
    int i;
    for(i=0; i<list->result_count; i++) {
        if(!strcmp(object_id, list->results[i].object_id)) {
            return &(list->results[i]);
        }
    }

    return NULL;
}

void test_list_objects() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    char obj1[ATMOS_OID_LENGTH];
    char obj2[ATMOS_OID_LENGTH];
    int found_obj1 = 0;
    int found_obj2 = 0;
    int iterations = 0;
    char token[ATMOS_TOKEN_MAX];
    AtmosListObjectsRequest list_request;
    AtmosListObjectsResponse list_response;
    AtmosObjectListing *entry;

    get_atmos_client(&atmos);
    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    RestRequest_set_array_body((RestRequest*)&request, "test", 4,
            "text/plain; charset=utf-8");
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request ,&response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);
    strncpy(obj1, response.object_id, ATMOS_OID_LENGTH);
    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    // List and make sure it comes back.
    AtmosListObjectsRequest_init(&list_request, "listable1");
    AtmosListObjectsResponse_init(&list_response);
    AtmosClient_list_objects(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);
    entry = find_object_in_list(&list_response, obj1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }
    assert_string_equal(obj1, entry->object_id);
    assert_int_equal(0, entry->parent.listable_meta_count);
    assert_int_equal(0, entry->parent.meta_count);
    assert_int64t_equal(0, entry->parent.system_metadata.itime); // should not be returned
    AtmosListObjectsRequest_destroy(&list_request);
    AtmosListObjectsResponse_destroy(&list_response);

    // Check metadata
    AtmosListObjectsRequest_init(&list_request, "listable1");
    AtmosListObjectsResponse_init(&list_response);
    list_request.include_meta = 1;
    AtmosClient_list_objects(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);
    entry = find_object_in_list(&list_response, obj1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }
    assert_string_equal(obj1, entry->object_id);
    assert_int_equal(2, entry->parent.listable_meta_count);
    assert_int_equal(4, entry->parent.meta_count);

    assert_true(0 < entry->parent.system_metadata.atime);
    assert_true(0 < entry->parent.system_metadata.ctime);
    assert_string_equal("apache", entry->parent.system_metadata.gid);
    assert_true(0 < entry->parent.system_metadata.itime);
    assert_true(0 < entry->parent.system_metadata.mtime);
    assert_true(0 == entry->parent.system_metadata.nlink);
    assert_string_equal(obj1, entry->parent.system_metadata.object_id);
    assert_string_equal("", entry->parent.system_metadata.objname);
    // policy might not be default
    assert_true(strlen(entry->parent.system_metadata.policyname) > 0);
    assert_int64t_equal(4, entry->parent.system_metadata.size);
    assert_string_equal("regular", entry->parent.system_metadata.type);
    assert_string_equal(uid1, entry->parent.system_metadata.uid);

    assert_string_equal("Value  with   spaces",
            AtmosObjectListing_get_metadata_value(entry,
                    "meta1", 0));
    assert_string_equal("character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><",
            AtmosObjectListing_get_metadata_value(entry,
                    "meta2", 0));
    assert_string_equal("value",
            AtmosObjectListing_get_metadata_value(entry,
                    "name  with   spaces", 0));
    assert_string_equal("",
            AtmosObjectListing_get_metadata_value(entry,
                    "empty value", 0));
    assert_string_equal("value1",
            AtmosObjectListing_get_metadata_value(entry,
                    "listable1", 1));
    assert_string_equal("",
            AtmosObjectListing_get_metadata_value(entry,
                    "listable2", 1));
    AtmosListObjectsRequest_destroy(&list_request);
    AtmosListObjectsResponse_destroy(&list_response);

    // Check *some* metadata
    AtmosListObjectsRequest_init(&list_request, "listable1");
    AtmosListObjectsResponse_init(&list_response);
    list_request.include_meta = 1;
    AtmosListObjectsRequest_add_user_tag(&list_request, "meta1");
    AtmosListObjectsRequest_add_user_tag(&list_request, "name  with   spaces");
    AtmosListObjectsRequest_add_user_tag(&list_request, "listable2");
    AtmosListObjectsRequest_add_system_tag(&list_request, ATMOS_SYSTEM_META_SIZE);
    AtmosListObjectsRequest_add_system_tag(&list_request, ATMOS_SYSTEM_META_OBJECTID);
    AtmosClient_list_objects(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);
    entry = find_object_in_list(&list_response, obj1);
    assert_true(entry != NULL);
    if(!entry) {
        return;
    }

    assert_string_equal(obj1, entry->object_id);
    assert_int_equal(1, entry->parent.listable_meta_count);
    assert_int_equal(2, entry->parent.meta_count);

    assert_true(0 == entry->parent.system_metadata.atime);
    assert_true(0 == entry->parent.system_metadata.ctime);
    assert_string_equal("", entry->parent.system_metadata.gid);
    assert_true(0 == entry->parent.system_metadata.itime);
    assert_true(0 == entry->parent.system_metadata.mtime);
    assert_true(0 == entry->parent.system_metadata.nlink);
    assert_string_equal(obj1, entry->parent.system_metadata.object_id);
    assert_string_equal("", entry->parent.system_metadata.objname);
    // policy might not be default
    assert_true(strlen(entry->parent.system_metadata.policyname) == 0);
    assert_int64t_equal(4, entry->parent.system_metadata.size);
    assert_string_equal(NULL, entry->parent.system_metadata.type);
    assert_string_equal("", entry->parent.system_metadata.uid);

    assert_string_equal("Value  with   spaces",
            AtmosObjectListing_get_metadata_value(entry,
                    "meta1", 0));
    assert_string_equal(NULL,
            AtmosObjectListing_get_metadata_value(entry,
                    "meta2", 0));
    assert_string_equal("value",
            AtmosObjectListing_get_metadata_value(entry,
                    "name  with   spaces", 0));
    assert_string_equal(NULL,
            AtmosObjectListing_get_metadata_value(entry,
                    "empty value", 0));
    assert_string_equal(NULL,
            AtmosObjectListing_get_metadata_value(entry,
                    "listable1", 1));
    assert_string_equal("",
            AtmosObjectListing_get_metadata_value(entry,
                    "listable2", 1));
    AtmosListObjectsRequest_destroy(&list_request);
    AtmosListObjectsResponse_destroy(&list_response);


    // Create a 2nd object
    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);

    RestRequest_set_array_body((RestRequest*)&request, "test", 4,
            "text/plain; charset=utf-8");
    AtmosCreateObjectRequest_add_metadata(&request, "meta1",
            "Value  with   spaces", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "meta2",
            "character test 1!2@3#4$5%6^7&8*9(0)`~-_+\\|]}[{;:'\"/?.><", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "name  with   spaces",
            "value", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "empty value", "", 0);
    AtmosCreateObjectRequest_add_metadata(&request, "listable1", "value1", 1);
    AtmosCreateObjectRequest_add_metadata(&request, "listable2", "", 1);

    AtmosClient_create_object(&atmos, &request ,&response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);
    strncpy(obj2, response.object_id, ATMOS_OID_LENGTH);
    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);


    // Check pagination
    found_obj1 = 0;
    found_obj2 = 0;
    token[0] = 0;
    iterations = 0;

    do {
        AtmosListObjectsRequest_init(&list_request, "listable1");
        AtmosListObjectsResponse_init(&list_response);
        list_request.parent.limit = 1;
        strcpy(list_request.parent.token, token);
        AtmosClient_list_objects(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.http_code);
        if(find_object_in_list(&list_response, obj1)) {
            found_obj1 = 1;
        }
        if(find_object_in_list(&list_response, obj2)) {
            found_obj2 = 1;
        }

        // Check for continuation token.
        if(list_response.token) {
            strncpy(token, list_response.token, ATMOS_TOKEN_MAX);
        } else {
            token[0] = 0;
        }
        AtmosListObjectsRequest_destroy(&list_request);
        AtmosListObjectsResponse_destroy(&list_response);
        iterations++;
    } while(strlen(token)>0);

    assert_int_equal(1, found_obj1);
    assert_int_equal(1, found_obj2);
    assert_true(iterations >= 2);

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, obj1, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, obj2, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosClient_destroy(&atmos);

}

void test_create_version() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateVersionResponse create_version_response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;
    RestResponse delete_version_response;
    char vid[ATMOS_OID_LENGTH];

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create a version
    AtmosCreateVersionResponse_init(&create_version_response);
    AtmosClient_create_version(&atmos, response.object_id,
            &create_version_response);
    check_error((AtmosResponse*)&create_version_response);
    assert_int_equal(201, create_version_response.parent.parent.http_code);
    strncpy(vid, create_version_response.version_id, ATMOS_OID_LENGTH);
    AtmosCreateVersionResponse_destroy(&create_version_response);

    // Read back the version
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, vid, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_string_equal("test", read_response.parent.parent.body);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid, &delete_version_response);
    assert_int_equal(204, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}


void test_delete_version() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateVersionResponse create_version_response;
    RestResponse delete_response;
    RestResponse delete_version_response;
    char vid[ATMOS_OID_LENGTH];

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create a version
    AtmosCreateVersionResponse_init(&create_version_response);
    AtmosClient_create_version(&atmos, response.object_id,
            &create_version_response);
    check_error((AtmosResponse*)&create_version_response);
    assert_int_equal(201, create_version_response.parent.parent.http_code);
    strncpy(vid, create_version_response.version_id, ATMOS_OID_LENGTH);
    AtmosCreateVersionResponse_destroy(&create_version_response);

    // Delete twice -- the 2nd delete should return 404.
    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid, &delete_version_response);
    assert_int_equal(204, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid, &delete_version_response);
    assert_int_equal(404, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    // Cleanup original object
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_restore_version() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateVersionResponse create_version_response;
    AtmosReadObjectResponse read_response;
    RestResponse delete_response;
    RestResponse delete_version_response;
    RestResponse update_response;
    RestResponse restore_response;
    char vid[ATMOS_OID_LENGTH];

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4,
            "text/plain; charset=utf-8", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create a version
    AtmosCreateVersionResponse_init(&create_version_response);
    AtmosClient_create_version(&atmos, response.object_id,
            &create_version_response);
    check_error((AtmosResponse*)&create_version_response);
    assert_int_equal(201, create_version_response.parent.parent.http_code);
    strncpy(vid, create_version_response.version_id, ATMOS_OID_LENGTH);
    AtmosCreateVersionResponse_destroy(&create_version_response);

    // Update the original object
    RestResponse_init(&update_response);
    AtmosClient_update_object_simple(&atmos, response.object_id,
            "Updated Content", 15, "text/plain; charset=iso-8859-1",
            &update_response);
    assert_int_equal(200, update_response.http_code);
    RestResponse_destroy(&update_response);

    // Read back the original
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_string_equal("Updated Content", read_response.parent.parent.body);
    assert_string_equal("text/plain; charset=iso-8859-1",
            read_response.parent.parent.content_type);
    AtmosReadObjectResponse_destroy(&read_response);

    // Restore the version
    RestResponse_init(&restore_response);
    AtmosClient_restore_version(&atmos, response.object_id, vid,
            &restore_response);
    assert_int_equal(200, restore_response.http_code);
    RestResponse_destroy(&restore_response);

    // Read back the original after the restore.
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_string_equal("test", read_response.parent.parent.body);
    assert_string_equal("text/plain; charset=utf-8",
            read_response.parent.parent.content_type);
    AtmosReadObjectResponse_destroy(&read_response);

    // Cleanup
    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid, &delete_version_response);
    assert_int_equal(204, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

AtmosVersion*
find_version_in_list(const char *vid, AtmosVersion *list, int list_size) {
    int i;

    for(i=0; i<list_size; i++) {
        if(!strcmp(vid, list[i].object_id)) {
            return &list[i];
        }
    }

    return NULL;
}

void test_list_versions() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateVersionResponse create_version_response;
    RestResponse delete_response;
    RestResponse delete_version_response;
    AtmosListVersionsRequest list_request;
    AtmosListVersionsResponse list_response;
    char vid1[ATMOS_OID_LENGTH];
    char vid2[ATMOS_OID_LENGTH];
    char token[ATMOS_TOKEN_MAX];
    int vid1_found;
    int vid2_found;
    int pagecount;
    AtmosVersion *v;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create two versions
    AtmosCreateVersionResponse_init(&create_version_response);
    AtmosClient_create_version(&atmos, response.object_id,
            &create_version_response);
    check_error((AtmosResponse*)&create_version_response);
    assert_int_equal(201, create_version_response.parent.parent.http_code);
    strncpy(vid1, create_version_response.version_id, ATMOS_OID_LENGTH);
    AtmosCreateVersionResponse_destroy(&create_version_response);

    AtmosCreateVersionResponse_init(&create_version_response);
    AtmosClient_create_version(&atmos, response.object_id,
            &create_version_response);
    check_error((AtmosResponse*)&create_version_response);
    assert_int_equal(201, create_version_response.parent.parent.http_code);
    strncpy(vid2, create_version_response.version_id, ATMOS_OID_LENGTH);
    AtmosCreateVersionResponse_destroy(&create_version_response);

    // List objects
    AtmosListVersionsRequest_init(&list_request, response.object_id);
    AtmosListVersionsResponse_init(&list_response);
    AtmosClient_list_versions(&atmos, &list_request, &list_response);
    check_error((AtmosResponse*)&list_response);
    assert_int_equal(200, list_response.parent.parent.http_code);
    assert_int_equal(2, list_response.version_count);
    v = find_version_in_list(vid1, list_response.versions,
            list_response.version_count);
    assert_true(v != NULL);
    if(v) {
        assert_int_equal(0, v->version_number);
        assert_true(0 < v->itime);
    }
    v = find_version_in_list(vid2, list_response.versions,
            list_response.version_count);
    assert_true(v != NULL);
    if(v) {
        assert_int_equal(1, v->version_number);
        assert_true(0 < v->itime);
    }
    AtmosListVersionsRequest_destroy(&list_request);
    AtmosListVersionsResponse_destroy(&list_response);

    // Test pagination
    vid1_found = 0;
    vid2_found = 0;
    *token = 0;
    pagecount = 0;
    do {
        AtmosListVersionsRequest_init(&list_request, response.object_id);
        AtmosListVersionsResponse_init(&list_response);
        list_request.parent.limit = 1;
        if(*token) {
            strncpy(list_request.parent.token, token, ATMOS_TOKEN_MAX);
        }
        AtmosClient_list_versions(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.http_code);

        if(list_response.token) {
            strncpy(token, list_response.token, ATMOS_TOKEN_MAX);
        } else {
            *token = 0;
        }

        pagecount++;
        if(find_version_in_list(vid1, list_response.versions,
            list_response.version_count)) {
            vid1_found++;
        }
        if(find_version_in_list(vid2, list_response.versions,
            list_response.version_count)) {
            vid2_found++;
        }

        AtmosListVersionsRequest_destroy(&list_request);
        AtmosListVersionsResponse_destroy(&list_response);
    } while(*token);

    assert_int_equal(3, pagecount);
    assert_int_equal(1, vid1_found);
    assert_int_equal(1, vid2_found);

    // Cleanup
    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid1, &delete_version_response);
    assert_int_equal(204, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    RestResponse_init(&delete_version_response);
    AtmosClient_delete_version(&atmos, vid2, &delete_version_response);
    assert_int_equal(204, delete_version_response.http_code);
    RestResponse_destroy(&delete_version_response);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_create_dl_token_obj() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosCreateAccessTokenRequest token_request;
    AtmosCreateAccessTokenResponse token_response;
    AtmosGetAccessTokenInfoResponse info_response;
    RestRequest dl_request;
    RestResponse dl_response;
    RestFilter *chain = NULL;
    time_t now;
    char buffer[ATMOS_PATH_MAX];

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create a download token
    AtmosCreateAccessTokenRequest_init_obj(&token_request, response.object_id);
    AtmosCreateAccessTokenResponse_init(&token_response);
    time(&now);
    now += 3600; // 1 hr
    token_request.policy = Atmos_policyType_init(malloc(sizeof(Atmos_policyType)));
    token_request.policy->max_downloads = 1;
    token_request.policy->max_downloads_set = 1;
    token_request.policy->expiration = now;
    token_request.policy->expiration_set = 1;

    AtmosClient_create_access_token(&atmos, &token_request, &token_response);
    check_error((AtmosResponse*)&token_response);
    assert_int_equal(201, token_response.parent.parent.http_code);
    AtmosCreateAccessTokenRequest_destroy(&token_request);
    assert_true(strlen(token_response.token_id) > 0);

    // Verify the token
    AtmosGetAccessTokenInfoResponse_init(&info_response);
    AtmosClient_get_access_token_info(&atmos, token_response.token_id, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);
    assert_true(info_response.token_info != NULL);
    if(info_response.token_info) {
        assert_int64t_equal(now, info_response.token_info->expiration);
        assert_int64t_equal(1, info_response.token_info->max_downloads);
        assert_int64t_equal(1, info_response.token_info->max_downloads_set);
        assert_string_equal(token_response.token_id, info_response.token_info->access_token_id);
        assert_string_equal(response.object_id, info_response.token_info->object_id);
    }

    AtmosGetAccessTokenInfoResponse_destroy(&info_response);

    // Try and download it.
    snprintf(buffer, ATMOS_PATH_MAX, "/rest/accesstokens/%s", token_response.token_id);
    RestRequest_init(&dl_request, buffer, HTTP_GET);
    dl_request.uri_encoded = 1;
    RestResponse_init(&dl_response);
    chain = RestFilter_add(chain, RestFilter_execute_curl_request);
    RestClient_execute_request((RestClient*)&atmos, chain, &dl_request, &dl_response);
    assert_int_equal(200, dl_response.http_code);
    assert_int64t_equal(4, dl_response.content_length);
    assert_string_equal("test", dl_response.body);
    RestRequest_destroy(&dl_request);
    RestResponse_destroy(&dl_response);
    RestFilter_free(chain);

    // Cleanup
    // Delete token (should already be deleted though if download succeeded)
    RestResponse_init(&delete_response);
    AtmosClient_delete_access_token(&atmos, token_response.token_id,
            &delete_response);
    assert_int_equal(404, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Delete object
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateAccessTokenResponse_destroy(&token_response);
    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

void test_create_dl_token_ns() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosCreateAccessTokenRequest token_request;
    AtmosCreateAccessTokenResponse token_response;
    AtmosGetAccessTokenInfoResponse info_response;
    RestRequest dl_request;
    RestResponse dl_response;
    RestFilter *chain = NULL;
    time_t now;
    char buffer[ATMOS_PATH_MAX];
    char path[ATMOS_PATH_MAX];
    char randfile[9];

    random_file(randfile, 8);
    sprintf(path, "/atmos-c-unittest/%s.txt", randfile);
    printf("Creating object: %s\n", path);

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4, "text/plain", &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Create a download token
    AtmosCreateAccessTokenRequest_init_ns(&token_request, path);
    AtmosCreateAccessTokenResponse_init(&token_response);
    time(&now);
    now += 3600; // 1 hr
    token_request.policy = Atmos_policyType_init(malloc(sizeof(Atmos_policyType)));
    token_request.policy->max_downloads = 1;
    token_request.policy->max_downloads_set = 1;
    token_request.policy->expiration = now;
    token_request.policy->expiration_set = 1;

    AtmosClient_create_access_token(&atmos, &token_request, &token_response);
    check_error((AtmosResponse*)&token_response);
    assert_int_equal(201, token_response.parent.parent.http_code);
    AtmosCreateAccessTokenRequest_destroy(&token_request);
    assert_true(strlen(token_response.token_id) > 0);

    // Verify the token
    AtmosGetAccessTokenInfoResponse_init(&info_response);
    AtmosClient_get_access_token_info(&atmos, token_response.token_id, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);
    assert_true(info_response.token_info != NULL);
    if(info_response.token_info) {
        assert_int64t_equal(now, info_response.token_info->expiration);
        assert_int64t_equal(1, info_response.token_info->max_downloads);
        assert_int64t_equal(1, info_response.token_info->max_downloads_set);
        assert_string_equal(token_response.token_id, info_response.token_info->access_token_id);
        assert_string_equal(path, info_response.token_info->path);
    }

    AtmosGetAccessTokenInfoResponse_destroy(&info_response);

    // Try and download it.
    snprintf(buffer, ATMOS_PATH_MAX, "/rest/accesstokens/%s", token_response.token_id);
    RestRequest_init(&dl_request, buffer, HTTP_GET);
    dl_request.uri_encoded = 1;
    RestResponse_init(&dl_response);
    chain = RestFilter_add(chain, RestFilter_execute_curl_request);
    RestClient_execute_request((RestClient*)&atmos, chain, &dl_request, &dl_response);
    assert_int_equal(200, dl_response.http_code);
    assert_int64t_equal(4, dl_response.content_length);
    assert_string_equal("test", dl_response.body);
    RestRequest_destroy(&dl_request);
    RestResponse_destroy(&dl_response);
    RestFilter_free(chain);

    // Cleanup
    // Delete token (should already be deleted though if download succeeded)
    RestResponse_init(&delete_response);
    AtmosClient_delete_access_token(&atmos, token_response.token_id,
            &delete_response);
    assert_int_equal(404, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Delete object
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateAccessTokenResponse_destroy(&token_response);
    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
}

// Form to upload to a token.. includes metadata and a short text file.
#define FORM_DATA "--AAAAAA\r\n"\
    "Content-Type: text/plain\r\n"\
    "Content-Disposition: form-data; name=\"x-emc-meta\"\r\n"\
    "\r\n"\
    "name1=value1\r\n"\
    "--AAAAAA\r\n"\
    "Content-Disposition: form-data; name=\"data\"; filename=\"foo.txt\"\r\n"\
    "Content-Type: text/plain\r\n"\
    "\r\n"\
    "Hello World!\r\n"\
    "--AAAAAA--"


void test_create_ul_token() {
    AtmosClient atmos;
    RestResponse delete_response;
    AtmosCreateAccessTokenRequest token_request;
    AtmosCreateAccessTokenResponse token_response;
    AtmosGetAccessTokenInfoResponse info_response;
    RestRequest ul_request;
    RestResponse ul_response;
    RestFilter *chain = NULL;
    time_t now;
    char buffer[ATMOS_PATH_MAX];
    const char *formdata = FORM_DATA;
    const char *location;
    char object_id[ATMOS_OID_LENGTH];

    get_atmos_client(&atmos);

    // Create an upload token
    AtmosCreateAccessTokenRequest_init(&token_request);
    AtmosCreateAccessTokenResponse_init(&token_response);
    time(&now);
    now += 3600; // 1 hr
    token_request.policy = Atmos_policyType_init(malloc(sizeof(Atmos_policyType)));
    token_request.policy->max_uploads = 1;
    token_request.policy->max_uploads_set = 1;
    token_request.policy->expiration = now;
    token_request.policy->expiration_set = 1;
    token_request.policy->form_field = malloc(sizeof(Atmos_formFieldType));
    token_request.policy->form_field_count = 1;
    Atmos_formFieldType_init(&token_request.policy->form_field[0]);
    token_request.policy->form_field[0].name = strdup("x-emc-meta");
    token_request.policy->form_field[0].contains = malloc(sizeof(char*));
    token_request.policy->form_field[0].contains_count = 1;
    token_request.policy->form_field[0].contains[0] = strdup("name1");

    AtmosClient_create_access_token(&atmos, &token_request, &token_response);
    check_error((AtmosResponse*)&token_response);
    assert_int_equal(201, token_response.parent.parent.http_code);
    AtmosCreateAccessTokenRequest_destroy(&token_request);
    assert_true(strlen(token_response.token_id) > 0);

    // Verify the token
    AtmosGetAccessTokenInfoResponse_init(&info_response);
    AtmosClient_get_access_token_info(&atmos, token_response.token_id, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);
    assert_true(info_response.token_info != NULL);
    if(info_response.token_info) {
        assert_int64t_equal(now, info_response.token_info->expiration);
        assert_int64t_equal(1, info_response.token_info->max_uploads);
        assert_int64t_equal(1, info_response.token_info->max_uploads_set);
        assert_string_equal(token_response.token_id, info_response.token_info->access_token_id);
     }
    AtmosGetAccessTokenInfoResponse_destroy(&info_response);

    // Upload to the token.
    snprintf(buffer, ATMOS_PATH_MAX, "/rest/accesstokens/%s", token_response.token_id);
    RestRequest_init(&ul_request, buffer, HTTP_POST);
    RestRequest_set_array_body(&ul_request, formdata, strlen(formdata), "multipart/form-data; boundary=AAAAAA");
    ul_request.uri_encoded = 1;
    RestResponse_init(&ul_response);
    chain = RestFilter_add(chain, RestFilter_execute_curl_request);
    chain = RestFilter_add(chain, RestFilter_set_content_headers);
    RestClient_execute_request((RestClient*)&atmos, chain, &ul_request, &ul_response);
    assert_int_equal(201, ul_response.http_code);
    location = RestResponse_get_header_value(&ul_response, HTTP_HEADER_LOCATION);
    assert_true(strstr(location, "/rest/objects/") == location);
    strncpy(object_id, location+14, ATMOS_OID_LENGTH);
    assert_int64t_equal(44, strlen(object_id));
    RestRequest_destroy(&ul_request);
    RestResponse_destroy(&ul_response);
    RestFilter_free(chain);

    // Cleanup
    // Delete token
    RestResponse_init(&delete_response);
    AtmosClient_delete_access_token(&atmos, token_response.token_id,
            &delete_response);
    assert_int_equal(404, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    // Delete object
    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateAccessTokenResponse_destroy(&token_response);
    AtmosClient_destroy(&atmos);
}

void test_list_tokens() {
    AtmosClient atmos;
    AtmosCreateAccessTokenRequest token_request;
    AtmosCreateAccessTokenResponse token_response;
    AtmosListAccessTokensRequest list_request;
    AtmosListAccessTokensResponse list_response;
    RestResponse delete_response;
    char token1[ATMOS_PATH_MAX];
    char token2[ATMOS_PATH_MAX];
    int pagecount;
    int token1found;
    int token2found;
    time_t now;
    char page_token[ATMOS_TOKEN_MAX];

    get_atmos_client(&atmos);
    time(&now);
    now += 3600; // 1 hr

    // Create two tokens
    AtmosCreateAccessTokenRequest_init(&token_request);
    AtmosCreateAccessTokenResponse_init(&token_response);
    token_request.policy = Atmos_policyType_init(malloc(sizeof(Atmos_policyType)));
    token_request.policy->max_uploads = 1;
    token_request.policy->max_uploads_set = 1;
    token_request.policy->expiration = now;
    token_request.policy->expiration_set = 1;
    AtmosClient_create_access_token(&atmos, &token_request, &token_response);
    check_error((AtmosResponse*)&token_response);
    assert_int_equal(201, token_response.parent.parent.http_code);
    strncpy(token1, token_response.token_id, ATMOS_PATH_MAX);
    AtmosCreateAccessTokenRequest_destroy(&token_request);
    AtmosCreateAccessTokenResponse_destroy(&token_response);

    AtmosCreateAccessTokenRequest_init(&token_request);
    AtmosCreateAccessTokenResponse_init(&token_response);
    token_request.policy = Atmos_policyType_init(malloc(sizeof(Atmos_policyType)));
    token_request.policy->max_uploads = 1;
    token_request.policy->max_uploads_set = 1;
    token_request.policy->expiration = now;
    token_request.policy->expiration_set = 1;
    AtmosClient_create_access_token(&atmos, &token_request, &token_response);
    check_error((AtmosResponse*)&token_response);
    assert_int_equal(201, token_response.parent.parent.http_code);
    strncpy(token2, token_response.token_id, ATMOS_PATH_MAX);
    AtmosCreateAccessTokenRequest_destroy(&token_request);
    AtmosCreateAccessTokenResponse_destroy(&token_response);

    // List tokens and check
    pagecount = 0;
    token1found = 0;
    token2found = 0;
    *page_token = 0;
    do {
        Atmos_accessTokensListType *tokenlist;
        int i;

        AtmosListAccessTokensRequest_init(&list_request);
        AtmosListAccessTokensResponse_init(&list_response);
        list_request.parent.limit = 1;
        if(*page_token) {
            strcpy(list_request.parent.token, page_token);
        }
        AtmosClient_list_access_tokens(&atmos, &list_request, &list_response);
        check_error((AtmosResponse*)&list_response);
        assert_int_equal(200, list_response.parent.parent.http_code);
        tokenlist = &list_response.token_list->access_tokens_list;
        if(tokenlist) {
            for(i=0; i<tokenlist->access_token_count; i++) {
                if(!strcmp(token1, tokenlist->access_token[i].access_token_id)) {
                    token1found++;
                    assert_int64t_equal(now, tokenlist->access_token[i].expiration);
                    assert_int_equal(1, tokenlist->access_token[i].expiration_set);
                    assert_int64t_equal(1, tokenlist->access_token[i].max_uploads);
                    assert_int_equal(1, tokenlist->access_token[i].max_uploads_set);
                }
                if(!strcmp(token2, tokenlist->access_token[i].access_token_id)) {
                    token2found++;
                    assert_int64t_equal(now, tokenlist->access_token[i].expiration);
                    assert_int_equal(1, tokenlist->access_token[i].expiration_set);
                    assert_int64t_equal(1, tokenlist->access_token[i].max_uploads);
                    assert_int_equal(1, tokenlist->access_token[i].max_uploads_set);
                }
            }
        }
        pagecount++;
        if(list_response.token) {
            strcpy(page_token, list_response.token);
        } else {
            *page_token = 0;
        }
        AtmosListAccessTokensRequest_destroy(&list_request);
        AtmosListAccessTokensResponse_destroy(&list_response);
    } while(*page_token);

    assert_true(pagecount > 1);
    assert_int_equal(1, token1found);
    assert_int_equal(1, token2found);

    // Delete tokens
    RestResponse_init(&delete_response);
    AtmosClient_delete_access_token(&atmos, token1,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);
    RestResponse_init(&delete_response);
    AtmosClient_delete_access_token(&atmos, token2,
            &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);
    AtmosClient_destroy(&atmos);
}

#define CHECK_STRING_1 "hello world"
#define CHECK_STRING_1a "hello"
#define CHECK_STRING_1b " world"
#define CHECK_STRING_1_SHA0 "9fce82c34887c1953b40b3a2883e18850c4fa8a6"
#define CHECK_STRING_1a_SHA0 "ac62a630ca850b4ea07eda664eaecf9480843152"
#define CHECK_STRING_1_SHA1 "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed"
#define CHECK_STRING_1a_SHA1 "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d"
#define CHECK_STRING_1_MD5 "5eb63bbbe01eeed093cb22bb8f5acdc3"
#define CHECK_STRING_1a_MD5 "5d41402abc4b2a76b9719d911017c592"
#define CHECK_STRING_1_OFFSET "/11/"

void test_checksums() {
    AtmosChecksum *ck;
    char *value;

    // SHA0
    ck = AtmosChecksum_init(malloc(sizeof(AtmosChecksum)), ATMOS_WSCHECKSUM,
            ATMOS_ALG_SHA0);
    AtmosChecksum_update(ck, CHECK_STRING_1, strlen(CHECK_STRING_1));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_SHA0 CHECK_STRING_1_OFFSET CHECK_STRING_1_SHA0, value);
    free(value);

    AtmosChecksum_destroy(ck);

    // Incremental check
    ck = AtmosChecksum_init(ck, ATMOS_WSCHECKSUM, ATMOS_ALG_SHA0);
    AtmosChecksum_update(ck, CHECK_STRING_1a, strlen(CHECK_STRING_1a));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1a_SHA0, value);
    free(value);

    AtmosChecksum_update(ck, CHECK_STRING_1b, strlen(CHECK_STRING_1b));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_SHA0 CHECK_STRING_1_OFFSET CHECK_STRING_1_SHA0, value);
    free(value);

    AtmosChecksum_destroy(ck);
    free(ck);

    // SHA1
    ck = AtmosChecksum_init(malloc(sizeof(AtmosChecksum)), ATMOS_WSCHECKSUM,
            ATMOS_ALG_SHA1);
    AtmosChecksum_update(ck, CHECK_STRING_1, strlen(CHECK_STRING_1));

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_SHA1 CHECK_STRING_1_OFFSET CHECK_STRING_1_SHA1, value);
    free(value);

    AtmosChecksum_destroy(ck);

    // Incremental
    ck = AtmosChecksum_init(ck, ATMOS_WSCHECKSUM, ATMOS_ALG_SHA1);
    AtmosChecksum_update(ck, CHECK_STRING_1a, strlen(CHECK_STRING_1a));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1a_SHA1, value);
    free(value);

    AtmosChecksum_update(ck, CHECK_STRING_1b, strlen(CHECK_STRING_1b));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1_SHA1, value);
    free(value);

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_SHA1 CHECK_STRING_1_OFFSET CHECK_STRING_1_SHA1, value);
    free(value);

    AtmosChecksum_destroy(ck);
    free(ck);

    // MD5
    ck = AtmosChecksum_init(malloc(sizeof(AtmosChecksum)), ATMOS_WSCHECKSUM,
            ATMOS_ALG_MD5);
    AtmosChecksum_update(ck, CHECK_STRING_1, strlen(CHECK_STRING_1));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1_MD5, value);
    free(value);

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_MD5 CHECK_STRING_1_OFFSET CHECK_STRING_1_MD5, value);
    free(value);

    AtmosChecksum_destroy(ck);

    // Incremental
    ck = AtmosChecksum_init(ck, ATMOS_WSCHECKSUM, ATMOS_ALG_MD5);
    AtmosChecksum_update(ck, CHECK_STRING_1a, strlen(CHECK_STRING_1a));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1a_MD5, value);
    free(value);

    AtmosChecksum_update(ck, CHECK_STRING_1b, strlen(CHECK_STRING_1b));

    value = AtmosChecksum_get_value(ck);
    assert_string_equal(CHECK_STRING_1_MD5, value);
    free(value);

    value = AtmosChecksum_get_state(ck);
    assert_string_equal(ATMOS_CHECKSUM_ALG_MD5 CHECK_STRING_1_OFFSET CHECK_STRING_1_MD5, value);
    free(value);

    AtmosChecksum_destroy(ck);
    free(ck);
}

void test_create_object_wschecksum() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateObjectRequest request;
    RestResponse delete_response;
    AtmosChecksum ck;
    char *value;

    if(atmos_major < 2 && atmos_minor < 4) {
        printf("...skipped (<1.4)\n");
        return;
    }

    printf("SHA0\n");
    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);
    AtmosCreateObjectRequest_init(&request);

    RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
            strlen(CHECK_STRING_1), "text/plain");

    AtmosChecksum_init(&ck, ATMOS_WSCHECKSUM, ATMOS_ALG_SHA0);
    request.parent.checksum = &ck;

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    value = AtmosChecksum_get_value(&ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosChecksum_destroy(&ck);

    printf("SHA1\n");
    if((atmos_major == 2 && atmos_minor >= 1) || atmos_major > 2) {
        AtmosCreateObjectResponse_init(&response);
        AtmosCreateObjectRequest_init(&request);

        RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
                strlen(CHECK_STRING_1), "text/plain");

        AtmosChecksum_init(&ck, ATMOS_WSCHECKSUM, ATMOS_ALG_SHA1);
        request.parent.checksum = &ck;

        AtmosClient_create_object(&atmos, &request, &response);
        check_error((AtmosResponse*)&response);
        assert_int_equal(201, response.parent.parent.http_code);
        assert_true(strlen(response.object_id) > 0);

        printf("Created object: %s\n", response.object_id);

        value = AtmosChecksum_get_value(&ck);
        assert_string_equal(CHECK_STRING_1_SHA1, value);
        free(value);

        RestResponse_init(&delete_response);
        AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
        assert_int_equal(204, delete_response.http_code);
        RestResponse_destroy(&delete_response);

        AtmosCreateObjectResponse_destroy(&response);
        AtmosChecksum_destroy(&ck);

    } else {
        printf("...skipped (atmos<2.1)\n");
    }

    printf("MD5\n");
    if((atmos_major == 2 && atmos_minor >= 1) || atmos_major > 2) {
        AtmosCreateObjectResponse_init(&response);
        AtmosCreateObjectRequest_init(&request);

        RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
                strlen(CHECK_STRING_1), "text/plain");

        AtmosChecksum_init(&ck, ATMOS_WSCHECKSUM, ATMOS_ALG_MD5);
        request.parent.checksum = &ck;

        AtmosClient_create_object(&atmos, &request, &response);
        check_error((AtmosResponse*)&response);
        assert_int_equal(201, response.parent.parent.http_code);
        assert_true(strlen(response.object_id) > 0);

        printf("Created object: %s\n", response.object_id);

        value = AtmosChecksum_get_value(&ck);
        assert_string_equal(CHECK_STRING_1_MD5, value);
        free(value);

        RestResponse_init(&delete_response);
        AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
        assert_int_equal(204, delete_response.http_code);
        RestResponse_destroy(&delete_response);

        AtmosCreateObjectResponse_destroy(&response);
        AtmosChecksum_destroy(&ck);

    } else {
        printf("...skipped (atmos<2.1)\n");
    }

    AtmosClient_destroy(&atmos);
}

void test_append_object_wschecksum() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateObjectRequest request;
    AtmosUpdateObjectRequest update_request;
    RestResponse update_response;
    RestResponse delete_response;
    AtmosChecksum ck;
    char *value;

    if(atmos_major < 2 && atmos_minor < 4) {
        printf("...skipped (<1.4)\n");
        return;
    }

    printf("SHA0\n");
    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);
    AtmosCreateObjectRequest_init(&request);

    RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1a,
            strlen(CHECK_STRING_1a), "text/plain");

    AtmosChecksum_init(&ck, ATMOS_WSCHECKSUM, ATMOS_ALG_SHA0);
    request.parent.checksum = &ck;

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    AtmosUpdateObjectRequest_init(&update_request, response.object_id);
    RestResponse_init(&update_response);
    update_request.parent.checksum = &ck;
    AtmosUpdateObjectRequest_set_range_offset_size(&update_request,
            strlen(CHECK_STRING_1a), strlen(CHECK_STRING_1b));
    RestRequest_set_array_body((RestRequest*)&update_request, CHECK_STRING_1b,
            strlen(CHECK_STRING_1b), "text/plain");
    AtmosClient_update_object(&atmos, &update_request, &update_response);
    assert_int_equal(200, update_response.http_code);

    value = AtmosChecksum_get_value(&ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);


    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosChecksum_destroy(&ck);
    AtmosClient_destroy(&atmos);
}

void test_generate_checksum() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    AtmosCreateObjectRequest request;
    RestResponse delete_response;
    AtmosChecksum ck;
    char *value;

    if(atmos_major < 2 && atmos_minor < 4) {
        printf("...skipped (<1.4)\n");
        return;
    }

    printf("SHA0\n");
    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);
    AtmosCreateObjectRequest_init(&request);

    RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
            strlen(CHECK_STRING_1), "text/plain");

    AtmosChecksum_init(&ck, ATMOS_GENERATE_CHECKSUM, ATMOS_ALG_SHA0);
    request.parent.checksum = &ck;

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    assert_int_equal(1, AtmosChecksum_verify_response(&ck, (RestResponse*)&response));

    value = AtmosChecksum_get_value(&ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosChecksum_destroy(&ck);

    printf("SHA1\n");
    if((atmos_major == 2 && atmos_minor >= 1) || atmos_major > 2) {
        AtmosCreateObjectResponse_init(&response);
        AtmosCreateObjectRequest_init(&request);

        RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
                strlen(CHECK_STRING_1), "text/plain");

        AtmosChecksum_init(&ck, ATMOS_GENERATE_CHECKSUM, ATMOS_ALG_SHA1);
        request.parent.checksum = &ck;

        AtmosClient_create_object(&atmos, &request, &response);
        check_error((AtmosResponse*)&response);
        assert_int_equal(201, response.parent.parent.http_code);
        assert_true(strlen(response.object_id) > 0);

        printf("Created object: %s\n", response.object_id);

        assert_int_equal(1, AtmosChecksum_verify_response(&ck, (RestResponse*)&response));

        value = AtmosChecksum_get_value(&ck);
        assert_string_equal(CHECK_STRING_1_SHA1, value);
        free(value);

        RestResponse_init(&delete_response);
        AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
        assert_int_equal(204, delete_response.http_code);
        RestResponse_destroy(&delete_response);

        AtmosCreateObjectResponse_destroy(&response);
        AtmosChecksum_destroy(&ck);

    } else {
        printf("...skipped (atmos<2.1)\n");
    }

    printf("MD5\n");
    if((atmos_major == 2 && atmos_minor >= 1) || atmos_major > 2) {
        AtmosCreateObjectResponse_init(&response);
        AtmosCreateObjectRequest_init(&request);

        RestRequest_set_array_body((RestRequest*)&request, CHECK_STRING_1,
                strlen(CHECK_STRING_1), "text/plain");

        AtmosChecksum_init(&ck, ATMOS_GENERATE_CHECKSUM, ATMOS_ALG_MD5);
        request.parent.checksum = &ck;

        AtmosClient_create_object(&atmos, &request, &response);
        check_error((AtmosResponse*)&response);
        assert_int_equal(201, response.parent.parent.http_code);
        assert_true(strlen(response.object_id) > 0);

        printf("Created object: %s\n", response.object_id);

        assert_int_equal(1, AtmosChecksum_verify_response(&ck, (RestResponse*)&response));

        value = AtmosChecksum_get_value(&ck);
        assert_string_equal(CHECK_STRING_1_MD5, value);
        free(value);

        RestResponse_init(&delete_response);
        AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
        assert_int_equal(204, delete_response.http_code);
        RestResponse_destroy(&delete_response);

        AtmosCreateObjectResponse_destroy(&response);
        AtmosChecksum_destroy(&ck);

    } else {
        printf("...skipped (atmos<2.1)\n");
    }

    AtmosClient_destroy(&atmos);

}

void test_generate_checksum_file() {
    AtmosClient atmos;
    AtmosCreateObjectRequest request;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;
    AtmosChecksum ck;
    FILE *f;
    char *value;

    f = tmpfile();
    fwrite(CHECK_STRING_1, strlen(CHECK_STRING_1), 1, f);
    fseek(f, 0, SEEK_SET);

    get_atmos_client(&atmos);
    //atmos.signature_debug = 1;
    AtmosCreateObjectRequest_init(&request);
    AtmosCreateObjectResponse_init(&response);
    AtmosChecksum_init(&ck, ATMOS_GENERATE_CHECKSUM, ATMOS_ALG_SHA0);
    request.parent.checksum = &ck;
    RestRequest_set_file_body((RestRequest*)&request, f, strlen(CHECK_STRING_1), "text/plain");

    AtmosClient_create_object(&atmos, &request, &response);
    check_error((AtmosResponse*)&response);
    assert_int_equal(201, response.parent.parent.http_code);
    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    // Verify the server's checksum response with our local one.
    assert_int_equal(1, AtmosChecksum_verify_response(&ck, (RestResponse*)&response));

    value = AtmosChecksum_get_value(&ck);
    assert_string_equal(CHECK_STRING_1_SHA0, value);
    free(value);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
    AtmosChecksum_destroy(&ck);
    fclose(f);
}


void test_atmos_suite() {
    char proxy_port_str[32];

    FILE *config = fopen("atmostest.ini", "r");
    if (!config) {
        fprintf(stderr, "Error opening atmostest.ini: %s\n", strerror(errno));
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }

    memset(user_id, 0, sizeof(user_id));
    memset(key, 0, sizeof(key));
    memset(endpoint, 0, sizeof(endpoint));

    if(!fgets(user_id, 255, config)) {
        fprintf(stderr, "Missing user_id in atmostest.ini");
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }
    if(!fgets(key, 64, config)) {
        fprintf(stderr, "Missing secret key in atmostest.ini");
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }
    if(!fgets(endpoint, 255, config)) {
        fprintf(stderr, "Missing endpoint in atmostest.ini");
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }
    if(!fgets(uid1, 64, config)) {
        fprintf(stderr, "Missing uid1 in atmostest.ini");
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }
    if(!fgets(uid2, 64, config)) {
        fprintf(stderr, "Missing uid2 in atmostest.ini");
        fprintf(stderr, "Check your atmostest.ini and try again\n");
        exit(1);
    }
    if(!fgets(proxy_host, 256, config)) {
        // OK, optional
    }
    if(!fgets(proxy_port_str, 32, config)) {
        // OK, optional
    }
    if(!fgets(proxy_user, 256, config)) {
        // OK, optional
    }
    if(!fgets(proxy_pass, 256, config)) {
        // OK, optional
    }

    // Strip newlines
    user_id[strlen(user_id) - 1] = 0;
    key[strlen(key) - 1] = 0;
    endpoint[strlen(endpoint) - 1] = 0;
    uid1[strlen(uid1) - 1] = 0;
    uid2[strlen(uid2) - 1] = 0;

    if (strlen(proxy_host) > 0) {
        proxy_host[strlen(proxy_host) - 1] = 0;
    }
    if (strlen(proxy_user) > 0) {
        proxy_user[strlen(proxy_user) - 1] = 0;
    }
    if (strlen(proxy_pass) > 0) {
        proxy_pass[strlen(proxy_pass) - 1] = 0;
    }
    if (strlen(proxy_port_str) > 1) {
        proxy_port = atoi(proxy_port_str);
    } else {
        proxy_port = -1;
    }

    fclose(config);

    printf("==================\n");
    printf("TEST CONFIGURATION\n");
    printf("==================\n");
    printf("user_id: %s\n", user_id);
    printf("key: %s\n", key);
    printf("endpoint: %s\n", endpoint);
    printf("uid1: %s\n", uid1);
    printf("uid2: %s\n", uid2);
    printf("proxy_host: %s\n", proxy_host);
    printf("proxy_port: %d\n", proxy_port);
    printf("proxy_user: %s\n", proxy_user);
    printf("proxy_pass: %s\n", proxy_pass);
    fflush(stdout);

    if (strlen(user_id) < 1) {
        fprintf(stderr, "user_id unset.  Check atmostest.ini\n");
        exit(1);
    }
    if (strlen(key) < 1) {
        fprintf(stderr, "key unset.  Check atmostest.ini\n");
        exit(1);
    }
    if (strlen(endpoint) < 1) {
        fprintf(stderr, "endpoint unset.  Check atmostest.ini\n");
        exit(1);
    }
    if (strlen(uid1) < 1) {
        fprintf(stderr, "uid1 unset.  Check atmostest.ini\n");
        exit(1);
    }
    if (strlen(uid2) < 1) {
        fprintf(stderr, "uid2 unset.  Check atmostest.ini\n");
        exit(1);
    }

    if (strncmp("http", endpoint, 4) != 0) {
        fprintf(stderr,
                "endpoint should start with either http or https.  Check atmostest.ini\n");
        exit(1);
    }

#ifdef srandomdev
    srandomdev();
#else
    {
        time_t t;
        time(&t);
        srandom((unsigned int)t);
    }
#endif

    // Run tests
    test_fixture_start();

    // Test internal functions
    start_test_msg("test_normalize_whitespace");
    run_test(test_normalize_whitespace);

    start_test_msg("test_sign_string");
    run_test(test_sign_string);

    start_test_msg("test_sign_request");
    run_test(test_sign_request);

    start_test_msg("test_acl_generator");
    run_test(test_acl_generator);

    start_test_msg("test_meta_generator");
    run_test(test_meta_generator);

    start_test_msg("test_meta_generator_utf8");
    run_test(test_meta_generator_utf8);

    start_test_msg("test_parse_meta");
    run_test(test_parse_meta);

    start_test_msg("test_parse_meta_utf8");
    run_test(test_parse_meta_utf8);

    start_test_msg("test_parse_system_meta");
    run_test(test_parse_system_meta);

    start_test_msg("test_parse_system_meta_utf8");
    run_test(test_parse_system_meta_utf8);

    start_test_msg("test_parse_acl");
    run_test(test_parse_acl);

    start_test_msg("test_parse_object_info");
    run_test(test_parse_object_info);

    start_test_msg("test_parse_dirlist");
    run_test(test_parse_dirlist);

    start_test_msg("test_checksums");
    run_test(test_checksums);

    // Start network tests to Atmos

    start_test_msg("test_get_service_info");
    run_test(test_get_service_info);

    start_test_msg("test_parse_error");
    run_test(test_parse_error);

    start_test_msg("test_create_object");
    run_test(test_create_object);

    start_test_msg("test_create_object_ns");
    run_test(test_create_object_ns);

    start_test_msg("test_create_object_keypool");
    run_test(test_create_object_keypool);

    start_test_msg("test_create_object_meta");
    run_test(test_create_object_meta);

    start_test_msg("test_create_object_meta_utf8");
    run_test(test_create_object_meta_utf8);

    start_test_msg("test_create_object_acl");
    run_test(test_create_object_acl);

    start_test_msg("test_read_object");
    run_test(test_read_object);

    start_test_msg("test_read_object_ns");
    run_test(test_read_object_ns);

    start_test_msg("test_read_object_keypool");
    run_test(test_read_object_keypool);

    start_test_msg("test_read_object_with_meta_and_acl");
    run_test(test_read_object_with_meta_and_acl);

    start_test_msg("test_read_object_range");
    run_test(test_read_object_range);

    start_test_msg("test_read_object_file");
    run_test(test_read_object_file);

    start_test_msg("test_get_user_meta");
    run_test(test_get_user_meta);

    start_test_msg("test_get_user_meta_keypool");
    run_test(test_get_user_meta_keypool);

    start_test_msg("test_get_user_meta_ns");
    run_test(test_get_user_meta_ns);

    start_test_msg("test_delete_user_meta");
    run_test(test_delete_user_meta);

    start_test_msg("test_delete_user_meta_ns");
    run_test(test_delete_user_meta_ns);

    start_test_msg("test_delete_user_meta_keypool");
    run_test(test_delete_user_meta_keypool);

    start_test_msg("test_set_user_meta");
    run_test(test_set_user_meta);

    start_test_msg("test_set_user_meta_ns");
    run_test(test_set_user_meta_ns);

    start_test_msg("test_set_user_meta_keypool");
    run_test(test_set_user_meta_keypool);

    start_test_msg("test_get_system_meta");
    run_test(test_get_system_meta);

    start_test_msg("test_get_system_meta_ns");
    run_test(test_get_system_meta_ns);

    start_test_msg("test_get_system_meta_keypool");
    run_test(test_get_system_meta_keypool);

    start_test_msg("test_set_get_acl");
    run_test(test_set_get_acl);

    start_test_msg("test_set_get_acl_ns");
    run_test(test_set_get_acl_ns);

    start_test_msg("test_update_object");
    run_test(test_update_object);

    start_test_msg("test_update_object_ns");
    run_test(test_update_object_ns);

    start_test_msg("test_update_object_keypool");
    run_test(test_update_object_keypool);

    start_test_msg("test_update_object_file");
    run_test(test_update_object_file);

    start_test_msg("test_update_object_file_ns");
    run_test(test_update_object_file_ns);

    start_test_msg("test_update_object_file_keypool");
    run_test(test_update_object_file_keypool);

    start_test_msg("test_update_object_range");
    run_test(test_update_object_range);

    start_test_msg("test_update_object_range_file");
    run_test(test_update_object_range_file);

    start_test_msg("test_update_object_meta_acl");
    run_test(test_update_object_meta_acl);

    start_test_msg("test_get_object_info");
    run_test(test_get_object_info);

    start_test_msg("test_get_object_info_ns");
    run_test(test_get_object_info_ns);

    start_test_msg("test_get_object_info_keypool");
    run_test(test_get_object_info_keypool);

    start_test_msg("test_rename_object");
    run_test(test_rename_object);

    start_test_msg("test_list_directory");
    run_test(test_list_directory);

    start_test_msg("test_get_listable_tags");
    run_test(test_get_listable_tags);

    start_test_msg("test_get_listable_tags_utf8");
    run_test(test_get_listable_tags_utf8);

    start_test_msg("test_list_objects");
    run_test(test_list_objects);

    start_test_msg("test_create_version");
    run_test(test_create_version);

    start_test_msg("test_delete_version");
    run_test(test_delete_version);

    start_test_msg("test_restore_version");
    run_test(test_restore_version);

    start_test_msg("test_list_versions");
    run_test(test_list_versions);

    start_test_msg("test_create_dl_token_obj");
    run_test(test_create_dl_token_obj);

    start_test_msg("test_create_dl_token_ns");
    run_test(test_create_dl_token_ns);

    start_test_msg("test_create_ul_token");
    run_test(test_create_ul_token);

    start_test_msg("test_list_tokens");
    run_test(test_list_tokens);

    start_test_msg("test_create_object_wschecksum");
    run_test(test_create_object_wschecksum);

    start_test_msg("test_append_object_wschecksum");
    run_test(test_append_object_wschecksum);

    start_test_msg("test_generate_checksum");
    run_test(test_generate_checksum);

    start_test_msg("test_generate_checksum_file");
    run_test(test_generate_checksum_file);

    test_fixture_end();
}
