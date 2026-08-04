#include "test_common.h"
int main(void) { init_ramfs(); assert(rmkdir("/data") == SUCCESS); close_ramfs(); assert(root == NULL); }
