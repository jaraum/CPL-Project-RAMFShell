#include "test_common.h"
int main(void) { init_ramfs(); assert(smkdir("/data") == SUCCESS); assert(smkdir("/data") == PROBLEM); assert(smkdir("/missing/data") == PROBLEM); close_ramfs(); }
