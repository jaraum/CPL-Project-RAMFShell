.PHONY: all run binary clean gdb

INC_PATH := include/

all: compile

compile: 
	@gcc -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell

run: compile
	@./ramfs-shell

gdb: compile
	gdb ramfs-shell

test_address:
	@clang -fsanitize=address -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_undefined:
	@clang -fsanitize=undefined -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_leak:
	@clang -fsanitize=leak -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_memory:
	@clang -fsanitize=memory -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_thread:
	@clang -fsanitize=thread -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

clean:
	@rm test

