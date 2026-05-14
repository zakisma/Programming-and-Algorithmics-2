# Programming and Algorithmics 2 (BI-PA2)

## Overview
This repository contains my C++ solutions for the **Programming and Algorithmics 2 (BI-PA2)** course. The projects demonstrate a progression from basic memory management and standard template library (STL) usage to advanced object-oriented design, binary file parsing, and graph algorithms.

All code is written in modern C++ and built using CMake. 

## Coursework Breakdown

### `1hw` - Tax Register (`CTaxRegister`)
An efficient database for tracking citizens and their financial records.
* **Concepts:** STL containers, algorithmic complexity, lambda functions.
* **Implementation:** Uses dual sorted `std::vector` arrays and `std::lower_bound` to ensure $O(\log n)$ complexity for lookups, insertions, and income/expense updates based on either Name/Address or Account Number.

### `2hw` - Polynomial Calculator (`CPolynomial`)
A robust mathematical class for manipulating polynomials of arbitrary degree.
* **Concepts:** Operator overloading, dynamic memory handling, string stream formatting.
* **Implementation:** Overloads `[]`, `()`, `*`, `*=`, `==`, `!=`, and `<<` operators. Dynamically resizes the underlying coefficient vector and formats output cleanly (handling zeros, signs, and coefficients of 1 appropriately).

### `3hw` - Memory Management Concepts
* **Concepts:** Pointers, dynamic allocation (`new`/`delete`), and class constructors/destructors.
* **Implementation:** Explores memory lifecycle rules and the necessity of proper destructors when managing object pointers to prevent memory leaks.

### `4hw` - Mail Log Parser (`CMailLog`)
A log processing system that filters and searches large server logs.
* **Concepts:** Advanced STL containers, string parsing, custom comparators.
* **Implementation:** Utilizes `std::istringstream` to parse timestamped log lines. Stores states using `std::map` and `std::multimap`, allowing efficient range queries (`listMail`) and unique user aggregation (`activeUsers`) via `std::set`.

### `5hw` - Custom Binary Linker (`CLinker`)
A simulated linker that combines object files and strips dead code.
* **Concepts:** Binary File I/O, Graph Traversal (DFS/Reachability), Symbol Tables.
* **Implementation:** Reads custom binary `.o` files, parses export/import tables, and builds a global symbol map. Uses graph traversal to identify reachable functions from an entry point (e.g., `main`) and generates a flat binary executable with properly resolved memory offsets.

### `6hw` - ASCII Spreadsheet Renderer (`CTable`)
An Object-Oriented spreadsheet engine capable of rendering text and images in a grid.
* **Concepts:** Polymorphism, abstract classes, smart pointers, deep copying.
* **Implementation:** Uses a base `CCell` class with derived `CText`, `CImage`, and `CEmpty` classes. Employs `std::shared_ptr` and polymorphic `clone()` methods to handle deep copies of the grid while correctly aligning content and drawing ASCII borders.

### `7hw` - Network Router (`CNet`)
A generic network graph designed to find the shortest path between nodes.
* **Concepts:** Template Meta-programming, Graph Theory, Breadth-First Search (BFS).
* **Implementation:** A fully templated class (`template<typename T_>`) that builds an adjacency list. Features an `optimize()` phase to construct the graph and a `totalCost()` function that calculates unweighted shortest paths using a BFS queue.

---

## Build Instructions

This project uses `CMake` for compilation. Each homework directory is entirely self-contained with its own `CMakeLists.txt`.

### Terminal Build Example
To build and run the 5th homework (Binary Linker):

```bash
cd 5hw
mkdir build && cd build
cmake ..
make
./5hw
