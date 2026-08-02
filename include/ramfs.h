#pragma once
#include <stdbool.h>
#include <stdint.h>

// FLAG
#define O_APPEND 02000 // Append: 
#define O_CREAT  0100  // Create: 
#define O_TRUNC  01000 // Truncate: 
#define O_RDONLY 00    // Read only
#define O_WRONLY 01    // Write only
#define O_RDWR   02    // Read & Write

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define SUCCESS 0
#define PROBLEM 1
#define FAILURE -1
#define ENOENT -2
#define ENOTDIR -3
#define EINVAL -4
#define EISFILE -5

typedef struct node {
  enum { FILE_NODE, DIR_NODE } type;
  struct node **dirents; // if DIR_NODE
  void *content;
  int nrde;
  int size;
  char *name;
} node;

typedef struct FD {
  bool used;
  int offset;
  int flags;
  node *f;
} FD;

typedef intptr_t ssize_t;
typedef uintptr_t size_t;
typedef long off_t;

int ropen(const char *pathname, int flags);
int rclose(int fd);
ssize_t rwrite(int fd, const void *buf, size_t count);
ssize_t rread(int fd, void *buf, size_t count);
off_t rseek(int fd, off_t offset, int whence);
int rmkdir(const char *pathname);
int rrmdir(const char *pathname);
int runlink(const char *pathname);
void init_ramfs();
void close_ramfs();
node *find(const char *pathname);