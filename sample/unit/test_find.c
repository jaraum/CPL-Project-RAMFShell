#include "test_common.h"
int main(void) { init_ramfs(); assert(rmkdir("/data") == SUCCESS); assert(find("/") == root); assert(find("/data") != NULL); assert(find("/missing") == NULL); assert(find("data") == NULL); close_ramfs(); }
