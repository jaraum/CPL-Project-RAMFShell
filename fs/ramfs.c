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

static char *get_basename(const char *pathname) {
  if (pathname == NULL) return NULL;
  const char *start = pathname;
  char *basename = calloc(MAX_NAME_LENGTH + 1, 1);
  if (!basename) return NULL;

  while (*start != '\0') {
    while (*start == '/') {
      ++start;
    }
    if (*start == '\0')
      break;
    const char *end;
    size_t length;
    end = start;
    while (*end != '\0' && *end != '/')
      ++end;
    length = (size_t)(end - start);
    if (length > MAX_NAME_LENGTH) {
      free(basename);
      return NULL;
    }
    memcpy(basename, start, length);
    basename[length] = '\0';
    start = end;
  }
  return basename;
}

static bool is_valid_name(const char *name) {
  if (name == NULL) {
    return false;
  }

  size_t length = strlen(name);
  if (length == 0 || length > MAX_NAME_LENGTH) {
    return false;
  }

  for (size_t i = 0; i < length; ++i) {
    unsigned char c = (unsigned char)name[i];
    if (!(c == '.' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z')))
      return false;
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
    q = p;
    while (*q != '\0' && *q != '/')
      ++q;
    size_t length = (size_t)(q - p);
    if (length > MAX_NAME_LENGTH)
      return false;
    memcpy(name, p, length);
    name[length] = '\0';
    if (!is_valid_name(name))
      return false;
    p = q;
  }
  return true;
}

static bool valid_fd(int fd) { return fd >= 0 && fd < NRFD && fdesc[fd].used; }

static bool can_read(const FD *fd) {
  int access = fd->flags & 03;
  return access == O_RDONLY || access == O_RDWR;
}

static bool can_write(const FD *fd) {
  int access = fd->flags & 03;
  return access == O_WRONLY || access == O_RDWR;
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
  
  node **new_dirents = realloc(parent->dirents, (size_t)(parent->nrde + 1) * sizeof(*parent->dirents));

  if (new_dirents == NULL) {
    return false;
  }
  parent->dirents = new_dirents;
  parent->dirents[parent->nrde] = child;
  parent->nrde++;
  return true;
}

// return ptr to basename node
static node *find_child(const node *dir, char *basename) {
  if (dir == NULL || basename == NULL || dir->type != DIR_NODE) {
    return NULL;
  }
  if (!is_valid_name(basename))
    return NULL;

  for (int i = 0; i < dir->nrde; i++) {
    if (strcmp(dir->dirents[i]->name, basename) == 0) {
      return dir->dirents[i];
    }
  }
  return NULL;
}

// Return the parent directory and copy the final path component into basename.
static node *find_parent(const char *pathname, char *basename) {
  const char *start, *last;
  char prefix[NRFD + 1];
  size_t prefix_len;
  if (!is_valid_path(pathname))
    return NULL;
  last = pathname + strlen(pathname);
  while (last > pathname && last[-1] != '/')
    --last;
  if (last == pathname)
    return NULL;

  start = last;
  while (start > pathname && start[-1] != '/')
    --start;
  if ((size_t)(last - start) > MAX_NAME_LENGTH)
    return NULL;
  memcpy(basename, start, (size_t)(last - start));
  basename[last - start] = '\0';
  if (!valid_name(basename))
    return NULL;

  prefix_len = (size_t)(start - pathname);
  if (prefix_len == 0)
    return root;
  if (prefix_len >= sizeof(prefix))
    return NULL;
  memcpy(prefix, pathname, prefix_len);
  prefix[prefix_len] = '\0';
  return find(prefix);
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
  char component[MAX_NAME_LENGTH + 1];
  return find_child(pathname, component);
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
  close_ramfs();
  
  root = new_node(DIR_NODE, "/");
}

void close_ramfs() {
  if (root != NULL)
    free_node(root);
  root = NULL;
  node_count = 0;
  memset(fdesc, 0, sizeof(fdesc));
}