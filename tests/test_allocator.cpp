#include <cassert> // Tripwire for test failures
#include <iostream> // For outputting test results in terminal
#include <cstdint> // Needed for uintptr_t (checking memory addresses)
#include <stdexcept> // For throwing exceptions in case of critical errors
#include "../include/Allocator.hpp" // Include the header for the Allocator class

// Test 1: Basic allocation ensures we get a valid memory address
void test_basic_allocation() {
    Allocator my_allocator;
    void* ptr = my_allocator.allocate(16);
    
    assert(ptr != nullptr);
    std::cout << "[PASS] test_basic_allocation\n";
}

// Test 2: Write verification ensures we can actually use the memory without crashing
void test_write_memory() {
    Allocator my_allocator;
    // Request space for an integer (4 bytes, but will map to our smallest slab bucket)
    int* ptr = static_cast<int*>(my_allocator.allocate(sizeof(int)));
    
    assert(ptr != nullptr);
    
    // Write data to the allocated memory
    *ptr = 42;
    assert(*ptr == 42);
    
    std::cout << "[PASS] test_write_memory\n";
}

// Test 3: Deallocation and recycling ensures freed blocks are reused
void test_deallocation_recycling() {
    Allocator my_allocator;
    
    void* ptr1 = my_allocator.allocate(16);
    assert(ptr1 != nullptr);
    
    // Deallocate the first block
    my_allocator.deallocate(ptr1);
    
    // Allocate again; a proper slab allocator should immediately recycle and hand back the exact same address
    void* ptr2 = my_allocator.allocate(16);
    assert(ptr2 != nullptr);
    
    assert(ptr1 == ptr2); // Verifies recycling behavior
    std::cout << "[PASS] test_deallocation_recycling\n";
}

// Test 4: Iterate through all powers of two to ensure every bucket works
void test_all_bucket_sizes() {
    Allocator my_allocator;
    void* pointers[8]; 
    
    size_t size = 8;
    for (int i = 0; i < 8; i++) {
        pointers[i] = my_allocator.allocate(size);
        assert(pointers[i] != nullptr);
        
        // Multiply size by 2 for the next loop (8 -> 16 -> 32 -> 64...)
        size = size * 2; 
    }
    
    for (int i = 0; i < 8; i++) {
        my_allocator.deallocate(pointers[i]);
    }
    std::cout << "[PASS] test_all_bucket_sizes\n";
}

// Test 5: Zero-byte allocations should throw an exception
void test_zero_allocation() {
    Allocator my_allocator;
    bool caught_exception = false;
    
    try {
        my_allocator.allocate(0);
    } catch (const std::invalid_argument& e) {
        caught_exception = true; // The exception was successfully caught!
    }
    
    // Assert that the exception was actually thrown
    assert(caught_exception == true);
    std::cout << "[PASS] test_zero_allocation (Exception Thrown)\n";
}

// Test 6: Out of Memory (OOM) requests should not crash the system
void test_out_of_memory() {
    Allocator my_allocator;
    
    // Request 2 Gigabytes of memory (larger than our initial mmap pool)
    size_t massive_request = 2000000000; 
    void* ptr = my_allocator.allocate(massive_request);
    
    assert(ptr == nullptr);
    std::cout << "[PASS] test_out_of_memory\n";
}

// Test 7: Alignment ensures hardware compatibility
void test_alignment() {
    Allocator my_allocator;
    void* ptr = my_allocator.allocate(16);
    
    // Cast the pointer to a raw integer to check its mathematical address
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    
    // Modulo 8 checks if the address is a perfect multiple of 8
    assert(address % 8 == 0);
    std::cout << "[PASS] test_alignment\n";
}

// Test 8: Stress testing the intrusive list under heavy load
void test_stress_and_stability() {
    Allocator my_allocator;
    void* pointers[1000]; // An array to hold 1,000 memory addresses
    
    // Phase 1: Allocate 1,000 blocks of 32 bytes
    for (int i = 0; i < 1000; i++) {
        pointers[i] = my_allocator.allocate(32);
        assert(pointers[i] != nullptr);
    }
    
    // Phase 2: Deallocate all 1,000 blocks back to the free list
    for (int i = 0; i < 1000; i++) {
        my_allocator.deallocate(pointers[i]);
    }
    
    // Phase 3: Allocate one more time to ensure the list isn't broken
    void* final_ptr = my_allocator.allocate(32);
    assert(final_ptr != nullptr);
    
    std::cout << "[PASS] test_stress_and_stability\n";
}

// Test 9: Large allocation bypass ensures objects > 1024 bytes succeed
void test_large_allocation() {
    Allocator my_allocator;
    
    // Request 4096 bytes (a standard OS page size, far beyond our 1024 bucket)
    void* large_ptr = my_allocator.allocate(4096);
    assert(large_ptr != nullptr);
    
    my_allocator.deallocate(large_ptr);
    
    std::cout << "[PASS] test_large_allocation\n";
}

int main() {
    std::cout << "--- Starting Slab Allocator Test Suite ---\n";
    
    test_basic_allocation();
    test_write_memory();
    test_deallocation_recycling();
    test_all_bucket_sizes();
    test_zero_allocation();
    test_out_of_memory();
    test_alignment();
    test_stress_and_stability();
    test_large_allocation();

    std::cout << "--- All Tests Completed Successfully ---\n";
    return 0;
}