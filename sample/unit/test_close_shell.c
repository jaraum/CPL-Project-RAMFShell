#include "test_common.h"
int main(void) { init_ramfs(); test_prepare_shell(); close_shell(); assert(swhich("demo") == PROBLEM); close_ramfs(); }
