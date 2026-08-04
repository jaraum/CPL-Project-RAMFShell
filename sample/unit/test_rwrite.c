#include "test_common.h"
int main(void) { int fd; init_ramfs(); fd = ropen("/data", O_CREAT | O_WRONLY); assert(rwrite(fd, "abc", 3) == 3); assert(find("/data")->size == 3); assert(rclose(fd) == SUCCESS); close_ramfs(); }
