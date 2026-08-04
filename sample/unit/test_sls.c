#include "test_common.h"
int main(void) { init_ramfs(); assert(rmkdir("/data") == SUCCESS); assert(sls("/") == SUCCESS); assert(sls("/missing") == PROBLEM); close_ramfs(); }
