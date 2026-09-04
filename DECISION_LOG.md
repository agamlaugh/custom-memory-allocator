# Memory Management Journal: Decision Log

## Architecture Planning

**Entry 1: The Inspiration and Baseline Architecture Plan**
After thoroughly enjoying the foundational theory in my operating systems course, specifically hands-on memory operations and system calls in C, I decided to plan a project implementing these concepts from scratch in an unlimited environment. My initial blueprint utilized `mmap` with a "Next-Fit" search algorithm. To track the variable sizes of chunks, I mapped out a `BlockHeader` struct (storing the block size and a boolean status flag) that would sit physically in front of every single allocated piece of memory.

**Entry 2: The Bottleneck Discovery and Discarding Best-Fit**
During the whiteboarding phase, I realized Next-Fit would leave awkward memory gaps. I considered planning a "Best-Fit" algorithm instead to find the tightest possible space. However, through architectural mapping, I realized two massive flaws. Searching an implicit list for the best fit would create a severe linear $O(N)$ performance bottleneck, and allowing variable chunk sizes across the heap would inevitably lead to external fragmentation. I discarded Best-Fit as a viable solution before writing the code.

**Entry 3: The Slab Allocation Pivot**
My OS course had briefly touched on Slab Allocation. Intrigued by this modern approach and knowing it is used effectively under the hood in languages like Python, I completely pivoted my planned architecture to Segregated Free Lists (Slabs). By dedicating entire pages to specific block sizes (like a page strictly for 16-byte blocks), the allocator will be able to grab the first available block in $O(1)$ time. This design choice entirely defeats the need for a Best-Fit search and neutralizes external fragmentation.

**Entry 4: The Intrusive List and Metadata Optimization**
The biggest theoretical victory of the Slab pivot was realizing I could destroy the `BlockHeader`. Because every block in a slab is guaranteed to be the exact same size, I will not need to attach a size variable to every block. By planning an "intrusive linked list" (writing the next pointer directly into the empty space of free blocks), I can eliminate the metadata header and its associated 7-byte compiler padding entirely to achieve zero memory overhead.

---

## Implementation Journey

**Entry 5: Strict Test-Driven Development (TDD)**
Before writing the core allocator logic, I built a comprehensive TDD suite. One crucial test verified memory recycling by asserting that after freeing a block, the very next allocation request returns the exact same physical memory address (`ptr1 == ptr2`). This mathematical proof validates the immediate, LIFO (Last-In, First-Out) nature of the intrusive free list.

**Entry 6: Security vs. The C++ Standard (Dangling Pointers)**
While designing the `deallocate(void* ptr)` function, I heavily considered passing the pointer by reference (`void*& ptr`) so the allocator could automatically set it to `nullptr` and prevent Dangling Pointer vulnerabilities (Use-After-Free). However, I decided to stick to the C++ standard of passing by value. Nullifying a reference creates a false sense of security due to aliasing (copies of the pointer would remain dangling). Furthermore, forcing the CPU to write a zero violates the C++ zero-overhead principle. The allocator will stay fast and minimal, leaving pointer tracking to the developer.

**Entry 7: Handling Limits and Large Object Bypass**
To make the system production-ready, I expanded the test suite to check hardware limitations, including strict modulo-8 memory alignment and heavy-load stress testing (1,000 rapid allocations). I also mapped out a "Large Object Bypass." Since the slab buckets only handle powers of two up to 1024 bytes, requests larger than this will bypass the free list entirely. The allocator will make a custom `mmap` call directly to the OS for massive objects, temporarily bringing back an 8-byte header just for those blocks so `munmap` knows how much to return later.

**Entry 8: Modern Error Handling**
I initially planned for a 0-byte allocation request to silently return a `nullptr`, mimicking C's `malloc`. However, I pivoted to a modern C++ approach. The allocator is now strictly designed to throw a `std::invalid_argument` exception for 0-byte requests. This forces the calling program to handle the error immediately via try/catch, preventing silent failures from creeping into the system.

**Entry 9: Lazy Slab Carving and Bump Pointer Mechanics**
Rather than pre-carving the 1MB memory pool into fixed buckets upon initialization, which risks static fragmentation if workloads favor specific object sizes, I implemented a "lazy allocation" strategy using a bump pointer (`pool_offset`). Memory is carved dynamically in 4096-byte OS page chunks on demand when a bucket runs empty. The page is partitioned into uniform blocks and chained via intrusive pointers, terminating with `nullptr`. This amortizes initialization overhead while adapting the pool dynamically to runtime request distributions.

**Entry 10: Low-Level Optimization with Bitwise Arithmetic**
To optimize block-size derivation from bucket indices, I replaced runtime iterative multiplication loops with bitwise left shifts (`8 << index`). Because each bucket scale follows powers of two, the left-shift operation executes directly within the CPU's Arithmetic Logic Unit (ALU) via a single clock-cycle instruction, eliminating branching overhead and loop counter initialization during allocation.

**Entry 11: Explicit C++ Type Safety & Memory Overlap Lifecycle**
I transitioned some raw pointer offsets in the bypass path to explicit C++ `static_cast` expressions instead of C-style casting, improving type clarity and audibility. Additionally, I formalized the dual-lifecycle model of intrusive lists: blocks transition between holding an 8-byte `FreeBlock* next` pointer when managed by the allocator and storing raw user payload when active, maintaining zero metadata overhead per allocated block.

**Entry 12: TDD Validation and Integer Overflow Patch**
While running the test suite, the Out-Of-Memory (OOM) test caught a critical systems vulnerability. By requesting a massive allocation via `static_cast<size_t>(-1)`, the test forced an integer overflow in the Large Object Bypass calculation when 8 was added to the size. The overflow caused the requested size to mathematically wrap around to a tiny number, allowing the OS to falsely succeed instead of safely throwing `std::bad_alloc`. This perfectly validated my strict Test-Driven Development strategy. I patched the vulnerability by adding a mathematical wrap-around check (`total_size < size`) immediately before the `mmap` call, ensuring the allocator remains completely secure under extreme hardware stress limits.