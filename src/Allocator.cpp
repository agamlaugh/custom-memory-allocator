#include "../include/Allocator.hpp"
#include <sys/mman.h>  // The OS library for mmap and munmap
#include <stdexcept>   // For throwing errors

// Define our total pool size: 1 Megabyte (1024 * 1024 bytes)
const size_t POOL_SIZE = 1048576; 

// ---------------------------------------------------------
// CONSTRUCTOR: Ask the OS for memory
// ---------------------------------------------------------
Allocator::Allocator() {
    pool_size = POOL_SIZE;

    // The core system call bypassing the standard heap
    memory_pool = mmap(
        nullptr,                      // Let the OS pick the memory address
        pool_size,                    // Ask for exactly 1 Megabyte
        PROT_READ | PROT_WRITE,       // Ask for permission to read and write
        MAP_PRIVATE | MAP_ANONYMOUS,  // Anonymous means "RAM only, no files attached"
        -1,                           // No file descriptor needed
        0                             // No offset needed
    );

    // If the OS is completely out of RAM, mmap returns MAP_FAILED
    if (memory_pool == MAP_FAILED) {
        throw std::bad_alloc(); // Standard C++ "Out of Memory" crash
    }

    // Set all 8 of our slab buckets to empty (nullptr) to start
    for (int i = 0; i < 8; i++) {
        free_lists[i] = nullptr;
    }
}

// ---------------------------------------------------------
// DESTRUCTOR: Give the memory back to the OS
// ---------------------------------------------------------
Allocator::~Allocator() {
    // Check if we actually have a valid pool to return
    if (memory_pool != MAP_FAILED && memory_pool != nullptr) {
        munmap(memory_pool, pool_size); // The system call to release the memory
    }
}