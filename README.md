# 无禁手五子棋必胜树生成器

## 项目简介

本项目是一个无禁手五子棋的必胜树生成器。该程序能地毯式遍历并生成五子棋必胜树，其中叶节点通过 VCT 求解。

## 构建与运行

#### 前置条件

gcc 编译器

#### Linux
```bash
./build.sh
```

#### Windows
```bash
./build.bat
```

构建脚本会自动编译源代码并生成可执行文件 `generator` 和 `test`，同时创建 `data` 目录用于存储生成的必胜树数据。

## 可修改的宏定义
- THREAD_COUNT 设置线程数量
- INITIAL_DEPTH 设置初始的迭代深度
- LOAD_FROM_FILE_ID 设置从 `wtree_<id>.bin` 读取到必胜树
  需要文件存在并清除 generator.c 中的注释
  ```c
  // load_from_file(&wt, LOAD_FROM_FILE_ID);
  ```
