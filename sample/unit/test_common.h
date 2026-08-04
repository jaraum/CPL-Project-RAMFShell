#pragma once

#include "ramfs.h"
#include "shell.h"

#include <assert.h>
#include <string.h>

extern node *root;

static inline void test_write_text(const char *pathname, const char *text) {
  int fd = ropen(pathname, O_CREAT | O_WRONLY);
  assert(fd >= 0);
  assert(rwrite(fd, text, strlen(text)) == (ssize_t)strlen(text));
  assert(rclose(fd) == SUCCESS);
}

static inline void test_prepare_file(const char *pathname) {
  assert(rmkdir("/tmp") == SUCCESS);
  test_write_text(pathname, "abc");
}

static inline void test_prepare_shell(void) {
  assert(rmkdir("/home") == SUCCESS);
  assert(rmkdir("/home/ubuntu") == SUCCESS);
  assert(rmkdir("/bin") == SUCCESS);
  test_write_text("/home/ubuntu/.bashrc", "export PATH=/bin\n");
  test_write_text("/bin/demo", "executable");
  init_shell();
}
