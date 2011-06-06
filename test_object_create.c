#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "stdio.h"
#include "transport.h"
#include "atmos_rest.h"
#include "crypto.h"


static const char *user_id = "9e21feebb9f5455f99cd1ad8c76ebe1e/EMC0056CC2B8DDD13D60";
static const char *key = "1dz6W6m2GJLcF54xHQfkfyomPrA=";
static const char *endpoint = "accesspoint.emccis.com";//*/



int main() { 

    credentials *c = init_ws(user_id, key, endpoint);
    ws_result result;
    char *object_id = (char*)malloc(45);;
    system_meta sm;
    user_meta *um = NULL;	
    //*** Create

    create_obj(c,object_id, NULL,NULL,  NULL, &result);
    printf("result %d\n", result.return_code);
    printf("%s\n", object_id);
    result_deinit(&result);


    free(c);
}


