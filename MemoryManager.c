/* Including Libraries. */
#include <stdio.h>
#include <memory.h>
#include <unistd.h>   // Include <unistd.h> for getpagesize()
#include <sys/mman.h>

/* Global variable to hold systems page size */
static size_t SYSTEM_PAGE_SIZE = 0;

/* Memory Manager Initializer: computing the system page size. */
void memory_manager_init() {
    // Size of virtual memory page is constant on the system (8192 Bytes on most system)
    SYSTEM_PAGE_SIZE = getpagesize();
}

/* API to request Virtual Memory Page from the kernel */
static void* memory_manager_get_new_vm_from_kernel(int units) {
    // Allocating vm_page units.
    char* virtual_memory_page = mmap(
        0,
        units * SYSTEM_PAGE_SIZE,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );

    // Error handling for allocation.
    if (virtual_memory_page == MAP_FAILED) {
        printf("Error: VM Page Allocation Failed!\n");
        return NULL;
    }

    memset(virtual_memory_page, 0, units * SYSTEM_PAGE_SIZE);
    return (void*)virtual_memory_page;
}

/* API to release Virtual Memory Page from the kernel */
static void memory_manager_release_vm_to_kernel(void* virtual_memory_page, int units) {
    if (munmap(virtual_memory_page, units * SYSTEM_PAGE_SIZE)) {
        printf("Error: VM Page not Released to Kernel!\n");
    }
}

int main(int argc, char** argv) {
    memory_manager_init();

    printf("Virtual Memory Page Size = %lu\n", SYSTEM_PAGE_SIZE);

    void* virtualPage1 = memory_manager_get_new_vm_from_kernel(1);
    void* virtualPage2 = memory_manager_get_new_vm_from_kernel(1);
    printf("Address of Page1 = %p, Page2 = %p\n", virtualPage1, virtualPage2);

    // Remember to release the allocated memory before the program exits.
    memory_manager_release_vm_to_kernel(virtualPage1, 1);
    memory_manager_release_vm_to_kernel(virtualPage2, 1);

    return 0;
}
