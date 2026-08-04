#include "test_common.h"
int main(void) { init_ramfs(); test_prepare_shell(); assert(secho("PATH=$PATH") == SUCCESS); close_shell(); close_ramfs(); }
