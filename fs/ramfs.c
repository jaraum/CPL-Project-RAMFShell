#include "ramfs.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

node *root = NULL;

#define NRFD 4096
#define MAX_NODES 65536
#define MAX_NAME_LEN 32

static FD fdesc[NRFD];
static int node_count;

static bool valid_name(const char *name) {
  size_t length;
  if (name == NULL || *name == '\0')
    return false;
  length = strlen(name);
  if (length > MAX_NAME_LEN)
    return false;
  for (size_t i = 0; i < length; ++i) {
    unsigned char c = (unsigned char)name[i];
    if (!(c == '.' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z')))
      return false;
  }
  return true;
}

static bool valid_path(const char *pathname) {
  const char *part;
  if (pathname == NULL || pathname[0] != '/')
    return false;
  part = pathname;
  while (*part != '\0') {
    const char *end;
    char name[MAX_NAME_LEN + 1];
    size_t length;
    while (*part == '/')
      ++part;
    if (*part == '\0')
      break;
    end = part;
    while (*end != '\0' && *end != '/')
      ++end;
    length = (size_t)(end - part);
    if (length > MAX_NAME_LEN)
      return false;
    memcpy(name, part, length);
    name[length] = '\0';
    if (!valid_name(name))
      return false;
    part = end;
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
  return access == O_WRONLY || access == O_RDWR || access == 03;
}

static node *new_node(int type, const char *name) {
  node *created;
  if (node_count >= MAX_NODES)
    return NULL;
  created = calloc(1, sizeof(*created));
  if (created == NULL)
    return NULL;
  created->name = malloc(strlen(name) + 1);
  if (created->name == NULL) {
    free(created);
    return NULL;
  }
  strcpy(created->name, name);
  created->type = type;
  ++node_count;
  return created;
}

static node *child_named(const node *directory, const char *name) {
  if (directory == NULL || directory->type != DIR_NODE)
    return NULL;
  for (int i = 0; i < directory->nrde; ++i)
    if (strcmp(directory->dirents[i]->name, name) == 0)
      return directory->dirents[i];
  return NULL;
}

static node *find_child(const char *pathname, char *basename) {
  node *current = root;
  const char *part = pathname;
  if (!valid_path(pathname) || current == NULL)
    return NULL;
  while (*part == '/')
    ++part;
  if (*part == '\0') {
    if (basename != NULL)
      basename[0] = '\0';
    return current;
  }
  while (*part != '\0') {
    const char *end = part;
    size_t length;
    while (*end != '\0' && *end != '/')
      ++end;
    length = (size_t)(end - part);
    if (basename != NULL) {
      memcpy(basename, part, length);
      basename[length] = '\0';
    }
    current = child_named(current, basename != NULL ? basename : "");
    if (current == NULL)
      return NULL;
    while (*end == '/')
      ++end;
    part = end;
  }
  return current;
}

/* Return the parent directory and copy the final path component into basename.
 */
static node *find_parent(const char *pathname, char *basename) {
  const char *last, *start;
  char prefix[4097];
  size_t prefix_len;
  if (!valid_path(pathname))
    return NULL;
  last = pathname + strlen(pathname);
  while (last > pathname && last[-1] == '/')
    --last;
  if (last == pathname)
    return NULL;
  start = last;
  while (start > pathname && start[-1] != '/')
    --start;
  if ((size_t)(last - start) > MAX_NAME_LEN)
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

static bool add_child(node *parent, node *child) {
  node **expanded;
  if (parent == NULL || parent->type != DIR_NODE)
    return false;
  expanded =
      realloc(parent->dirents, (size_t)(parent->nrde + 1) * sizeof(*expanded));
  if (expanded == NULL)
    return false;
  parent->dirents = expanded;
  parent->dirents[parent->nrde++] = child;
  return true;
}

static void remove_child(node *parent, node *child) {
  for (int i = 0; i < parent->nrde; ++i) {
    if (parent->dirents[i] == child) {
      memmove(&parent->dirents[i], &parent->dirents[i + 1],
              (size_t)(parent->nrde - i - 1) * sizeof(*parent->dirents));
      --parent->nrde;
      return;
    }
  }
}

static void invalidate_fds(node *current) {
  for (int i = 0; i < NRFD; ++i)
    if (fdesc[i].used && fdesc[i].f == current)
      fdesc[i].used = false;
  if (current->type == DIR_NODE)
    for (int i = 0; i < current->nrde; ++i)
      invalidate_fds(current->dirents[i]);
}

static void free_node(node *current) {
  if (current == NULL)
    return;
  if (current->type == DIR_NODE) {
    for (int i = 0; i < current->nrde; ++i)
      free_node(current->dirents[i]);
    free(current->dirents);
  }
  free(current->content);
  free(current->name);
  free(current);
  --node_count;
}

node *find(const char *pathname) {
  char component[MAX_NAME_LEN + 1];
  return find_child(pathname, component);
}

int ropen(const char *pathname, int flags) {
  char basename[MAX_NAME_LEN + 1];
  node *file;
  int fd;
  if (!valid_path(pathname))
    return FAILURE;
  for (fd = 0; fd < NRFD && fdesc[fd].used; ++fd) {
  }
  if (fd == NRFD)
    return FAILURE;
  file = find(pathname);
  if (file == NULL && (flags & O_CREAT)) {
    node *parent = find_parent(pathname, basename);
    if (parent == NULL || parent->type != DIR_NODE)
      return FAILURE;
    file = new_node(FILE_NODE, basename);
    if (file == NULL || !add_child(parent, file)) {
      free_node(file);
      return FAILURE;
    }
  }
  if (file == NULL)
    return FAILURE;
  if ((flags & O_TRUNC) && can_write(&(FD){.flags = flags}) &&
      file->type == FILE_NODE) {
    free(file->content);
    file->content = NULL;
    file->size = 0;
  }
  fdesc[fd] = (FD){.used = true,
                   .offset = (flags & O_APPEND) ? file->size : 0,
                   .flags = flags,
                   .f = file};
  return fd;
}

int rclose(int fd) {
  if (!valid_fd(fd))
    return FAILURE;
  fdesc[fd].used = false;
  return SUCCESS;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
  FD *descriptor;
  int end;
  void *expanded;
  if (!valid_fd(fd) || buf == NULL)
    return FAILURE;
  descriptor = &fdesc[fd];
  if (!can_write(descriptor) || descriptor->f->type != FILE_NODE ||
      count > INT_MAX)
    return FAILURE;
  if (descriptor->offset < 0 || descriptor->offset > INT_MAX - (int)count)
    return FAILURE;
  end = descriptor->offset + (int)count;
  if (end > descriptor->f->size) {
    expanded = realloc(descriptor->f->content, (size_t)end);
    if (expanded == NULL && end != 0)
      return FAILURE;
    descriptor->f->content = expanded;
    memset((char *)descriptor->f->content + descriptor->f->size, 0,
           (size_t)(end - descriptor->f->size));
    descriptor->f->size = end;
  }
  memcpy((char *)descriptor->f->content + descriptor->offset, buf, count);
  descriptor->offset = end;
  return (ssize_t)count;
}

ssize_t rread(int fd, void *buf, size_t count) {
  FD *descriptor;
  size_t available, amount;
  if (!valid_fd(fd) || buf == NULL)
    return FAILURE;
  descriptor = &fdesc[fd];
  if (!can_read(descriptor) || descriptor->f->type != FILE_NODE ||
      descriptor->offset < 0)
    return FAILURE;
  if (descriptor->offset >= descriptor->f->size)
    return 0;
  available = (size_t)(descriptor->f->size - descriptor->offset);
  amount = count < available ? count : available;
  memcpy(buf, (char *)descriptor->f->content + descriptor->offset, amount);
  descriptor->offset += (int)amount;
  return (ssize_t)amount;
}

off_t rseek(int fd, off_t offset, int whence) {
  off_t base, target;
  if (!valid_fd(fd) || fdesc[fd].f->type != FILE_NODE)
    return FAILURE;
  if (whence == SEEK_SET)
    base = 0;
  else if (whence == SEEK_CUR)
    base = fdesc[fd].offset;
  else if (whence == SEEK_END)
    base = fdesc[fd].f->size;
  else
    return FAILURE;
  target = base + offset;
  if (target < 0 || target > INT_MAX)
    return FAILURE;
  fdesc[fd].offset = (int)target;
  return target;
}

int rmkdir(const char *pathname) {
  char basename[MAX_NAME_LEN + 1];
  node *parent, *created;
  if (!valid_path(pathname) || find(pathname) != NULL)
    return FAILURE;
  parent = find_parent(pathname, basename);
  if (parent == NULL || parent->type != DIR_NODE)
    return FAILURE;
  created = new_node(DIR_NODE, basename);
  if (created == NULL || !add_child(parent, created)) {
    free_node(created);
    return FAILURE;
  }
  return SUCCESS;
}

int rrmdir(const char *pathname) {
  node *target = find(pathname);
  char basename[MAX_NAME_LEN + 1];
  node *parent;
  if (target == NULL || target == root || target->type != DIR_NODE ||
      target->nrde != 0)
    return FAILURE;
  parent = find_parent(pathname, basename);
  if (parent == NULL)
    return FAILURE;
  remove_child(parent, target);
  invalidate_fds(target);
  free_node(target);
  return SUCCESS;
}

int runlink(const char *pathname) {
  node *target = find(pathname);
  char basename[MAX_NAME_LEN + 1];
  node *parent;
  if (target == NULL || target->type != FILE_NODE)
    return FAILURE;
  parent = find_parent(pathname, basename);
  if (parent == NULL)
    return FAILURE;
  remove_child(parent, target);
  invalidate_fds(target);
  free_node(target);
  return SUCCESS;
}

void init_ramfs() {
  close_ramfs();
  memset(fdesc, 0, sizeof(fdesc));
  node_count = 0;
  root = new_node(DIR_NODE, "/");
}

void close_ramfs() {
  if (root != NULL)
    free_node(root);
  root = NULL;
  node_count = 0;
  memset(fdesc, 0, sizeof(fdesc));
}
