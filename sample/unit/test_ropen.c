#include "test_common.h"
int main(void) { int fd; init_ramfs(); assert(ropen("/missing", O_RDONLY) == FAILURE); fd = ropen("/data", O_CREAT | O_RDWR); assert(fd >= 0 && find("/data") != NULL); assert(rclose(fd) == SUCCESS); close_ramfs(); }
