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

int main(int argc, char **argv) {

  char *object = argv[1];
  credentials *c = init_ws(user_id, key, endpoint);
  ws_result result;
  
  list_ns(c, object, NULL, 0, &result);


  int header = 0;
  for (; header < result.header_count; header++) {
    printf("%s\n", result.headers[header]);
    
  }

  
}
  
