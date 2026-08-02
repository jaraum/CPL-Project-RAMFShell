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
  enum { FILE_NODE, DIR_NODE } type; // Node type: file or directory
  struct node **dirents;             // Array of child nodes (only used for directories)
  void *content;                     // File content data (only used for files)
  int nrde;                          // Number of entries in this directory
  int size;                          // Size of the file content in bytes
  char *name;                        // File or directory name
} node;

typedef struct FD {
  bool used;  // Whether this file descriptor entry is currently in use
  int offset; // Current read/write position within the file
  int flags;  // File access flags (e.g., O_RDONLY, O_WRONLY, O_RDWR)
  node *f;    // Pointer to the file node associated with this descriptor
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
int runlink(const char *pathname); // unlink
void init_ramfs();
void close_ramfs();
node *find(const char *pathname);