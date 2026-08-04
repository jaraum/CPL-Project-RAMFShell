#include "test_common.h"
int main(void) { int fd; init_ramfs(); fd = ropen("/data", O_CREAT); assert(fd >= 0 && rclose(fd) == SUCCESS); assert(rclose(fd) == FAILURE); close_ramfs(); }
