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

static const char *filechars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~!$&\"()*+,;=:";

static void random_file(char *name, int count) {
    int i=0;
    int num_filechars = strlen(filechars);
    for(;i<count;i++) {
        name[i] = filechars[random() % num_filechars];
    }
    name[i] = 0;
}

void check_error(AtmosResponse *response) {
    RestResponse *rest_response = (RestResponse*)response;
    if(rest_response->http_code == 0) {
        // Transport-level error
        printf("Connection error %d: %s\n", rest_response->curl_error,
                rest_response->curl_error_message);
    } else if(rest_response->http_code > 399) {
        printf("HTTP error %d: %s\n", rest_response->http_code,
                rest_response->http_status);
    }
    if(response->atmos_error > 0) {
        printf("Atmos error %d: %s\n", response->atmos_error,
                response->atmos_error_message);
    }
}

static const char *string_to_sign =
        "POST\n"
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
    RestClient_add_curl_config_handler((RestClient*)atmos, rest_verbose_config);

    if(strlen(proxy_host) > 0) {
        if(strlen(proxy_user) > 0) {
            RestClient_set_proxy((RestClient*)atmos, proxy_host, proxy_port, proxy_user, proxy_pass);
        } else {
            RestClient_set_proxy((RestClient*)atmos, proxy_host, proxy_port, NULL, NULL);
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
    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta: title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta : title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:  title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:   title=Moutain Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:title=Moutain  Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:title=Moutain   Dew",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew ",
            "x-emc-meta:title=Moutain Dew");

    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew, type=White Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew,type=White Out",
            "x-emc-meta:title=Moutain Dew,type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew , type=White Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain Dew, type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain  Dew, type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain  Dew , type=White  Out",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title=Moutain   Dew  , type=White  Out  ",
            "x-emc-meta:title=Moutain Dew, type=White Out");

    do_test_whitespace(
            "x-emc-meta:title= Moutain   Dew  , type= White  Out  ",
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
    RestRequest_add_header(&request, "x-emc-uid:6039ac182f194e15b9261d73ce044939/user1");
    RestRequest_add_header(&request, "x-emc-meta:part1=buy");
    RestRequest_add_header(&request, "x-emc-listable-meta:part4/part7/part8=quick");
    RestRequest_add_header(&request, "x-emc-useracl:john=FULL_CONTROL,mary=WRITE");
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

    check_error((AtmosResponse*)&res);
    assert_int_equal(200, res.parent.parent.http_code);
    assert_true(strlen(res.version) > 0);
    printf("Atmos Version %s\n", res.version);

    if(res.version[0] >= 50) { // '2'
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

void test_create_object() {
    AtmosClient atmos;
    AtmosCreateObjectResponse response;
    RestResponse delete_response;

    get_atmos_client(&atmos);
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple(&atmos, "test", 4, "text/plain", &response);

    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object(&atmos, response.object_id, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
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
    atmos.signature_debug = 1;
    AtmosCreateObjectResponse_init(&response);

    AtmosClient_create_object_simple_ns(&atmos, path, "test", 4,
            "text/plain", &response);

    assert_true(strlen(response.object_id) > 0);

    printf("Created object: %s\n", response.object_id);

    RestResponse_init(&delete_response);
    AtmosClient_delete_object_ns(&atmos, path, &delete_response);
    assert_int_equal(204, delete_response.http_code);
    RestResponse_destroy(&delete_response);

    AtmosCreateObjectResponse_destroy(&response);
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

    if(strlen(proxy_host)>0) {
        proxy_host[strlen(proxy_host)-1] = 0;
    }
    if(strlen(proxy_user)>0) {
        proxy_user[strlen(proxy_user)-1] = 0;
    }
    if(strlen(proxy_pass)>0) {
        proxy_pass[strlen(proxy_pass)-1] = 0;
    }
    if(strlen(proxy_port_str)>1) {
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

    start_test_msg("test_get_service_info");
    run_test(test_get_service_info);

    start_test_msg("test_parse_error");
    run_test(test_parse_error);

    start_test_msg("test_create_object");
    run_test(test_create_object);

    start_test_msg("test_create_object_ns");
    run_test(test_create_object_ns);

    test_fixture_end();

}
