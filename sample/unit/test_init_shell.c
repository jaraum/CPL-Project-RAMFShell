#include "test_common.h"
int main(void) { init_ramfs(); test_prepare_shell(); assert(swhich("demo") == SUCCESS); close_shell(); close_ramfs(); }
