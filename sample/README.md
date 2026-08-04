# README 样例测试

`sample1.c` 至 `sample5.c` 对应项目 README 中的五个样例。`unit/` 目录按“一个公开 API 一个 `.c` 文件”的方式提供独立单元测试；共享的初始化辅助代码仅位于 `unit/test_common.h`。它们互不影响，也不会覆盖仓库根目录的 `main.c`。

在仓库根目录运行：

```sh
./sample/run.sh        # 依次运行全部样例
./sample/run.sh 1      # 只运行样例 1
./sample/run.sh unit   # 只运行每个 API 的独立单元测试
```

每个测试都包含 `assert`；某项断言失败时，程序会立即退出，说明该接口尚未实现或实现不符合要求。输出型 Shell API 主要验证返回值和文件系统状态；`ls` 的目录项输出顺序不作假设。
