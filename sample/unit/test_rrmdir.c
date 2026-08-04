#include "test_common.h"
int main(void) { init_ramfs(); assert(rmkdir("/data") == SUCCESS); assert(rrmdir("/data") == SUCCESS); assert(rrmdir("/data") == FAILURE); close_ramfs(); }
