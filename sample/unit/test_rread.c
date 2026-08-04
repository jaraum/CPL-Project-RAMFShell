#include "test_common.h"
int main(void) { char text[4] = {0}; int fd; init_ramfs(); test_write_text("/data", "abc"); fd = ropen("/data", O_RDONLY); assert(rread(fd, text, 3) == 3 && strcmp(text, "abc") == 0); assert(rclose(fd) == SUCCESS); close_ramfs(); }
