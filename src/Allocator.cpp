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
    pool_offset = 0; // CRITICAL FIX: Ensure the bookmark starts at exactly 0

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

// ---------------------------------------------------------
// HELPER: Find the correct bucket index for a given size
// ---------------------------------------------------------
int Allocator::get_bucket_index(size_t size) {
    // If the request is massive, return -1 to trigger the Large Object Bypass
    if (size > 1024) {
        return -1; 
    }
    
    int index = 0;
    size_t bucket_size = 8; // Our smallest bucket size
    
    // Keep doubling the bucket size until it is big enough to hold the request
    while (bucket_size < size) {
        bucket_size = bucket_size * 2;
        index++;
    }
    
    return index;
}

// ---------------------------------------------------------
// ALLOCATE: The core engine for handing out memory
// ---------------------------------------------------------
void* Allocator::allocate(size_t size) {
    if (size == 0) {
        throw std::invalid_argument("Size cannot be zero");
    }

    int index = get_bucket_index(size);

    // The Large Object Bypass
    if (index == -1) {
        size_t total_size = size + 8;
        void* large_block = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        if (large_block == MAP_FAILED) {
            throw std::bad_alloc(); // Throw exception if OS mmap fails
        }
        
        //alternate casting more common in C
        //*(size_t*)large_block = total_size;
        //return (char*)large_block + 8;

        // Write size metadata into the first 8 bytes
        *static_cast<size_t*>(large_block) = total_size;

        // Shift forward by 8 bytes to hide the header
        return static_cast<char*>(large_block) + 8;
    }

    // The Slab Carving Engine
    if (free_lists[index] == nullptr) {
        size_t carve_size = 4096;
        
        // Out of Memory (OOM) check for our internal pool
        if (pool_offset + carve_size > pool_size) {
            throw std::bad_alloc(); // NEW: Throw exception if our 1MB pool is full
        }

        char* slab_start = (char*)memory_pool + pool_offset;
        pool_offset = pool_offset + carve_size; 
        
        //this can be optimised with bitshifting or a simple equation
        size_t block_size = 8;
        for (int i = 0; i < index; i++) {
            block_size = block_size * 2; 
        }

        //alternative bitshift method
        // Left-shift 8 by the index amount (e.g., if index is 2, 8 << 2 = 32)
        // size_t block_size = 8 << index;

        int num_blocks = carve_size / block_size;
        for (int i = 0; i < num_blocks - 1; i++) {
            FreeBlock* current = (FreeBlock*)(slab_start + (i * block_size));
            FreeBlock* next_block = (FreeBlock*)(slab_start + ((i + 1) * block_size));

            current->next = next_block; 
            // (*current).next = next_block; same as line above but broken down, dereference and accessing the next pointer is the arrow
        }
        
        FreeBlock* last = (FreeBlock*)(slab_start + ((num_blocks - 1) * block_size));
        last->next = nullptr;

        free_lists[index] = (FreeBlock*)slab_start;
    }

    FreeBlock* block_to_return = free_lists[index];
    free_lists[index] = block_to_return->next; 
    return (void*)block_to_return;
}