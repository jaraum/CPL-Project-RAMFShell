#include "ramfs.h"
#include "shell.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
  init_ramfs();
  init_shell();

  assert(sls("/home") == 1);
  assert(scat("/home/ubuntu/.bashrc") == 1);
  assert(scat("/") == 1);
  assert(smkdir("/home") == 0);
  assert(smkdir("/test/1") == 1);
  assert(stouch("/home/1") == 0);
  assert(smkdir("/home/1/1") == 1);
  assert(stouch("/test/1") == 1);
  assert(swhich("notexist") == 1);

  close_shell();
  close_ramfs();
}