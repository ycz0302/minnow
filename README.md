<div align="right">

<kbd>English</kbd> &nbsp;|&nbsp; <kbd><a href="README.zh-CN.md">中文</a></kbd>

</div>

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
