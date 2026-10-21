#include <benchmark/benchmark.h>
#include <cstdlib>
#include <new>
#include <vector>
#include "../include/Allocator.hpp"

// Block sizes cycled through by the mixed-size scenarios (one per slab bucket).
static const size_t kSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};

// ---------------------------------------------------------
// Scenario 1: Hot churn (best case for the slab allocator)
// Allocate one block, touch it, free it, repeat. The LIFO free list hands
// back the same block every time, so this is a pointer pop + push.
// ---------------------------------------------------------
static void BM_Churn_Custom(benchmark::State& state) {
    Allocator alloc;
    const size_t size = state.range(0);
    for (auto _ : state) {
        char* p = static_cast<char*>(alloc.allocate(size));
        p[0] = 1;
        benchmark::DoNotOptimize(p);
        alloc.deallocate(p, size);
    }
}

static void BM_Churn_Malloc(benchmark::State& state) {
    const size_t size = state.range(0);
    for (auto _ : state) {
        char* p = static_cast<char*>(std::malloc(size));
        p[0] = 1;
        benchmark::DoNotOptimize(p);
        std::free(p);
    }
}

static void BM_Churn_NewDelete(benchmark::State& state) {
    const size_t size = state.range(0);
    for (auto _ : state) {
        char* p = static_cast<char*>(::operator new(size));
        p[0] = 1;
        benchmark::DoNotOptimize(p);
        ::operator delete(p, size);
    }
}

// ---------------------------------------------------------
// Scenario 2: Batch of mixed small objects (a "normal" workload)
// Allocate N objects of mixed sizes, touch each, then free them all in
// reverse order. Models building and tearing down a data structure.
// ---------------------------------------------------------
static void BM_Batch_Custom(benchmark::State& state) {
    Allocator alloc;
    const int n = state.range(0);
    std::vector<char*> ptrs(n);
    for (auto _ : state) {
        for (int i = 0; i < n; i++) {
            ptrs[i] = static_cast<char*>(alloc.allocate(kSizes[i % 8]));
            ptrs[i][0] = 1;
        }
        benchmark::DoNotOptimize(ptrs.data());
        for (int i = n - 1; i >= 0; i--) alloc.deallocate(ptrs[i], kSizes[i % 8]);
    }
    state.SetItemsProcessed(state.iterations() * n);
}

static void BM_Batch_Malloc(benchmark::State& state) {
    const int n = state.range(0);
    std::vector<char*> ptrs(n);
    for (auto _ : state) {
        for (int i = 0; i < n; i++) {
            ptrs[i] = static_cast<char*>(std::malloc(kSizes[i % 8]));
            ptrs[i][0] = 1;
        }
        benchmark::DoNotOptimize(ptrs.data());
        for (int i = n - 1; i >= 0; i--) std::free(ptrs[i]);
    }
    state.SetItemsProcessed(state.iterations() * n);
}

static void BM_Batch_NewDelete(benchmark::State& state) {
    const int n = state.range(0);
    std::vector<char*> ptrs(n);
    for (auto _ : state) {
        for (int i = 0; i < n; i++) {
            ptrs[i] = static_cast<char*>(::operator new(kSizes[i % 8]));
            ptrs[i][0] = 1;
        }
        benchmark::DoNotOptimize(ptrs.data());
        for (int i = n - 1; i >= 0; i--) ::operator delete(ptrs[i], kSizes[i % 8]);
    }
    state.SetItemsProcessed(state.iterations() * n);
}

// ---------------------------------------------------------
// Scenario 3: Large objects (worst case for the slab allocator)
// Anything over 1024 bytes goes straight to mmap/munmap, so every call is
// a system call. Included to show the cost of the bypass honestly.
// ---------------------------------------------------------
static void BM_Large_Custom(benchmark::State& state) {
    Allocator alloc;
    const size_t size = state.range(0);
    for (auto _ : state) {
        char* p = static_cast<char*>(alloc.allocate(size));
        p[0] = 1;
        benchmark::DoNotOptimize(p);
        alloc.deallocate(p, size);
    }
}

static void BM_Large_Malloc(benchmark::State& state) {
    const size_t size = state.range(0);
    for (auto _ : state) {
        char* p = static_cast<char*>(std::malloc(size));
        p[0] = 1;
        benchmark::DoNotOptimize(p);
        std::free(p);
    }
}

BENCHMARK(BM_Churn_Custom)->Arg(16)->Arg(64)->Arg(512);
BENCHMARK(BM_Churn_Malloc)->Arg(16)->Arg(64)->Arg(512);
BENCHMARK(BM_Churn_NewDelete)->Arg(16)->Arg(64)->Arg(512);

BENCHMARK(BM_Batch_Custom)->Arg(512);
BENCHMARK(BM_Batch_Malloc)->Arg(512);
BENCHMARK(BM_Batch_NewDelete)->Arg(512);

BENCHMARK(BM_Large_Custom)->Arg(4096);
BENCHMARK(BM_Large_Malloc)->Arg(4096);

BENCHMARK_MAIN();
