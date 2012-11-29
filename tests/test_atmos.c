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

#include "seatest.h"
#include "test.h"
#include "atmos.h"
#include "atmos_util.h"
#include "atmos_private.h" // To test private methods
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

static const char
        *filechars =
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~!$&\"()*+;:";

static void random_file(char *name, int count) {
    int i = 0;
    int num_filechars = strlen(filechars);
    for (; i < count; i++) {
        name[i] = filechars[random() % num_filechars];
    }
    name[i] = 0;
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
    strcpy(meta[3].value, "invalid character test ,=\v\x80\n");

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
    strcpy(meta[3].value, "invalid character test ,=\v\x80\n");

    RestRequest_init(&request, "http://www.google.com", HTTP_GET);
    AtmosUtil_set_metadata_header(meta, 4, 1, 1, &request);
    header_value = RestRequest_get_header_value(&request,
            ATMOS_HEADER_LISTABLE_META);
    assert_string_equal("cryllic=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE,"
            "Japanese=%E3%81%A9%E3%81%86%E3%82%82%E3%81%82%E3%82%8A%E3%81%8C%E3%81%A8%E3%81%86,"
            "Composed%20accents=%C3%A9%C3%AA%C3%AB%C3%A8,"
            "Special%20Characters=invalid%20character%20test%20%2C%3D%0B%80%0A",
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
            "Special%20Characters=invalid%20character%20test%20%2C%3D%0B%80%0A");
    RestResponse_add_header(
            &response,
            ATMOS_HEADER_LISTABLE_META ":"
            "%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE,"
            "Japanese=%E3%81%A9%E3%81%86%E3%82%82%E3%81%82%E3%82%8A%E3%81%8C%E3%81%A8%E3%81%86,"
            "Composed%20accents=%C3%A9%C3%AA%C3%AB%C3%A8,"
            "Special%20Characters=invalid%20character%20test%20%2C%3D%0B%80%0A");
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
    assert_string_equal("invalid character test ,=\v\x80\n", meta[3].value);
    assert_string_equal("русский", listable_meta[0].name);
    assert_string_equal("Japanese", listable_meta[1].name);
    assert_string_equal("Composed accents", listable_meta[2].name);
    assert_string_equal("Special Characters", listable_meta[3].name);
    assert_string_equal("спасибо", listable_meta[0].value);
    assert_string_equal("どうもありがとう", listable_meta[1].value);
    assert_string_equal("éêëè", listable_meta[2].value);
    assert_string_equal("invalid character test ,=\v\x80\n", listable_meta[3].value);

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
    assert_int_equal(1234974444, system_meta.atime);
    assert_int_equal(1234974444, system_meta.ctime);
    assert_string_equal("apache", system_meta.gid);
    assert_int_equal(1234973032, system_meta.itime);
    assert_int_equal(1234973032, system_meta.mtime);
    assert_int_equal(0, system_meta.nlink);
    assert_string_equal("499ad542a1a8bc200499ad5a6b05580499c3168560a4", system_meta.object_id);
    assert_string_equal("", system_meta.objname);
    assert_string_equal("default", system_meta.policyname);
    assert_int_equal(211, system_meta.size);
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
    assert_int_equal(1325866560, system_meta.atime);
    assert_int_equal(1325866560, system_meta.ctime);
    assert_string_equal("apache", system_meta.gid);
    assert_int_equal(1325865567, system_meta.itime);
    assert_int_equal(1325865568, system_meta.mtime);
    assert_int_equal(1, system_meta.nlink);
    assert_string_equal("4ef49feaa106904c04ef4a066e778104f071a5ff0c85", system_meta.object_id);
    assert_string_equal("υπολογιστή.jpg", system_meta.objname);
    assert_string_equal("default", system_meta.policyname);
    assert_int_equal(459, system_meta.size);
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
    assert_int_equal(0, info.retention_end);
    assert_int_equal(1, info.expiration_enabled);
    assert_int_equal(5, info.expiration_end);

    AtmosGetObjectInfoResponse_destroy(&info);
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
            "invalid character test ,=\v\x80\n", 0);

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
    assert_int_equal(4, read_response.parent.parent.content_length);
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
    assert_int_equal(4, read_response.parent.parent.content_length);
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
    assert_int_equal(4, read_response.parent.parent.content_length);
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
    assert_int_equal(4, read_response.system_metadata.size);
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
    assert_int_equal(1024*1024*2, read_response.parent.parent.content_length);
    assert_int_equal(1024*1024*2, ftell(f2));

    // Check data in f2.
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);

    for(i=0; i<8192; i++) {
        fread(buffer, 256, 1, f1);
        fread(buffer2, 256, 1, f2);

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

    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectRequest_destroy(&request);
    AtmosCreateObjectResponse_destroy(&response);

    AtmosClient_destroy(&atmos);
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
    assert_int_equal(4, meta_response.system_metadata.size);
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
    assert_int_equal(0, meta_response.system_metadata.size);
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
    assert_int_equal(4, meta_response.system_metadata.size);
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
    assert_int_equal(0, meta_response.system_metadata.size);
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
    assert_int_equal(5, read_response.parent.parent.content_length);
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
    assert_int_equal(5, read_response.parent.parent.content_length);
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
    assert_int_equal(5, read_response.parent.parent.content_length);
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
    assert_int_equal(5, read_response.parent.parent.content_length);
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
    assert_int_equal(13, read_response.parent.parent.content_length);
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
    assert_int_equal(19, read_response.parent.parent.content_length);
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
    assert_int_equal(19, read_response.parent.parent.content_length);
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

    // Read back and check
    AtmosReadObjectResponse_init(&read_response);
    AtmosClient_read_object_simple(&atmos, response.object_id, &read_response);
    check_error((AtmosResponse*)&read_response);
    assert_int_equal(200, read_response.parent.parent.http_code);
    assert_int_equal(5, read_response.parent.parent.content_length);
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
    AtmosSetUserMetaRequest set_request;
    AtmosResponse set_response;
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
    assert_true(strlen(info_response.selection) > 0);
    AtmosGetObjectInfoResponse_destroy(&info_response);

    // Manually enable expiraton on the object to check those
    // features.
    AtmosSetUserMetaRequest_init(&set_request, response.object_id);
    AtmosSetUserMetaRequest_add_metadata(&set_request,
            "user.maui.expirationEnable", "true", 0);
    AtmosSetUserMetaRequest_add_metadata(&set_request,
            "user.maui.expirationEnd", "2040-01-01T00:00:01Z", 0);
    AtmosResponse_init(&set_response);
    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
    check_error(&set_response);
    assert_int_equal(200, set_response.parent.http_code);
    AtmosResponse_destroy(&set_response);
    AtmosSetUserMetaRequest_destroy(&set_request);

    // Get info again and check expiration/retention
    AtmosGetObjectInfoResponse_init(&info_response);
    AtmosClient_get_object_info(&atmos, response.object_id, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);
    assert_int_equal(1, info_response.expiration_enabled);
    assert_int64t_equal(2208988801, info_response.expiration_end);
    AtmosGetObjectInfoResponse_destroy(&info_response);


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
    AtmosSetUserMetaRequest set_request;
    AtmosResponse set_response;
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
    assert_true(strlen(info_response.selection) > 0);
    AtmosGetObjectInfoResponse_destroy(&info_response);

    // Manually enable expiraton on the object to check those
    // features.
    AtmosSetUserMetaRequest_init_ns(&set_request, path);
    AtmosSetUserMetaRequest_add_metadata(&set_request,
            "user.maui.expirationEnable", "true", 0);
    AtmosSetUserMetaRequest_add_metadata(&set_request,
            "user.maui.expirationEnd", "2040-01-01T00:00:01Z", 0);
    AtmosResponse_init(&set_response);
    AtmosClient_set_user_meta(&atmos, &set_request, &set_response);
    check_error(&set_response);
    assert_int_equal(200, set_response.parent.http_code);
    AtmosResponse_destroy(&set_response);
    AtmosSetUserMetaRequest_destroy(&set_request);

    // Get info again and check expiration/retention
    AtmosGetObjectInfoResponse_init(&info_response);
    AtmosClient_get_object_info_ns(&atmos, path, &info_response);
    check_error((AtmosResponse*)&info_response);
    assert_int_equal(200, info_response.parent.parent.http_code);
    assert_int_equal(1, info_response.expiration_enabled);
    assert_int64t_equal(2208988801, info_response.expiration_end);
    AtmosGetObjectInfoResponse_destroy(&info_response);


    // Cleanup
    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
    AtmosClient_destroy(&atmos);
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

    fgets(user_id, 255, config);
    fgets(key, 64, config);
    fgets(endpoint, 255, config);
    fgets(uid1, 64, config);
    fgets(uid2, 64, config);
    fgets(proxy_host, 256, config);
    fgets(proxy_port_str, 32, config);
    fgets(proxy_user, 256, config);
    fgets(proxy_pass, 256, config);

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

    srandomdev();

    // Run tests
    test_fixture_start();

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

    start_test_msg("test_get_service_info");
    run_test(test_get_service_info);

    start_test_msg("test_parse_error");
    run_test(test_parse_error);

    start_test_msg("test_create_object");
    run_test(test_create_object);

    start_test_msg("test_create_object_ns");
    run_test(test_create_object_ns);

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

    start_test_msg("test_read_object_with_meta_and_acl");
    run_test(test_read_object_with_meta_and_acl);

    start_test_msg("test_read_object_range");
    run_test(test_read_object_range);

    start_test_msg("test_read_object_file");
    run_test(test_read_object_file);

    start_test_msg("test_get_user_meta");
    run_test(test_get_user_meta);

    start_test_msg("test_get_user_meta_ns");
    run_test(test_get_user_meta_ns);

    start_test_msg("test_delete_user_meta");
    run_test(test_delete_user_meta);

    start_test_msg("test_delete_user_meta_ns");
    run_test(test_delete_user_meta_ns);

    start_test_msg("test_set_user_meta");
    run_test(test_set_user_meta);

    start_test_msg("test_set_user_meta_ns");
    run_test(test_set_user_meta_ns);

    start_test_msg("test_get_system_meta");
    run_test(test_get_system_meta);

    start_test_msg("test_get_system_meta_ns");
    run_test(test_get_system_meta_ns);

    start_test_msg("test_set_get_acl");
    run_test(test_set_get_acl);

    start_test_msg("test_set_get_acl_ns");
    run_test(test_set_get_acl_ns);

    start_test_msg("test_update_object");
    run_test(test_update_object);

    start_test_msg("test_update_object_ns");
    run_test(test_update_object_ns);

    start_test_msg("test_update_object_file");
    run_test(test_update_object_file);

    start_test_msg("test_update_object_file_ns");
    run_test(test_update_object_file_ns);

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

    start_test_msg("test_rename_object");
    run_test(test_rename_object);

    test_fixture_end();

}
