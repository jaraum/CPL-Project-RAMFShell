#include "test_common.h"
int main(void) { init_ramfs(); assert(root != NULL && root->type == DIR_NODE); init_ramfs(); assert(find("/") == root); close_ramfs(); }
