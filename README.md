<div align="right">

<kbd><a href="#english-version">English</a></kbd> &nbsp;|&nbsp; <kbd><a href="#chinese-version">中文</a></kbd>

</div>

---

<a id="english-version"></a>

# CS144 Minnow — Networking Lab

My solution to the lab assignments of **CS144: Introduction to Computer Networking** (Stanford, Fall 2025).

Lab writeups are in the [`docs/`](docs/) folder (`check0.pdf` – `check7.pdf`).

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Test

```bash
cmake --build build --target checkN   # N = checkpoint number (see docs/)
cmake --build build --target test     # run all tests
cmake --build build --target speed    # benchmarks
```

## Academic Honesty

This repository is public **for reference only**. Do not copy-paste code into your own submission.

---

<a id="chinese-version"></a>

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

---

<div align="right">

[⬆ Back to English](#english-version) &nbsp;|&nbsp; [⬆ 回到中文](#chinese-version)

</div>
