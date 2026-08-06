# RAMFShell 开发大纲

这是一个运行在内存中的简化文件系统，以及操作它的简化 Shell。程序结束后，全部文件数据都会消失。

## 1. 目标与范围

- 用树形结构表示目录和普通文件；根目录固定为 `/`。
- 仅支持绝对路径；连续的 `/` 视为一个 `/`。
- 文件或目录名只能包含字母、数字和 `.`，长度为 1～32。
- 目录可以包含子节点；普通文件只能保存字节内容。
- Shell 层负责把 `ls`、`cat`、`mkdir`、`touch`、`echo`、`which` 等命令转成文件系统 API 调用。

暂不考虑：相对路径、`.` / `..`、权限、磁盘持久化、并发和符号链接。

## 2. 目录与职责

```text
main.c          初始化与简单测试入口
include/ramfs.h 文件系统数据结构、常量与 API 声明
fs/ramfs.c      内存文件系统实现
include/shell.h Shell 接口声明
sh/shell.c      Shell 命令实现
sample/         示例与测试脚本
```

调用主线：

```text
Shell 函数 → ramfs API → 路径/节点辅助函数 → 文件树或描述符表
```

## 3. 核心数据结构

### `node`：文件树节点

| 字段 | 含义 |
| --- | --- |
| `type` | `FILE_NODE` 或 `DIR_NODE` |
| `name` | 当前节点的 basename；根节点名为 `/` |
| `dirents` / `nrde` | 目录的子节点数组及数量 |
| `content` / `size` | 普通文件的内容及字节数 |

约束：目录只使用 `dirents` 和 `nrde`；文件只使用 `content` 和 `size`。

### `FD`：文件描述符表项

| 字段 | 含义 |
| --- | --- |
| `used` | 该描述符是否已分配 |
| `f` | 指向已打开的节点 |
| `offset` | 下次读写的起始位置 |
| `flags` | 打开时的读写标志 |

`fdesc[NRFD]` 是全局描述符表。**offset 属于描述符，不属于文件**：同一文件被打开两次时，两个 offset 独立。

## 4. 辅助函数设计（`fs/ramfs.c`）

这些函数只服务于文件系统实现，优先完成它们，再写 API。

| 函数 | 输入 / 输出 | 职责 |
| --- | --- | --- |
| `valid_path` | 路径 → `bool` | 检查绝对路径、每段长度和字符合法性 |
| `valid_name` | basename → `bool` | 检查单个文件名是否合法 |
| `copy_name` | 字符串片段 → 名称 | 复制路径中的一段名称并补 `\0` |
| `find_child` | 目录、名称 → 节点 | 在一个目录的直接子节点中查找 |
| `find_parent` | 路径 → 父节点 + basename | 找到目标路径的父目录，并取出最后一段名称 |
| `find` | 路径 → 节点 / `NULL` | 从根目录逐段向下查找完整路径 |
| `new_node` | 类型、名称 → 新节点 | 分配并初始化文件或目录节点 |
| `add_child` | 父目录、子节点 → `bool` | 扩容目录项数组并加入节点 |
| `free_node` | 节点 → 无 | 递归释放节点、文件内容、名称和目录项 |
| `valid_fd` | fd → `bool` | 检查 fd 范围及是否正在使用 |
| `can_read` / `can_write` | `FD` → `bool` | 根据 flags 判断是否允许读或写 |

路径查找规则：每次跳过多余的 `/`，取出一个 basename，在当前目录调用 `find_child`；任一段不存在、或中途遇到普通文件，就查找失败。

## 5. 文件系统 API 设计

### 生命周期

| 函数 | 责任 |
| --- | --- |
| `init_ramfs()` | 创建根目录，清空文件描述符表 |
| `close_ramfs()` | 递归释放整棵文件树，重置全局状态 |

### 目录与节点操作

| 函数 | 成功行为 | 主要失败情况 |
| --- | --- | --- |
| `rmkdir(path)` | 在父目录下创建空目录 | 路径非法、父目录不存在/不是目录、目标已存在 |
| `rrmdir(path)` | 删除空目录 | 目标不存在、不是目录、目录非空、试图删除根目录 |
| `runlink(path)` | 删除普通文件 | 目标不存在、目标是目录、试图删除根目录 |

删除节点前，需要在父目录的 `dirents` 中找到它，将后续元素前移，再释放该节点。

### 文件描述符操作

| 函数 | 成功行为 | 关键点 |
| --- | --- | --- |
| `ropen(path, flags)` | 返回一个未使用 fd | 文件不存在且有 `O_CREAT` 时创建；`O_APPEND` 初始 offset 在文件末尾；可写且 `O_TRUNC` 时清空文件 |
| `rclose(fd)` | 释放该 fd | 不释放文件节点本身 |
| `rread(fd, buf, count)` | 从 offset 读出实际可读字节数 | fd 必须有效、可读且对应普通文件；读到末尾返回 0 |
| `rwrite(fd, buf, count)` | 从 offset 写入并返回写入字节数 | fd 必须有效、可写且对应普通文件；必要时扩容，并用 `\0` 填补空洞 |
| `rseek(fd, offset, whence)` | 设置并返回新的 offset | 支持 `SEEK_SET`、`SEEK_CUR`、`SEEK_END`；新 offset 不得为负 |

### flags 速查

| 标志 | 含义 |
| --- | --- |
| `O_RDONLY` | 只读（值为 0） |
| `O_WRONLY` | 只写 |
| `O_RDWR` | 可读写 |
| `O_CREAT` | 文件不存在时创建 |
| `O_APPEND` | 打开后 offset 从文件末尾开始 |
| `O_TRUNC` | 以可写方式打开时清空文件 |

## 6. Shell API 设计（`sh/shell.c`）

| 函数 | 建议调用 | 预期行为 |
| --- | --- | --- |
| `sls(path)` | `find` | 列出目录下的直接子项；文件可仅输出自身信息 |
| `scat(path)` | `ropen` + `rread` + `rclose` | 输出文件全部内容 |
| `smkdir(path)` | `rmkdir` | 创建目录并返回结果 |
| `stouch(path)` | `ropen(..., O_CREAT)` + `rclose` | 文件不存在时创建；已存在则不清空 |
| `secho(content)` | 输出函数 | 输出给定文本（后续若扩展重定向，再写入文件） |
| `swhich(cmd)` | 命令表 | 判断命令是否为 Shell 支持的内建命令 |
| `init_shell()` / `close_shell()` | 初始化 / 清理 | 管理 Shell 自己的状态；若无状态可先保留为空 |

## 7. 推荐实现顺序

1. 完成 `new_node`、`add_child`、`free_node`，并让 `init_ramfs` / `close_ramfs` 可重复调用。
2. 完成 `valid_name`、`valid_path`、`find_child`、`find_parent`、`find`。
3. 完成 `rmkdir`、`rrmdir`、`runlink`，验证文件树的增删查。
4. 完成 `valid_fd`、`can_read`、`can_write` 与 `ropen` / `rclose`。
5. 完成 `rread`、`rwrite`、`rseek`，重点测试 offset 和扩容。
6. 最后实现 Shell 函数；Shell 不直接修改节点，统一调用 ramfs API。

## 8. 每次改动后的检查清单

- 根目录是否始终存在，且没有被删除？
- 非法路径是否被拒绝？
- 创建节点前是否确认父节点是目录且目标不存在？
- 删除节点前是否确认类型、空目录条件和父目录位置？
- 每个 `malloc` / `realloc` 失败时是否保持原状态可用？
- `rread` / `rwrite` 后 offset 是否正确推进？
- 写到文件末尾之后时，中间空洞是否补零？
- 关闭文件描述符时，是否只释放 fd 而不误删文件？
- 关闭文件系统时，是否释放所有节点和文件内容？

## 9. 最小测试场景

```text
init
mkdir /docs
touch /docs/a.txt
write /docs/a.txt: "hello"
cat /docs/a.txt              → hello
ls /docs                     → a.txt
seek 到 8 后写 "!"          → 文件中间补 \0
unlink /docs/a.txt
rmdir /docs
close
```

写代码时只要沿着“路径 → 节点 → fd → 读写”这条线查，就不需要把所有函数背下来。
