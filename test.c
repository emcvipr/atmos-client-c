#include <string.h>
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
/* For Sleep() */
#include <Windows.h>
#endif

#include "stdio.h"
#include "transport.h"
#include "atmos_rest.h"
#include "crypto.h"

/*    static const char *user_id = "whitewater";
      static const char *key = "EIJHmj9JZSGVFQ2Hsl/scAsKm00=";
      static const char *endpoint = "10.245.35.162";  //*/

static const char *user_id = "9e21feebb9f5455f99cd1ad8c76ebe1e/EMC0056CC2B8DDD13D60";
static const char *key = "1dz6W6m2GJLcF54xHQfkfyomPrA=";
//static const char *endpoint = "127.0.0.1";//"accesspoint.emccis.com";//*/
static const char *endpoint = "accesspoint.emccis.com";//*/


//hmac validater

void testhmac() {
    const char *teststring="POST\napplication/octet-stream\n\nThu, 05 Jun 2008 16:38:19 GMT\n/rest/objects\nx-emc-date:Thu, 05 Jun 2008 16:38:19 GMT\nx-emc-groupacl:other=NONE\nx-emc-listable-meta:part4/part7/part8=quick\nx-emc-meta:part1=buy\nx-emc-uid: 6039ac182f194e15b9261d73ce044939/user1\nx-emc-useracl:john=FULL_CONTROL,mary=WRITE";
    const char *testkey="LJLuryj6zs8ste6Y3jTGQp71xq0=";
    const char *testresult="gk5BXkLISd0x5uXw5uIE80XzhVY=";
    char *freeme = HMACSHA1((const unsigned char*)teststring, (char*)testkey, strlen(testkey));
    assert(strcmp(freeme,testresult)==0);
    free(freeme);
    printf("finished testhmac\n");
}

void testbuildhashstring() {
    const char *teststring="POST\napplication/octet-stream\n\nThu, 05 Jun 2008 16:38:19 GMT\n/rest/objects\nx-emc-date:Thu, 05 Jun 2008 16:38:19 GMT\nx-emc-groupacl:other=NONE\nx-emc-listable-meta:part4/part7/part8=quick\nx-emc-meta:part1=buy\nx-emc-uid: 6039ac182f194e15b9261d73ce044939/user1\nx-emc-useracl:john=FULL_CONTROL,mary=WRITE";
    
    int header_count=0;
    char *headers[6];
    char date[256];
    char acl[256];
    char meta[256];
    char uid[256];
    char useracl[256];
    char listable[256];
    char string[1024*1024];

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


    build_hash_string(string, POST,"application/octet-stream",NULL,"Thu, 05 Jun 2008 16:38:19 GMT", "/rest/objects",headers,header_count);
    assert(strcmp(string, teststring) == 0 );
    printf("%s\n", string);
    printf("Finished building hash string\n");
}

void aol_rename() {
    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    postdata pd;
    char *obj = "/rename_test_object1/a/1/a";
    char *renamed_obj = "1renamed_object";
    char *renamed_obj1 = "/1renamed_object";

    memset(&pd, 0, sizeof(postdata));

    //*** Create
    create_ns(c, obj, NULL, NULL, &pd, NULL, &result);
    printf("code: %d==201\t%s\n", result.return_code, result.response_body);
    result_deinit(&result);
    
#ifdef WIN32
	Sleep(2);
#else
    sleep(2);
#endif

    //**rename
    rename_ns(c, obj, renamed_obj, 1, &result);
    printf("rename code: %d==200\t%s\n", result.return_code, result.response_body);
    result_deinit(&result);
    
    //*** Delete - old object should fail
    delete_ns(c, obj, &result);
    printf("code: %d!=204\n", result.return_code);
    result_deinit(&result);

    /** Delete - new object should great success*/
    delete_ns(c, renamed_obj1, &result);
    printf("code: %d==204\n", result.return_code);
    result_deinit(&result);
    free(c);
}

//cycle through series of tests creating,setting meta data updateing deleting and listing objects
int api_testing(){

    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir = "/trash_test";
    char *body = NULL;
    int hc = 0;
    const int bd_size = 1024*64+2;// force boundary condistions in readfunction
    char big_data[1024*64+2];
    char *data = big_data;
    postdata d;
    char *body2 = NULL;

    //*** Create
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("code: %d==201\t%s\n", result.return_code, result.response_body);
    result_deinit(&result);


    //*** LIST
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    //result body size is not NULL terminated - could be binary
    body = (char*)malloc(result.body_size+1);
    memcpy(body, result.response_body, result.body_size);
    body[result.body_size] = '\0';
    printf("datum %ld:%s\n", (long)result.body_size, body);
    printf("code: %d==200\n", result.return_code);
    assert(result.return_code ==200);
    free(body);    
    printf("heads %d\n", result.header_count);
    for(; hc < result.header_count; hc++) {
	printf("%s\n",result.headers[hc]);
    }    
    result_deinit(&result);

    //*** Delete
  
    delete_ns(c, testdir, &result);
    printf("code: %d==204\n", result.return_code);
    assert(result.return_code ==204);
    result_deinit(&result);

    //*** LIST
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    printf("code: %d==404\n", result.return_code);
    assert(result.return_code ==404);
    result_deinit(&result);

    //*** Create  
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("created %s\tcode: %d==201\t%s\n", testdir,result.return_code, result.response_body);
    assert(result.return_code ==201);
    result_deinit(&result);

    /*  list_ns(c, testdir,NULL, 0,&result);    
	printf("listed %s\tcode: %d==201\t%s\n", testdir,result.return_code, result.response_body);
	assert(result.return_code >= 200 && result.return_code < 300);
	result_deinit(&result);
    */  

    //*** UPDATE  
    memset(&d, 0, sizeof(postdata));
    memset(big_data, 0, bd_size);
    d.data=data;
    d.body_size = bd_size;
#ifdef WIN32
	Sleep(5);
#else
    sleep(5);
#endif    
    result_init(&result);
    update_ns(c, testdir,NULL, NULL, &d, NULL,&result);
    printf("%s\tupdate code: %d==200\t%s\n", testdir,result.return_code, result.response_body);
    assert(result.return_code ==200);
    result_deinit(&result);

    //*** LIST
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    
    body2 = (char*)malloc(result.body_size+1);
    memcpy(body2, result.response_body, result.body_size);
    body2[result.body_size] = '\0';
    printf("datum %ld:%s\n", (long)result.body_size,body2);
    free(body2);
    printf("code: %d\n", result.return_code);
    result_deinit(&result);

    //*** Delete
  
    delete_ns(c, testdir, &result);
    printf("code: %d\n", result.return_code);
    result_deinit(&result);

    //*** LIST
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    printf("code: %d\n", result.return_code);
    result_deinit(&result);
    free(c);
}
void simple_update() {
    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir = "/updateest";
    char *body = NULL;
    int hc = 0;
    const int bd_size = 1024*64+2;// force boundary condistions in readfunction
    char big_data[1024*64+2];
    char *data = big_data;
    postdata d;
    char *body2 = NULL;

    //*** Create
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("code: %d==201\n", result.return_code);
    result_deinit(&result);

    //*** U&PDATE  
    memset(&d, 0, sizeof(postdata));
    memset(big_data, 0, bd_size);
    d.data=data;
    d.body_size = bd_size;
    update_ns(c, testdir,NULL, NULL, &d, NULL,&result);    
    printf("update code: %d==200\n", result.return_code);
    assert(result.return_code ==200);
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
    printf("type\t%s\n", sm.type );
    printf("uid\t%s\n",uid );
    printf("gid\t%s\n", sm.gid);
    printf("objectid\t[%s]\n", sm.objectid );
    printf("objname\t%s\n", sm.objname);
    printf("size\t%lld\n", sm.size);
    printf("nlink\t%d\n", sm.nlink);
    printf("policyname\t%s\n", sm.policyname);
  
}


void set_meta_data() {

    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir = "/Resources.tgz1";
    user_meta meta,meta1, meta2, meta3;
    user_meta *um = NULL;
    system_meta sm;
    user_meta *t;
    postdata d;

    //*** Create
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL,NULL, &d, NULL, &result);
    printf("code: %d\n", result.return_code);
    result_deinit(&result);
    t = new_user_meta("meta_test", "meta_pass", 1);
    add_user_meta(t, "1_test", "1_pass", 0);
    add_user_meta(t, "2_test", "2_pass", 0);
    add_user_meta(t, "3_test", "3_pass", 0);
    add_user_meta(t, "4_test", "4_pass", 0);
    add_user_meta(t, "5_test", "5_pass", 0);
    add_user_meta(t, "6_test", "6_pass", 0);
    user_meta_ns(c, testdir, NULL, t, &result);

    free_user_meta(t);
    result_deinit(&result);
    printf("send metadata\n");

  
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    
    printf("fetched` metadata\n");
    
    parse_headers(&result, &sm, &um);    
    
    print_system_meta(sm);

    result_deinit(&result);
    printf("%s\n", sm.objname);
    while(um != NULL) {
	user_meta *t = um->next;
	printf("%s=%s %d\n", um->key, um->value, um->listable);
	free(um);
	um=t;
    }
  
    delete_ns(c, testdir, &result);  
    result_deinit(&result);
    free(c);
}


//create -> list -> delete
void create_test() {

    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir="/FUSETEST";
    system_meta sm;
    user_meta *um = NULL;
    postdata d;



    //*** Create
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("result %d\n", result.return_code);
    result_deinit(&result);

  
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("result %d\n ", result.return_code);
    result_deinit(&result);
  
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);

    memset(&sm, 0, sizeof(sm));
    parse_headers(&result, &sm, &um);
    result_deinit(&result);

    //*** Delete
  
    delete_ns(c, testdir, &result);
    result_deinit(&result);


    free(c);
}

void list() {
    credentials *c = init_ws(user_id, key, endpoint);

    //char *testdir="/apache/A503558331078fce88fc/.mauiws/meta_test/";//
    char *testdir = "/desktop.ini2";//
    int loops=0;
    ws_result result;
    char big_data[2048];
    const int bd_size = 2048;
    postdata d;
    char *char_response_bucket= NULL;


    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    printf("%d\n", result.return_code);
    result_deinit(&result);
    //*** UPDATE  
    memset(&d, 0, sizeof(postdata));
    memset(big_data, 0, bd_size);
    strcpy(big_data,"many listers\n");
    d.data=big_data;
    d.body_size = strlen(big_data);
    update_ns(c, testdir,NULL, NULL, &d, NULL,&result);    
    printf("update code: %d==200\n", result.return_code);
    assert(result.return_code ==200);
    result_deinit(&result);


    for(loops;loops < 10; loops++) { 
        list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
	if(result.response_body){
	    char_response_bucket = malloc(result.body_size+1);
	    memcpy(char_response_bucket, result.response_body, result.body_size);
	    char_response_bucket[result.body_size] = '\0';
	    printf("%s\n", char_response_bucket);
	    free(char_response_bucket);
	} else  {
	    printf("error in list no response\n");
	}
      
	result_deinit(&result);
    }
    free(c);
}
      

void capstest() {
    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir="/IAMCAPS";
    system_meta sm;
    user_meta *um = NULL;
    postdata d;

    //*** Create
    memset(&d, 0, sizeof(postdata));
    create_ns(c, testdir, NULL, NULL, &d, NULL, &result);
    result_deinit(&result);

  
    list_ns(c, testdir, NULL, 0, 0, NULL, NULL, &result);
    memset(&sm, 0, sizeof(sm));
    parse_headers(&result, &sm, &um);
    result_deinit(&result);

    //*** Delete
  
    delete_ns(c, testdir, &result);
    result_deinit(&result);

    free(c);


}

void rangestest() {

    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testdir="/test123456/afile2";
    system_meta sm;
    user_meta *um = NULL;	
    postdata pd;
    postdata rd;

    memset(&pd, 0, sizeof(pd));

    pd.data = malloc(32);
    memset(pd.data,5, 32);
    pd.body_size=32;
    //pd.offset=31;

    //*** Create
  
    create_ns(c, testdir, NULL, NULL, &pd, NULL, &result);
    result_deinit(&result);

    //Now grow the object
    memset(pd.data,1, 32);
    pd.body_size=32;
    pd.offset=31;
    update_ns(c, testdir,NULL, NULL, &pd, NULL,&result);    
    result_deinit(&result);

  
    rd.offset=31;
    rd.body_size=32;
    read_obj_ns(c, testdir, &rd, &result);

    memset(&sm, 0, sizeof(sm));
    parse_headers(&result, &sm, &um);
    while(um != NULL) {
	user_meta *t = um->next;
	free(um);
	um=t;
    }
    
    result_deinit(&result);

    //*** Delete
    delete_ns(c, testdir, &result);
    result_deinit(&result);

    free(pd.data);
    free(c);


}

void truncate_obj() {
    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *testfile="/truncate";
    system_meta sm;
    user_meta *um = NULL;	
    postdata pd;
    postdata rd;

    memset(&pd, 0, sizeof(pd));

    pd.data = malloc(32);
  
    //memset(pd.data,5, 32);
    memcpy(pd.data, testfile, strlen(testfile));
    pd.body_size=strlen(testfile);
    //*** Create

    result_init(&result);
    create_ns(c, testfile, NULL,NULL, NULL, NULL, &result);
    result_deinit(&result);
  
    result_init(&result);
    update_ns(c, testfile,NULL, NULL, &pd, NULL,&result);    
    result_deinit(&result);

    memset(&rd, 0, sizeof(postdata));
    rd.offset=31;
    rd.body_size=32;
    read_obj_ns(c, testfile, &rd, &result);
    result_deinit(&result);

    result_init(&result);
    update_ns(c, testfile, NULL, NULL, NULL, NULL, &result);
    result_deinit(&result);

    result_init(&result);
    read_obj_ns(c, testfile, &rd, &result);
    result_deinit(&result);
    //*** Delete
  
    result_init(&result);
    delete_ns(c, testfile, &result);
    result_deinit(&result);

    free(pd.data);
    free(c);



}
void testacl() {
    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *obj = "/rename_test_object1";
    char *renamed_obj = "1renamed_object";
	acl *acllist, *acllist2, *acllist3;
    
    acllist = (acl*)malloc(sizeof(acl));
    memset(acllist, 0, sizeof(acl));
    strcpy(acllist->user,"john");
    strcpy(acllist->permission, ACL_FULL);

    acllist2 = (acl*)malloc(sizeof(acl));
    memset(acllist2, 0, sizeof(acl));
    acllist->next = acllist2;
    strcpy(acllist2->user,"mary");
    strcpy(acllist2->permission, ACL_READ);

    acllist3 = (acl*)malloc(sizeof(acl));
    memset(acllist3, 0, sizeof(acl));
    acllist2->next = acllist3;
    acllist3->is_group = 1;
    strcpy(acllist3->user, ACL_GROUP_PUBLIC);
    strcpy(acllist3->permission, ACL_READ);
    
    //*** Create acl
    create_ns(c, obj, NULL, acllist, NULL, NULL, &result);
    printf("code: %d==201\t%s\n", result.return_code, result.response_body);
    result_deinit(&result);
    
    
    //*** Delete - new object should great success
    delete_ns(c, obj, &result);
    printf("code: %d==204\n", result.return_code);
    result_deinit(&result);
    free(acllist);
    free(acllist2);
    free(c);
}

void get_sys_info() {
   credentials *c = init_ws(user_id, key, endpoint);
   ws_result result;
   get_service_info(c, &result);
   printf("%s\n", result.response_body);
   
}
int main() { 

    if(user_id) {
	get_sys_info();
	aol_rename();
	set_meta_data();
	testacl();
	set_meta_data();
	testacl();
	truncate_obj();
	api_testing();
	list();
	simple_update();

	rangestest();
	create_test();
	capstest();
	api_testing();
	testbuildhashstring();
	testhmac();
	//*/ 
    } else {
	printf("please edit test.c and add your credentials for user_id and shared_secret\n");
    }
}
