# Custom Memory Manager for Linux

## Overview

This project implements a custom heap memory manager in Linux, focusing on efficient struct-based allocation and minimizing fragmentation. The key components include `MemoryManager.c`, `test_application.c`, `MM.h`, and `UserAPI_MemoryManager.h`.

## Memory Manager Implementation

### MemoryManager.c

1. **Initialization:**
   - `mm_init()`: Acquires the system page size using `getpagesize()`.

2. **Page Family Management:**
   - `mm_instantiate_new_page_family(struct_name, struct_size)`: Creates a new page family for a specific structure type.
   - `lookup_page_family_by_name(struct_name)`: Finds a page family by its struct name.
   - `mm_print_registered_page_families()`: Prints information about all registered page families.

3. **Memory Allocation:**
   - `xcalloc(struct_name, units)`: Allocates memory for multiple instances of a structure.
     - Locates the appropriate page family.
     - Finds a suitable free block or adds a new VM page if needed.
     - Splits the block if necessary and marks it as allocated.
     - Returns a pointer to the allocated memory.

4. **Memory Deallocation:**
   - `xfree(app_data)`: Frees a previously allocated memory block.
     - Marks the corresponding block as free.
     - Merges adjacent free blocks to reduce fragmentation.
     - Releases empty VM pages back to the kernel.

5. **Information and Debugging:**
   - `mm_print_block_usage()`: Prints statistics about block usage within each page family.
   - `mm_print_memory_usage(struct_name)`: Prints detailed memory usage information, optionally filtered by struct name.

6. **Internal Helper Functions:**
   - Various functions for managing VM pages, block metadata, free block lists, etc.

### test_application.c

1. **Includes and Structs:**
   - Includes `UserAPI_MemoryManager.h` for memory management functions.
   - Defines two structs: `emp_t` for employees and `student_t` for students.

2. **Main Function:**
   - Initializes the memory manager using `mm_init()`.
   - Registers the two structs with the memory manager using `MM_REG_STRUCT`.
   - Prints registered page families using `mm_print_registered_page_families()`.

3. **Scenario 1:**
   - Allocates memory for 3 employees and 2 students using `XCALLOC`.
   - Prints memory usage and block usage using `mm_print_memory_usage` and `mm_print_block_usage`.

4. **Scenario 2:**
   - Frees memory for emp1, emp3, and stud2 using `XFREE`.
   - Prints memory usage and block usage again.

5. **Scenario 3:**
   - Frees remaining memory for emp2 and stud1.
   - Prints final memory usage and block usage.

## Header Files

### MM.h

- Overview of structures, macros, and functions for managing memory allocation.
- Key Definitions: `vm_bool_t`, `block_meta_data_t`, `vm_page_t`, `vm_page_family_t`, `vm_page_for_families_t`.
- Macros and Function Prototypes for efficient memory management.
- Additional Insights: Linked list-based block and page management, priority list for free block retrieval.

### UserAPI_MemoryManager.h

- Overview of user-facing API for interacting with the memory manager.
- Key Functions: `mm_init`, `mm_instantiate_new_page_family`, `mm_print_registered_page_families`, `xcalloc`, `xfree`, `mm_print_memory_usage`, `mm_print_block_usage`.
- Additional Insights: Usage of XCALLOC and XFREE macros, user-friendly design, collaboration with MM.h.

## Additional Insights

- The output of `mm_print_memory_usage` and `mm_print_block_usage` provides valuable insights into the memory manager's performance.
- Analysis of output across different scenarios helps evaluate efficiency in handling diverse memory usage patterns.

## Usage and Examples

- Instructions on how to compile and run the test application.
- Step-by-step explanations of scenarios and expected output.

## Contributions

- Guidelines for contributing to the project, if applicable.

## License

- Mention of the project's license.
