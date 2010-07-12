#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "stdio.h"
#include "transport.h"
#include "atmos_rest.h"
#include "crypto.h"

static const char *user_id = "604373f7c99b404caa2e430626f74977/mail";
static const char *key = "w7mxRvPlDYUkA4J6uTuItfUS1u4=";
static const char *endpoint = "10.241.38.90";


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

//cycle through series of tests creating,setting meta data updateing deleting and listing objects
int api_testing(){

  credentials *c = init_ws(user_id, key, endpoint);
  ws_result result;
  char *testdir = "/.Trash_test";
  char *body = NULL;
  int hc = 0;
  const int bd_size = 1024*64+2;// force boundary condistions in readfunction
  char big_data[1024*64+2];
  char *data = big_data;
  postdata d;
  char *body2 = NULL;

  //*** Create
  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("code: %d==201\n", result.return_code);
  result_deinit(&result);


  //*** LIST
  
  list_ns(c, testdir,NULL, 0,&result);    
  //result body size is not NULL terminated - could be binary
  body = malloc(result.body_size+1);
  memcpy(body, result.response_body, result.body_size);
  body[result.body_size] = '\0';
  printf("datum%d:%s\n", result.body_size,body);
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
  
  list_ns(c, testdir, NULL, 0,&result);    
  printf("code: %d==404\n", result.return_code);
  assert(result.return_code ==404);
  result_deinit(&result);

  //*** Create  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("code: %d==201\n", result.return_code);
  assert(result.return_code ==201);
  result_deinit(&result);

  //*** UPDATE  
  memset(&d, 0, sizeof(postdata));
  memset(big_data, 0, bd_size);
  d.data=data;
  d.body_size = bd_size;
  update_ns(c, testdir,NULL, NULL, &d, NULL,&result);    
  printf("update code: %d==200\n", result.return_code);
  assert(result.return_code ==200);
  result_deinit(&result);

  //*** LIST
  
  list_ns(c, testdir,NULL, 0,&result);    
    
  body2=malloc(result.body_size+1);
  memcpy(body2, result.response_body, result.body_size);
  body2[result.body_size] = '\0';
  printf("datum%d:%s\n", result.body_size,body2);
  free(body2);
  printf("code: %d\n", result.return_code);
  result_deinit(&result);

  //*** Delete
  
  delete_ns(c, testdir, &result);
  printf("code: %d\n", result.return_code);
  result_deinit(&result);

  //*** LIST
  
  list_ns(c, testdir,NULL, 0, &result);    
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
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
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
  printf("size\t%d\n", sm.size);
  printf("nlink\t%d\n", sm.nlink);
  printf("policyname\t%s\n", sm.policyname);
  
}


void set_meta_data() {

  credentials *c = init_ws(user_id, key, endpoint);
  ws_result result;
  char *testdir = "/Resources.tgz1";
  user_meta meta,meta1, meta2, meta3;
  user_meta *um = NULL;
  system_meta sm ;

  //*** Create
  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("code: %d\n", result.return_code);
  result_deinit(&result);
    
  //** update_meta

  memset(&meta, 0, sizeof(user_meta));
  memset(&meta1, 0, sizeof(user_meta));
  memset(&meta2, 0, sizeof(user_meta));
  memset(&meta3, 0, sizeof(user_meta));
  strcpy(meta.key, "meta_test");
  strcpy(meta.value, "meta_pass");
  meta.listable=true;
  strcpy(meta1.key, "1_test");
  strcpy(meta1.value, "1_pass");
  strcpy(meta2.key, "2_test");
  strcpy(meta2.value, "2_pass");
  strcpy(meta3.key, "3_test");
  strcpy(meta3.value, "3_pass");
    
  

  meta.next=&meta1;
  meta1.next=&meta2;
  meta2.next=&meta3;
  user_meta_ns(c, testdir, NULL, &meta, &result);
  result_deinit(&result);
  printf("send metadata\n");

  
  list_ns(c, testdir, NULL, 0, &result);    
    
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
  
      
  free(c);
}


//create -> list -> delete
void create_test() {

  credentials *c = init_ws(user_id, key, endpoint);
  ws_result result;
  char *testdir="/FUSETEST";
  system_meta sm;
  user_meta *um = NULL;	
  //*** Create

  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("result %d\n", result.return_code);
  result_deinit(&result);

  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("result %d\n ", result.return_code);
  result_deinit(&result);
  
  list_ns(c, testdir, NULL, 0, &result);

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
  

  create_ns(c, testdir, NULL,NULL,  NULL, &result);
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
    list_ns(c, testdir, NULL, 0,&result);    
    char *char_response_bucket= NULL;
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
  //*** Create

  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  result_deinit(&result);

  
  list_ns(c, testdir, NULL, 0, &result);
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
  
  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  result_deinit(&result);
    

  
  update_ns(c, testdir,NULL, NULL, &pd, NULL,&result);    

  result_deinit(&result);

  //Now grow the object
  memset(pd.data,1, 32);
  pd.body_size=32;
  pd.offset=31;

  
  update_ns(c, testdir,NULL, NULL, &pd, NULL,&result);    
  result_deinit(&result);

  
  rd.offset=31;
  rd.body_size=32;
  list_ns(c, testdir,&rd, 0, &result);
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

int main() { 

  if(user_id) {
    set_meta_data();
    list();
    simple_update();
    api_testing();
    rangestest();
    create_test();
    capstest();
    api_testing();
    testbuildhashstring();
    testhmac();
  } else {
    printf("please edit test.c and add your credentials for user_id and shared_secret\n");
  }
}
