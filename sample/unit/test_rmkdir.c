#include "test_common.h"
int main(void) { init_ramfs(); assert(rmkdir("/data") == SUCCESS); assert(rmkdir("/data") == FAILURE); assert(rmkdir("/missing/child") == FAILURE); close_ramfs(); }
