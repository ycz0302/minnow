<div align="right">

<kbd><a href="README.md">English</a></kbd> &nbsp;|&nbsp; <kbd>中文</kbd>

</div>

# CS144 Minnow — 网络实验

本仓库是 **CS144：计算机网络导论**（斯坦福大学，2025 年秋季学期）实验作业的个人解答。

实验手册存放于 [`docs/`](docs/) 目录（`check0.pdf` – `check7.pdf`）。

## 构建

```bash
cmake -S . -B build
cmake --build build
```

## 测试

```bash
cmake --build build --target checkN   # N = 实验编号（见 docs/）
cmake --build build --target test     # 运行所有测试
cmake --build build --target speed    # 性能基准测试
```

## 学术诚信

本仓库**仅供学习参考**，请勿将代码直接复制到你自己的作业中提交。
