// minimal test file to get started on obb
// the server receives a request and responds (i.e., acknowledges) 
//   with the same request msg id and increments the flag 
// more to be done...getting started... 

#include <stdio.h>
#include "config.h"

typedef struct obb_t{
    // msg id
    int msg_id; 
    // flag
    int flag;
} obb_t;

obb_t obb_req;
obb_t obb_resp;

int main(int argc, char *argv[])
{
    // start with a direct call to slip and get the message back via slip
    // void tpip_slip(char *dev) 
    // how to proceed?

    printf("obb test completed...\n");
}