# Custom Memory Allocator in C++

A low-level custom heap memory allocator implemented from scratch in C++. This project replaces standard runtime dynamic memory allocation (`malloc`/`free`, `new`/`delete`) with an explicit memory management engine. 

Built as a rigorous exploration of operating systems concepts, this allocator bypasses the standard library to interface directly with virtual memory, featuring custom slab carving, O(1) block recycling, and strict hardware alignment.

## Architectural Highlights

*   **Slab Allocation & Intrusive Lists:** Utilizes segregated free lists to mitigate external fragmentation and achieve O(1) LIFO block recycling. Metadata overhead is reduced to zero by embedding intrusive `next` pointers directly within free blocks.
*   **Modern C++ Compliance:** Built using modern C++ memory safety paradigms, including sized deallocation and zero-overhead standard containers (`std::array`) for bucket initialization.
*   **Hardware-Level Optimization:** Replaces iterative multiplication loops with single-cycle bitwise left-shift operations (`<<`) within the ALU to compute dynamic block sizes.
*   **Large Object Bypass:** Memory requests exceeding the 1024-byte slab limit automatically bypass the free lists, making direct, size-tracked system calls for efficient large-scale allocation.
*   **Hardened Security & Exception Handling:** Engineered with robust bounds management. Features strict modulo-8 memory alignment, `std::invalid_argument` trapping for zero-byte requests, and mathematical wrap-around checks to prevent integer overflow vulnerabilities during massive allocations.

> 📖 **Engineering Journal:** For a complete breakdown of design tradeoffs, bottleneck resolutions, and optimizations made during development, read the [Decision Log](DECISION_LOG.md).

## Tech Stack

*   **Language:** Modern C++ (freestanding operations, explicit casting, strict type safety)
*   **OS Primitives:** POSIX `mmap`, `munmap`
*   **Tooling:** G++ Compiler, macOS Terminal

## Comprehensive Testing

This engine was built using strict Test-Driven Development (TDD). The integrated test suite verifies:
*   Perfect hardware memory alignment (modulo 8).
*   Correct `std::bad_alloc` generation during Out-Of-Memory (OOM) conditions and integer overflow attacks.
*   Intrusive list stability under heavy-load stress testing (1,000+ rapid allocation/deallocation cycles).
*   Flawless zero-overhead memory recycling across all 8 bucket sizes.

## Build and Run Instructions

This project requires a C++17 compatible compiler (like `g++` or `clang++`). 

**1. Clone the repository**
```bash
git clone https://github.com/agamlaugh/custom-memory-allocator.git
cd custom-memory-allocator
```

**2. Compile and run the Test Suite**
```bash
g++ -std=c++17 src/Allocator.cpp tests/test_allocator.cpp -o allocator_test
./allocator_test
```

**3. Compile and run the Sandbox (Manual Testing)**
```bash
g++ -std=c++17 src/Allocator.cpp src/main.cpp -o sandbox
./sandbox
```

**4. Compile and run the Benchmarks (requires Google Benchmark, e.g. `brew install google-benchmark`)**
```bash
P=$(brew --prefix google-benchmark)
g++ -std=c++17 -O2 -I$P/include src/Allocator.cpp benchmarks/bench_allocator.cpp -L$P/lib -lbenchmark -lpthread -o bench
./bench
```

## Author
**Agam Cohen**