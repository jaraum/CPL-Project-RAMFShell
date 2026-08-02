#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD      4096  // Maximum number of open file descriptors
#define MAX_NODES 65536 // Maximum number of nodes in the filesystem
FD fdesc[NRFD];         // File descriptor table

// Auxiliary functions

static bool valid_path(const char *pathname) {

}

static bool valid_name(const char *name) {

}

static bool valid_fd(int fd) {

}

static bool can_read(const FD *fd) {

}

static bool can_write(const FD *fd) {

}

static node *new_node(int type, char *name) {

}

static node *find_child(const char *pathname, char *basename) {

}

static node *find_parent(const char *name) {

}

static void free_node(node *current) {

}


// API functions

node *find(const char *pathname) {
  return NULL;
}

int ropen(const char *pathname, int flags) {

}

int rclose(int fd) {

}

ssize_t rwrite(int fd, const void *buf, size_t count) {

}

ssize_t rread(int fd, void *buf, size_t count) {

}

off_t rseek(int fd, off_t offset, int whence) {

}

int rmkdir(const char *pathname) {

}

int rrmdir(const char *pathname) {

}

int runlink(const char *pathname) { // unlink

}

void init_ramfs() {
  root = calloc(1, sizeof(node));
  root->type = DIR_NODE;
  root->name = strduo("/");

  for (int i = 0; i < NRFD; i++) {
    fdesc[i].used = false;
  }
}

void close_ramfs() {

}