#include "test_common.h"
int main(void) { int fd; init_ramfs(); test_write_text("/data", "abc"); fd = ropen("/data", O_RDONLY); assert(rseek(fd, -1, SEEK_END) == 2); assert(rseek(fd, 1, SEEK_CUR) == 3); assert(rseek(fd, 0, 99) == FAILURE); assert(rclose(fd) == SUCCESS); close_ramfs(); }
