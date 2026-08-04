#include "test_common.h"
int main(void) { init_ramfs(); test_write_text("/data", "abc"); assert(runlink("/data") == SUCCESS); assert(find("/data") == NULL); assert(runlink("/data") == FAILURE); close_ramfs(); }
