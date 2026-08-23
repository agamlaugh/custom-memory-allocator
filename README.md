# Custom Memory Allocator in C++

A low-level custom heap memory allocator implemented from scratch in C++. This project replaces standard runtime dynamic memory allocation (`malloc`/`free`, `new`/`delete`) with an explicit memory management engine.

## Overview
- **OS Interfacing:** Directly requests large virtual memory pages using the Linux/POSIX `mmap()` system call.
- **Metadata Management:** Implements custom block headers to track allocation sizes, alignment, and chunk states.
- **Fragmentation Control:** Utilizes explicit free lists and bidirectional coalescing to mitigate external fragmentation.
- **Safety & Performance:** Focuses on cache efficiency, memory alignment constraints, and robust bounds management.

## Tech Stack
- **Language:** C++ (Low-level pointers, freestanding operations, RAII)
- **OS Primitives:** POSIX `mmap`, `munmap`, `sysconf`
- **Tooling:** CMake, Clang++/G++, GDB, Valgrind
