# OS Memory Allocator

This repository contains an Operating Systems project that implements a small custom memory allocator in C++.

The allocator manages a fixed-size heap, backed by `mmap`, and provides replacements for the basic `malloc` and `free` workflow:

- `my_malloc(size_t size)` finds a free block using a first-fit search and splits it when allocating memory.
- `my_free(void *ptr)` returns memory to the free list and coalesces adjacent free blocks.
- Helper functions expose allocator state for debugging and testing, such as available memory, the number of free nodes, and free-list printing.

## Project Structure

- `src/my_malloc.cpp` - allocator implementation
- `include/my_malloc.h` - allocator interface and shared constants/types
- `src/main.cpp` - small demo program that allocates and frees memory
- `test/test.cpp` - GoogleTest-based unit tests
- `Makefile` - build, test, clean, and submission targets

## Building

Build the demo app, test binary, and submission archive:

```sh
make
```

Build outputs include:

- `allocator_app`
- `allocator_test`
- `submission.zip`

## Running

Run the demo program:

```sh
./allocator_app
```

The demo prints available heap memory before and after allocations/frees, along with the current free list.

Run the tests:

```sh
./allocator_test
```

The tests check heap initialization, free-list state, and a simple allocation.

## Cleaning

Remove generated object files, binaries, and the submission archive:

```sh
make clean
```

## Notes

The allocator uses a fixed `HEAP_SIZE` of 4096 bytes and stores metadata in block headers. It is intended as an educational operating systems project rather than a production allocator.
