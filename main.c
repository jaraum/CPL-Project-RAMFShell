#include "ramfs.h"
#include "shell.h"
#include <assert.h>
#include <string.h>

extern node *root;
const char *content = "export PATH=$PATH:/usr/bin/\n";
const char *ct = "export PATH=/home:$PATH";

int main(void) {
  init_ramfs();
  assert(root != NULL);
  assert(root->type == DIR_NODE);
  assert(strcmp(root->name, "/") == 0);
}
