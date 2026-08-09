#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD      4096      // Maximum number of open file descriptors
#define MAX_NODES 65535     // Maximum number of nodes in the filesystem
#define MAX_NAME_LENGTH 32  // Maximum name length
FD fdesc[NRFD];             // File descriptor table
static uint16_t node_count = 0;

// Auxiliary functions

static void copy_name(char target[33], const char *start, int length) {
  memcpy(target, start, length);
  target[length] = '\0';
}

static bool is_valid_name(const char *name) {
  if (name == NULL) {
    return false;
  }

  size_t length = strlen(name);
  if (length <= 0 || length > MAX_NAME_LENGTH) {
    return false;
  }

  for (int i = 0; i < length; i++) {
    if (!isalnum((unsigned char)name[i] && name[i] != '.')) {
      return false;
    }
  }
  return true;
}

static bool is_valid_path(const char *pathname) { // check with basename
  if (pathname == NULL || pathname[0] != '/') {
    return false;
  }
  const char *p = pathname;

  while (*p != '\0') {
    while (*p == '/') {
      ++p;
    }
    if (*p == '\0')
      break;
    const char *q;
    char name[MAX_NAME_LENGTH + 1];
    size_t length;
    q = p;
    while (q != '\0' && q != '/')
      ++q;
    size_t length = (size_t)(q - p);
    if (length > MAX_NAME_LENGTH)
      return false;
    memcpy(name, p, length);
    name[length] = '\0';
    if (!is_valid_name(pathname))
      return false;
    p = q;
  }
  return true;
}

static bool valid_fd(int fd) { return fd >= 0 && fd < NRFD && fdesc[fd].used; }

static bool can_read(const FD *fd) {

}

static bool can_write(const FD *fd) {

}

static node *new_node(int type, char *name) {
  node *result = calloc(1, sizeof(node));
  if (result == NULL) {
    return NULL;
  }

  if (!is_valid_name(name) || node_count >= MAX_NODES) {
    free(result);
    return NULL;
  }

  result->type = type;
  result->name = malloc(strlen(name) + 1);
  if (result->name == NULL) {
    free(result);
    return NULL;
  }
  strcpy(result->name, name);
  ++node_count;
  return result;
}

static bool add_child(node *parent, node *child) {
  if (parent == NULL || child == NULL || parent->type != DIR_NODE) {
    return false;
  }
  
  node **new_dirents = realloc(parent->dirents, (parent->nrde + 1) * sizeof(node));

  if (new_dirents == NULL) {
    return false;
  }
  parent->dirents = new_dirents;
  parent->dirents[parent->nrde] = child;
  parent->nrde++;

  return true;
}

static node *find_child(const node *dir, char *basename) {
  if (dir == NULL || dir->type == FILE_NODE) {
    return NULL;
  }

  for (int i = 0; i < dir->nrde; i++) {
    if (strcmp(dir->dirents[i]->name, basename) == 0) {
      return dir->dirents[i];
    }
  }
  return NULL;
}

static node *find_parent(const char *name, char basename[MAX_NAME_LENGTH + 1]) {

}

static void free_node(node *current) {
  if (current == NULL)
    return;
  if (current->type == DIR_NODE) {
    for (int i = 0; i < current->nrde; ++i)
      free_node(current->dirents[i]);
    free(current->dirents);
  } else {
    free(current->content);
  }
  free(current->name);
  free(current);
  --node_count;
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
  root = new_node(DIR_NODE, "/");

  for (int i = 0; i < NRFD; i++) {
    fdesc[i].used = false;
  }
}

void close_ramfs() {

}