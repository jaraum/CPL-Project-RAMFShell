#include "test_common.h"
int main(void) { init_ramfs(); test_write_text("/data", "abc"); assert(scat("/data") == SUCCESS); assert(scat("/") == PROBLEM); close_ramfs(); }
