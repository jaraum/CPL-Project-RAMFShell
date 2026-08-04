#include "test_common.h"
int main(void) { init_ramfs(); assert(stouch("/data") == SUCCESS && find("/data") != NULL); assert(stouch("/data") == SUCCESS); assert(stouch("/missing/data") == PROBLEM); close_ramfs(); }
