#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32
/* For Sleep() */
#include <Windows.h>
#endif

#include "stdio.h"
#include "transport.h"
#include "atmos_rest.h"
#include "crypto.h"
#include "seatest.h"

char user_id[255];
char key[64];
char endpoint[255];

// For testing ACLs
char uid1[64];
char uid2[64];

// Proxy support
char proxy_host[256];
char proxy_user[256];
char proxy_pass[256];
int proxy_port;

credentials *get_connection() {
	credentials *c = init_ws(user_id, key, endpoint);

	if(strlen(proxy_host)>0) {
		config_proxy(c, proxy_host, proxy_port, proxy_user, proxy_pass);
	}

	return c;
}

void testhmac() {
	const char
			*teststring =
					"POST\napplication/octet-stream\n\nThu, 05 Jun 2008 16:38:19 GMT\n/rest/objects\nx-emc-date:Thu, 05 Jun 2008 16:38:19 GMT\nx-emc-groupacl:other=NONE\nx-emc-listable-meta:part4/part7/part8=quick\nx-emc-meta:part1=buy\nx-emc-uid: 6039ac182f194e15b9261d73ce044939/user1\nx-emc-useracl:john=FULL_CONTROL,mary=WRITE";
	const char *testkey = "LJLuryj6zs8ste6Y3jTGQp71xq0=";
	const char *testresult = "gk5BXkLISd0x5uXw5uIE80XzhVY=";
	char *freeme = HMACSHA1((const unsigned char*) teststring, (char*) testkey,
			strlen(testkey));
	assert_string_equal(testresult, freeme);
	free(freeme);
}

void testbuildhashstring() {
	const char
			*teststring =
					"POST\napplication/octet-stream\n\nThu, 05 Jun 2008 16:38:19 GMT\n/rest/objects\nx-emc-date:Thu, 05 Jun 2008 16:38:19 GMT\nx-emc-groupacl:other=NONE\nx-emc-listable-meta:part4/part7/part8=quick\nx-emc-meta:part1=buy\nx-emc-uid: 6039ac182f194e15b9261d73ce044939/user1\nx-emc-useracl:john=FULL_CONTROL,mary=WRITE";

	int header_count = 0;
	char *headers[6];
	char date[256];
	char acl[256];
	char meta[256];
	char uid[256];
	char useracl[256];
	char listable[256];
	char string[1024 * 1024];

	strcpy(date, "x-emc-date:Thu, 05 Jun 2008 16:38:19 GMT");
	strcpy(acl,"x-emc-groupacl:other=NONE");
	strcpy(listable, "x-emc-listable-meta:part4/part7/part8=quick");
	strcpy(meta, "x-emc-meta:part1=buy");
	strcpy(uid,"x-emc-uid: 6039ac182f194e15b9261d73ce044939/user1");
	strcpy(useracl, "x-emc-useracl:john=FULL_CONTROL,mary=WRITE");

	headers[header_count++] = date;
	headers[header_count++] = acl;
	headers[header_count++] = listable;
	headers[header_count++] = meta;
	headers[header_count++] = uid;
	headers[header_count++] = useracl;

	build_hash_string(string, POST, "application/octet-stream", NULL,
			"Thu, 05 Jun 2008 16:38:19 GMT", "/rest/objects", headers,
			header_count);
	assert_string_equal(teststring, string);
}

void aol_rename() {
	credentials *c = get_connection();
	ws_result result;
	postdata pd;
	char *obj = "/rename_test_object1/a/1/a";
	char *renamed_obj = "1renamed_object";
	char *renamed_obj1 = "/1renamed_object";

	memset(&pd, 0, sizeof(postdata));

	//*** Create
	create_ns(c, obj, NULL, NULL, &pd, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

#ifdef WIN32
	Sleep(2);
#else
	sleep(2);
#endif

	//**rename
	rename_ns(c, obj, renamed_obj, 1, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	//*** Delete - old object should fail
	delete_ns(c, obj, &result);
	assert_int_equal(404, result.return_code);
	result_deinit(&result);

	/** Delete - new object should great success*/
	delete_ns(c, renamed_obj1, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);
	free(c);
}

//cycle through series of tests creating,setting meta data updateing deleting and listing objects
void api_testing() {

	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/trash_test";
	char *body = NULL;
	int hc = 0;
	const int bd_size = 1024 * 64 + 2;// force boundary condistions in readfunction
	char big_data[1024 * 64 + 2];
	char *data = big_data;
	postdata d;
	char *body2 = NULL;

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	//*** LIST
	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	//result body size is not NULL terminated - could be binary
	body = (char*) malloc(result.body_size + 1);
	memcpy(body, result.response_body, result.body_size);
	body[result.body_size] = '\0';
	printf("datum %ld:%s\n", (long) result.body_size, body);
	assert_int_equal(200, result.return_code);
	free(body);
	printf("heads %d\n", result.header_count);
	for (; hc < result.header_count; hc++) {
		printf("%s\n", result.headers[hc]);
	}
	result_deinit(&result);

	//*** Delete

	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	//*** LIST
	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	assert_int_equal(404, result.return_code);
	result_deinit(&result);

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	/*  list_ns(c, testdir,NULL, 0,&result);
	 printf("listed %s\tcode: %d==201\t%s\n", testdir,result.return_code, result.response_body);
	 assert(result.return_code >= 200 && result.return_code < 300);
	 result_deinit(&result);
	 */

	//*** UPDATE
	memset(&d, 0, sizeof(postdata));
	memset(big_data, 0, bd_size);
	d.data = data;
	d.body_size = bd_size;
#ifdef WIN32
	Sleep(5);
#else
	sleep(5);
#endif    
	result_init(&result);
	update_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	//*** LIST
	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);

	body2 = (char*) malloc(result.body_size + 1);
	memcpy(body2, result.response_body, result.body_size);
	body2[result.body_size] = '\0';
	printf("datum %ld:%s\n", (long) result.body_size, body2);
	free(body2);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	//*** Delete

	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	//*** LIST
	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	assert_int_equal(404, result.return_code);
	result_deinit(&result);
	free(c);
}

void simple_update() {
	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/updateest";
	const int bd_size = 1024 * 64 + 2;// force boundary condistions in readfunction
	char big_data[1024 * 64 + 2];
	char *data = big_data;
	postdata d;

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	//*** U&PDATE
	memset(&d, 0, sizeof(postdata));
	memset(big_data, 0, bd_size);
	d.data = data;
	d.body_size = bd_size;
	update_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	delete_ns(c, testdir, &result);
	result_deinit(&result);
	free(c);
}

void print_system_meta(system_meta sm) {

	printf("atime\t%s\n", sm.atime);
	printf("mtime\t%s\n", sm.mtime);
	printf("ctime\t%s\n", sm.ctime);
	printf("itime\t%s\n", sm.itime);
	printf("type\t%s\n", sm.type);
	printf("uid\t%s\n", uid);
	printf("gid\t%s\n", sm.gid);
	printf("objectid\t[%s]\n", sm.objectid);
	printf("objname\t%s\n", sm.objname);
	printf("size\t%lld\n", sm.size);
	printf("nlink\t%d\n", sm.nlink);
	printf("policyname\t%s\n", sm.policyname);

}

void set_meta_data() {

	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/Resources.tgz1";
	user_meta *um = NULL;
	system_meta sm;
	user_meta *t;
	postdata d;

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);
	t = new_user_meta("meta_test", "meta_pass", 1);
	add_user_meta(t, "1_test", "1_pass", 0);
	add_user_meta(t, "2_test", "2_pass", 0);
	add_user_meta(t, "3_test", "3_pass", 1);
	add_user_meta(t, "4_test", "4_pass", 1);
	add_user_meta(t, "5_test", "5_pass", 1);
	add_user_meta(t, "6_test", "6_pass", 0);
	user_meta_ns(c, testdir, NULL, t, &result);

	free_user_meta(t);
	result_deinit(&result);
	printf("send metadata\n");

	get_meta_ns(c, testdir, &result);

	printf("fetched metadata\n");

	parse_headers(&result, &sm, &um);

	print_system_meta(sm);

	result_deinit(&result);
	printf("%s\n", sm.objname);

	int mcount = 0;
	while (um != NULL) {
		user_meta *t = um->next;
		printf("%s=%s %d\n", um->key, um->value, um->listable);
		free(um);
		um = t;
		mcount++;
	}

	assert_int_equal(7, mcount);

	delete_ns(c, testdir, &result);
	result_deinit(&result);
	free(c);
}

//create -> list -> delete
void create_test() {

	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/FUSETEST/";
	system_meta sm;
	user_meta *um = NULL;
	postdata d;

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	// Already exists
	assert_int_equal(400, result.return_code);
	result_deinit(&result);

	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	assert_int_equal(200, result.return_code);

	memset(&sm, 0, sizeof(sm));
	parse_headers(&result, &sm, &um);
	result_deinit(&result);

	//*** Delete

	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	free(c);
}

void list() {
	credentials *c = get_connection();

	//char *testdir="/apache/A503558331078fce88fc/.mauiws/meta_test/";//
	char *testdir = "/testdir/";
	char *testfile = "/testdir/testfile";//

	ws_result result;
	char big_data[2048];
	const int bd_size = 2048;
	postdata d;
	char *char_response_bucket = NULL;

	memset(&d, 0, sizeof(postdata));
	create_ns(c, testfile, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);
	//*** UPDATE
	memset(&d, 0, sizeof(postdata));
	memset(big_data, 0, bd_size);
	strcpy(big_data,"many listers\n");
	d.data = big_data;
	d.body_size = strlen(big_data);
	update_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	assert_int_equal(200, result.return_code);
	if (result.response_body) {
		char_response_bucket = malloc(result.body_size + 1);
		memcpy(char_response_bucket, result.response_body, result.body_size);
		char_response_bucket[result.body_size] = '\0';
		printf("%s\n", char_response_bucket);
		free(char_response_bucket);
	} else {
		assert_fail("error in list no response\n");
	}

	result_deinit(&result);

	delete_ns(c, testdir, &result);
	// Directory not empty
	assert_int_equal(400, result.return_code);
	result_deinit(&result);

	delete_ns(c, testfile, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	free(c);
}

void capstest() {
	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/IAMCAPS";
	system_meta sm;
	user_meta *um = NULL;
	postdata d;

	//*** Create
	memset(&d, 0, sizeof(postdata));
	create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	assert_int_equal(200, result.return_code);
	memset(&sm, 0, sizeof(sm));
	parse_headers(&result, &sm, &um);
	result_deinit(&result);

	//*** Delete

	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	free(c);

}

void rangestest() {

	credentials *c = get_connection();
	ws_result result;
	char *testdir = "/test123456/afile2";
	system_meta sm;
	user_meta *um = NULL;
	postdata pd;
	postdata rd;

	memset(&pd, 0, sizeof(pd));

	pd.data = malloc(32);
	memset(pd.data,5, 32);
	pd.body_size = 32;
	//pd.offset=31;

	//*** Create

	create_ns(c, testdir, NULL, NULL, &pd, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	//Now grow the object
	memset(pd.data,1, 32);
	pd.body_size = 32;
	pd.offset = 31;
	update_ns(c, testdir, NULL, NULL, &pd, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	rd.offset = 31;
	rd.body_size = 32;
	read_obj_ns(c, testdir, &rd, &result);

	memset(&sm, 0, sizeof(sm));
	parse_headers(&result, &sm, &um);
	while (um != NULL) {
		user_meta *t = um->next;
		free(um);
		um = t;
	}

	result_deinit(&result);

	//*** Delete
	delete_ns(c, testdir, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	free(pd.data);
	free(c);

}

void truncate_obj() {
	credentials *c = get_connection();
	ws_result result;
	char *testfile = "/truncate";
	char *testdata = "12345678901234567890123456789012"; // 32 bytes
	postdata pd;
	postdata rd;

	memset(&pd, 0, sizeof(pd));

	pd.data = malloc(32);

	//memset(pd.data,5, 32);
	memcpy(pd.data, testdata, strlen(testdata));
	pd.body_size = strlen(testdata);
	//*** Create

	result_init(&result);
	create_ns(c, testfile, NULL, NULL, NULL, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	result_init(&result);
	update_ns(c, testfile, NULL, NULL, &pd, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	memset(&rd, 0, sizeof(postdata));
	rd.offset = 31;
	rd.body_size = 1;
	read_obj_ns(c, testfile, &rd, &result);
	// 206 Partial content
	assert_int_equal(206, result.return_code);
	result_deinit(&result);

	result_init(&result);
	// Truncate
	pd.body_size = 0;
	pd.offset = -1;
	update_ns(c, testfile, NULL, NULL, &pd, NULL, &result);
	assert_int_equal(200, result.return_code);
	result_deinit(&result);

	result_init(&result);
	read_obj_ns(c, testfile, &rd, &result);
	// Range should now be unsatisfiable
	assert_int_equal(416, result.return_code);
	result_deinit(&result);
	//*** Delete

	result_init(&result);
	delete_ns(c, testfile, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);

	free(pd.data);
	free(c);

}
void testacl() {
	credentials *c = get_connection();
	ws_result result;
	char *obj = "/acltest";
	acl *acllist, *acllist2, *acllist3;

	acllist = (acl*) malloc(sizeof(acl));
	memset(acllist, 0, sizeof(acl));
	strcpy(acllist->user,"john");
	strcpy(acllist->permission, ACL_FULL);

	acllist2 = (acl*) malloc(sizeof(acl));
	memset(acllist2, 0, sizeof(acl));
	acllist->next = acllist2;
	strcpy(acllist2->user,"mary");
	strcpy(acllist2->permission, ACL_READ);

	acllist3 = (acl*) malloc(sizeof(acl));
	memset(acllist3, 0, sizeof(acl));
	acllist2->next = acllist3;
	acllist3->is_group = 1;
	strcpy(acllist3->user, ACL_GROUP_PUBLIC);
	strcpy(acllist3->permission, ACL_READ);

	//*** Create acl
	create_ns(c, obj, NULL, acllist, NULL, NULL, &result);
	assert_int_equal(201, result.return_code);
	result_deinit(&result);

	//*** Delete - new object should great success
	delete_ns(c, obj, &result);
	assert_int_equal(204, result.return_code);
	result_deinit(&result);
	free(acllist);
	free(acllist2);
	free(c);
}

void get_sys_info() {
	credentials *c = get_connection();
	ws_result result;
	get_service_info(c, &result);
	assert_int_equal(200, result.return_code);
	printf("%s\n", result.response_body);

}

void test_error() {
	char failmsg[1024];
	credentials *c = init_ws(user_id, key, "this.host.does.not.exist");
	c->curl_verbose = 1;
	ws_result result;
	printf("Testing bad hostname...");
	get_service_info(c, &result);
	if (result.return_code == 0) {
		if (result.curl_error_code == 6) {
			printf("OK\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
					result.return_code, result.curl_error_code,
					result.curl_error_message);
		} else {
			sprintf(failmsg,
					"FAIL got code %d expected 6\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
					result.curl_error_code, result.return_code,
					result.curl_error_code, result.curl_error_message);
			assert_fail(failmsg)
				;
		}
	} else {
		sprintf(failmsg,
				"FAIL (Expected error)\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
				result.return_code, result.curl_error_code,
				result.curl_error_message);
		assert_fail(failmsg)
			;
	}
	result_deinit(&result);
	free_ws(c);

	char badporthost[1024];
	sprintf(badporthost, "%s:1443", endpoint);
	//printf("badporthost: %s\n", badporthost);
	c = init_ws(user_id, key, badporthost);
	c->curl_verbose = 1;
	printf("Testing bad port (wait for timeout)...");
	fflush(stdout);
	get_service_info(c, &result);
	if (result.return_code == 0) {
		if (result.curl_error_code == 7) {
			//			printf("OK\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
			//					result.return_code, result.curl_error_code,
			//					result.curl_error_message);
		} else {
			sprintf(failmsg,
					"FAIL got code %d expected 7\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
					result.curl_error_code, result.return_code,
					result.curl_error_code, result.curl_error_message);
			assert_fail(failmsg)
				;
		}
	} else {
		sprintf(failmsg,
				"FAIL (Expected error)\nHTTP: %ld CURLCode: %d CURL Error: %s\n",
				result.return_code, result.curl_error_code,
				result.curl_error_message);
		assert_fail(failmsg)
			;
	}
	result_deinit(&result);
	free_ws(c);
}

static int i_was_called;

int curl_test_callback(credentials *c, CURL *handle) {
	i_was_called = 1;
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
	return 0;
}

int curl_test_callback_abort(credentials *c, CURL *handle) {
	// Just abort
	return 1;
}

void test_curl_callback(void) {
	credentials *c = get_connection();
	set_config_callback(c, curl_test_callback);

	i_was_called = 0;
	ws_result result;
	result_init(&result);
	get_service_info(c, &result);
	assert_int_equal(200, result.return_code);
	assert_int_equal(1, i_was_called);
	result_deinit(&result);

	i_was_called = 0;
	set_config_callback(c, curl_test_callback_abort);
	result_init(&result);
	get_service_info(c, &result);
	assert_int_equal(0, result.return_code);
	assert_int_equal(CURLE_ABORTED_BY_CALLBACK, result.curl_error_code);
	assert_int_equal(0, i_was_called);
}

static int retrycount;
static char correctsecret[255];

int curl_test_retry_callback(credentials *c, postdata *d,
		ws_result *result) {
	if(retrycount != 0) {
		fprintf(stderr, "Retry count > 0!\n");
		return 0;
	}

	// Increment retrycount
	retrycount++;

	assert_int_equal(403, result->return_code);

	// Fix the credentials object.
	strcpy(c->secret, correctsecret);

	// Clear the results object
	result_init(result);

	return 1;
}

void test_retry_callback(void) {
	credentials *c = get_connection();
	ws_result result;
	user_meta *t;
	char obj_id[64];

	// Save the secret and then break it
	strcpy(correctsecret, c->secret);
	strcpy(c->secret, "THISISNOTAREALSECRETKEYSTRN=");

	t = new_user_meta("meta_test", "meta_pass", 1);
	add_user_meta(t, "1_test", "1_pass", 0);
	add_user_meta(t, "2_test", "2_pass", 0);
	add_user_meta(t, "3_test", "3_pass", 1);
	add_user_meta(t, "4_test", "4_pass", 1);
	add_user_meta(t, "5_test", "5_pass", 1);
	add_user_meta(t, "6_test", "6_pass", 0);


	// Set the retry handler
	retrycount = 0;
	set_retry_callback(c, curl_test_retry_callback);
	result_init(&result);

	create_obj(c, obj_id, "text/plain", NULL, t, &result);

	assert_int_equal(201, result.return_code);
	assert_int_equal(1, retrycount);

	free_user_meta(t);
}

void start_test_msg(const char *test_name) {
	printf("\nTEST: %s\n", test_name);
	printf("================\n");
	fflush(stdout);
}

void all_tests(void) {
	test_fixture_start();
	start_test_msg("testbuildhashstring");
	run_test(testbuildhashstring);
	start_test_msg("testhmac");
	run_test(testhmac);
	start_test_msg("get_sys_info");
	run_test(get_sys_info);
	start_test_msg("test_error");
	run_test(test_error);
	start_test_msg("aol_rename");
	run_test(aol_rename);
	start_test_msg("set_meta_data");
	run_test(set_meta_data);
	start_test_msg("testacl");
	run_test(testacl);
	start_test_msg("truncate_obj");
	run_test(truncate_obj);
	start_test_msg("api_testing");
	run_test(api_testing);
	start_test_msg("list");
	run_test(list);
	start_test_msg("simple_update");
	run_test(simple_update);
	start_test_msg("rangestest");
	run_test(rangestest);
	start_test_msg("create_test");
	run_test(create_test);
	start_test_msg("capstest");
	run_test(capstest);
	start_test_msg("test_curl_callback");
	run_test(test_curl_callback);
	start_test_msg("test_retry_callback");
	run_test(test_retry_callback);
	test_fixture_end();
}

int main() {
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

	return run_tests(all_tests);
}
