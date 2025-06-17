# ♟️

[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/)
[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()
<!--[![License](https://img.shields.io/badge/license-MIT-lightgrey)]()-->

A chess engine written in modern C++.

---

## 🚀 Features

- ✅ **UCI Protocol Support** – Plug it into any GUI (e.g., Arena, CuteChess, Banksia)
- ♜ **Efficient Move Generation** – Magic bitboard optimizations
- 🔍 **Search Engine** – Alpha-beta pruning, Quiescence search
- ♻️ **Evaluation Function** – Material, Piece-square tables
- 🧪 **Modular & Testable** – Tested with *Doctest*
- 🕹️ **Command-Line Interface** – Play directly in the terminal

---

## 🧰 Requirements

- C++23 or later
- CMake
- A modern compiler: `g++`, `clang++`, or MSVC

---

## 🔨 Building

```bash
git clone https://github.com/MichaelKrLim/ChessEngine.git
cd ChessEngine
mkdir build && cd build
cmake ..
make engine
```
