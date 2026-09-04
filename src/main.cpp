#include <iostream>
#include "../include/Allocator.hpp"

int main() {
    std::cout << "--- Custom Allocator Sandbox ---\n";
    
    // Boot up your engine!
    Allocator my_allocator;

    // 1. Allocate space for a single integer
    int* my_number = static_cast<int*>(my_allocator.allocate(sizeof(int)));
    *my_number = 99;
    std::cout << "Stored integer: " << *my_number << "\n";

    // 2. Allocate space for an array of doubles
    size_t array_size = sizeof(double) * 3;
    double* my_array = static_cast<double*>(my_allocator.allocate(array_size));
    
    my_array[0] = 3.14159;
    my_array[1] = 2.71828;
    std::cout << "Stored doubles: " << my_array[0] << " and " << my_array[1] << "\n";

    // 3. Trigger the Large Object Bypass for a massive 4096-byte buffer
    size_t massive_size = 4096;
    char* big_buffer = static_cast<char*>(my_allocator.allocate(massive_size));
    std::cout << "Successfully allocated a massive 4096-byte buffer!\n";

    // 4. Test the raw bytes by writing the alphabet into the buffer
    std::cout << "Writing raw bytes...\n";
    for (int i = 0; i < 26; i++) {
        // ASCII math: 'A' is 65, 'B' is 66, etc.
        big_buffer[i] = 'A' + i; 
    }

    // Null-terminate the string at the 27th byte so the print function knows when to stop
    big_buffer[26] = '\0'; 

    // Print the result directly from your custom heap!
    std::cout << "Data inside the buffer: " << big_buffer << "\n";

    // 5. Return the memory (Sized Deallocation)
    my_allocator.deallocate(my_number, sizeof(int));
    my_allocator.deallocate(my_array, array_size);
    my_allocator.deallocate(big_buffer, massive_size);

    std::cout << "All memory safely returned. Sandbox closing.\n";
    return 0;
}

// g++ -std=c++17 src/Allocator.cpp src/main.cpp -o sandbox
// ./sandbox