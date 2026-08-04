#include "shell.h"
#include "ramfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ONLINE_JUDGE
#define print(...) do { printf("\033[31m"); printf(__VA_ARGS__); printf("\033[0m"); } while (0)
#else
#define print(...) do {} while (0)
#endif

static char *shell_path;

static char *copy_string(const char *source) {
  char *copy = malloc(strlen(source) + 1);
  if (copy != NULL)
    strcpy(copy, source);
  return copy;
}

/* Shell commands need to distinguish a missing component from traversal through
 * a file. */
static int path_problem(const char *pathname) {
  const char *part;
  char prefix[4097] = "";
  size_t used = 0;
  if (pathname == NULL || pathname[0] != '/')
    return ENOENT;
  part = pathname;
  while (*part == '/')
    ++part;
  while (*part != '\0') {
    const char *end = part;
    node *current;
    while (*end != '\0' && *end != '/')
      ++end;
    if (used + 1 + (size_t)(end - part) >= sizeof(prefix))
      return ENOENT;
    prefix[used++] = '/';
    memcpy(prefix + used, part, (size_t)(end - part));
    used += (size_t)(end - part);
    prefix[used] = '\0';
    current = find(prefix);
    if (current == NULL)
      return ENOENT;
    while (*end == '/')
      ++end;
    if (*end != '\0' && current->type != DIR_NODE)
      return ENOTDIR;
    part = end;
  }
  return SUCCESS;
}

static void set_path_value(const char *value) {
  char *replacement = copy_string(value);
  if (replacement != NULL) {
    free(shell_path);
    shell_path = replacement;
  }
}

static void apply_path_assignment(const char *value) {
  const char *marker = strstr(value, "$PATH");
  if (marker == NULL) {
    set_path_value(value);
    return;
  }
  size_t before = (size_t)(marker - value),
         old = shell_path ? strlen(shell_path) : 0;
  size_t after = strlen(marker + 5);
  char *combined = malloc(before + old + after + 1);
  if (combined == NULL)
    return;
  memcpy(combined, value, before);
  if (shell_path != NULL)
    memcpy(combined + before, shell_path, old);
  memcpy(combined + before + old, marker + 5, after + 1);
  free(shell_path);
  shell_path = combined;
}

int sls(const char *pathname) {
  node *directory;
  int problem;
  print("ls %s\n", pathname);
  problem = path_problem(pathname);
  if (problem == ENOENT) {
    printf("ls: cannot access '%s': No such file or directory\n", pathname);
    return PROBLEM;
  }
  if (problem == ENOTDIR) {
    printf("ls: cannot access '%s': Not a directory\n", pathname);
    return PROBLEM;
  }
  directory = find(pathname);
  if (directory->type == FILE_NODE) {
    printf("%s\n", directory->name);
    return SUCCESS;
  }
  for (int i = 0; i < directory->nrde; ++i)
    printf("%s%s", i ? " " : "", directory->dirents[i]->name);
  printf("\n");
  return SUCCESS;
}

int scat(const char *pathname) {
  node *file;
  int problem;
  print("cat %s\n", pathname);
  problem = path_problem(pathname);
  if (problem == ENOENT) {
    printf("cat: %s: No such file or directory\n", pathname);
    return PROBLEM;
  }
  if (problem == ENOTDIR) {
    printf("cat: %s: Not a directory\n", pathname);
    return PROBLEM;
  }
  file = find(pathname);
  if (file->type == DIR_NODE) {
    printf("cat: %s: Is a directory\n", pathname);
    return PROBLEM;
  }
  if (file->size > 0)
    fwrite(file->content, 1, (size_t)file->size, stdout);
  return SUCCESS;
}

int smkdir(const char *pathname) {
  int problem;
  print("mkdir %s\n", pathname);
  problem = path_problem(pathname);
  if (problem == ENOTDIR) {
    printf("mkdir: cannot create directory '%s': Not a directory\n", pathname);
    return PROBLEM;
  }
  if (find(pathname) != NULL) {
    printf("mkdir: cannot create directory '%s': File exists\n", pathname);
    return PROBLEM;
  }
  if (problem == ENOENT) {
    /* A missing final component is fine only when its parent exists. */
    const char *slash = strrchr(pathname, '/');
    char parent[4097];
    size_t length = (size_t)(slash - pathname);
    if (length == 0)
      strcpy(parent, "/");
    else {
      memcpy(parent, pathname, length);
      parent[length] = '\0';
    }
    if (find(parent) == NULL) {
      printf("mkdir: cannot create directory '%s': No such file or directory\n",
             pathname);
      return PROBLEM;
    }
  }
  if (rmkdir(pathname) != SUCCESS) {
    printf("mkdir: cannot create directory '%s': No such file or directory\n",
           pathname);
    return PROBLEM;
  }
  return SUCCESS;
}

int stouch(const char *pathname) {
  int problem;
  int fd;
  print("touch %s\n", pathname);
  problem = path_problem(pathname);
  if (problem == ENOTDIR) {
    printf("touch: cannot touch '%s': Not a directory\n", pathname);
    return PROBLEM;
  }
  if (problem == ENOENT) {
    const char *slash = strrchr(pathname, '/');
    char parent[4097];
    size_t length = (size_t)(slash - pathname);
    if (length == 0)
      strcpy(parent, "/");
    else {
      memcpy(parent, pathname, length);
      parent[length] = '\0';
    }
    if (find(parent) == NULL || find(parent)->type != DIR_NODE) {
      printf("touch: cannot touch '%s': No such file or directory\n", pathname);
      return PROBLEM;
    }
  }
  fd = ropen(pathname, O_CREAT);
  if (fd == FAILURE) {
    printf("touch: cannot touch '%s': No such file or directory\n", pathname);
    return PROBLEM;
  }
  rclose(fd);
  return SUCCESS;
}

int secho(const char *content) {
  const char *p = content;
  print("echo %s\n", content);
  if (content == NULL)
    return PROBLEM;
  while (*p != '\0') {
    if (*p == '\\' && p[1] != '\0') {
      putchar(p[1]);
      p += 2;
    } else if (strncmp(p, "$PATH", 5) == 0) {
      fputs(shell_path ? shell_path : "", stdout);
      p += 5;
    } else {
      putchar(*p++);
    }
  }
  putchar('\n');
  return SUCCESS;
}

int swhich(const char *cmd) {
  const char *part;
  print("which %s\n", cmd);
  if (cmd == NULL || shell_path == NULL)
    return PROBLEM;
  part = shell_path;
  while (true) {
    const char *end = strchr(part, ':');
    size_t length = end ? (size_t)(end - part) : strlen(part);
    char candidate[4097];
    node *found;
    if (length + strlen(cmd) + 2 < sizeof(candidate)) {
      memcpy(candidate, part, length);
      candidate[length] = '\0';
      if (length == 0 || candidate[length - 1] != '/')
        strcat(candidate, "/");
      strcat(candidate, cmd);
      found = find(candidate);
      if (found != NULL && found->type == FILE_NODE) {
        printf("%s\n", candidate);
        return SUCCESS;
      }
    }
    if (end == NULL)
      break;
    part = end + 1;
  }
  return PROBLEM;
}

void init_shell() {
  node *bashrc;
  char *text, *line;
  free(shell_path);
  shell_path = NULL;
  bashrc = find("/home/ubuntu/.bashrc");
  if (bashrc == NULL || bashrc->type != FILE_NODE)
    return;
  text = malloc((size_t)bashrc->size + 1);
  if (text == NULL)
    return;
  memcpy(text, bashrc->content, (size_t)bashrc->size);
  text[bashrc->size] = '\0';
  line = text;
  while (line != NULL) {
    char *next = strchr(line, '\n');
    if (next != NULL)
      *next++ = '\0';
    if (strncmp(line, "export PATH=", 12) == 0)
      apply_path_assignment(line + 12);
    line = next;
  }
  free(text);
}

void close_shell() {
  free(shell_path);
  shell_path = NULL;
}