#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "stdio.h"
#include "transport.h"
#include "atmos_rest.h"
#include "crypto.h"

static const char *user_id = "9e21feebb9f5455f99cd1ad8c76ebe1e/EMC0056CC2B8DDD13D60";
static const char *key = "1dz6W6m2GJLcF54xHQfkfyomPrA=";
static const char *endpoint = "accesspoint.emccis.com";

#define MEG 1<<20


int main(int argc, char **argv) {

  credentials *c = init_ws(user_id, key, endpoint);
  ws_result result;
  char *testdir="/bigdata/mybiggestdata.1tb";
  system_meta sm;
  user_meta *um = NULL;	
  int starting_offset = 0;

  if (argv[1]) starting_offset = atoi(argv[1]);
  //*** Create
  //  create_ns(c, testdir, NULL,NULL,  NULL, &result);
  printf("result %d\n", result.return_code);
  result_deinit(&result);
  //If the file exists find the last bit and append
  if(result.return_code == 400) {
    
  }
  
  char big_data[MEG];
  postdata d;
  
  unsigned long long volsize=1099511627776ull; //terabyte object
  int writesize = MEG; //1meg writes
  int loop_count = volsize/writesize;
  printf("loop count %d\n", loop_count);
  d.data = big_data;
  d.body_size = writesize;
  int current_loops;
  
  for (current_loops= 0; current_loops < loop_count; current_loops++) {

    d.offset = current_loops * writesize + starting_offset;
    printf("loops: %d\t offset %lld\n", current_loops, d.offset);
    update_ns(c, testdir,NULL, NULL, &d, NULL,&result);    
  }
  free(c);
}
