#pragma once
#include <array>
#include <cstddef> // Needed for size_t

// The Intrusive Free List Node
// This will sit INSIDE the empty space of our free blocks.
struct FreeBlock {
    FreeBlock* next;
};

class Allocator {
private:
    // The massive pool of memory we get from the OS
    void* memory_pool;
    
    // The total size of our massive pool (e.g., 1 Megabyte)
    size_t pool_size;

    // The bookmark tracking how much of the pool we have carved up
    size_t pool_offset; 
    
    // Our Slab Buckets! 
    // We have 8 buckets for sizes: 8, 16, 32, 64, 128, 256, 512, 1024
    // Each index holds a pointer to the first free block of that size.
    // FreeBlock* free_lists[8];

    std::array<FreeBlock*, 8> free_lists{}; 

    // A helper function to figure out which bucket index a requested size belongs to
    int get_bucket_index(size_t size);

public:
    // Constructor: Automatically runs to grab OS memory and set up buckets
    Allocator();

    // Destructor: Automatically runs to return the memory to the OS
    ~Allocator();

    // The core functions the user will call
    void* allocate(size_t size);
    void deallocate(void* ptr);
};