/* Including Libraries. */
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>   // Include <unistd.h> for getpagesize()
#include <sys/mman.h>
#include "MemoryManager.h"

/* Global variable to hold systems page size */
static size_t SYSTEM_PAGE_SIZE = 0;
/* Global variable to hold pointer to first vm page for Families */
static virtual_memory_page_families_t *first_vm_page_for_families = NULL;

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

/* Page Family Instantiation */

void memory_manager_instantiate_new_page_family(char *struct_name, uint32_t struct_size) {
    virtual_memory_page_family_t *virtual_memory_page_family_curr = NULL;
    virtual_memory_page_families_t *new_vm_page_for_families = NULL;

    // The structure demands more size than can be provided.
    if (struct_size > SYSTEM_PAGE_SIZE) {
        printf("Error: %s() structure %s size exceeds system page size\n", __FUNCTION__, struct_name);
        return;
    }

    // Handling no first vm Families page.
    if (!first_vm_page_for_families) {
        first_vm_page_for_families = (virtual_memory_page_families_t *)memory_manager_get_new_vm_from_kernel(1);
        first_vm_page_for_families->next = NULL;
        strncpy(first_vm_page_for_families->virtual_memory_page_family[0].struct_name, struct_name, MM_MAX_STRUCT_NAME);
        first_vm_page_for_families->virtual_memory_page_family[0].struct_size = struct_size;
        return;
    }

    uint32_t count = 0;
    Iterate_Page_Families_Begin(first_vm_page_for_families, virtual_memory_page_family_curr) {
        if (strcmp(virtual_memory_page_family_curr->struct_name, struct_name) != 0) {
            count++;
            continue;
        }
        assert(0);
    } Iterate_Page_Families_End(first_vm_page_for_families, virtual_memory_page_family_curr);

    if(count == MAX_Families_PER_VM_PAGE) {
        new_vm_page_for_families = (virtual_memory_page_families_t *)memory_manager_get_new_vm_from_kernel(1);
        new_vm_page_for_families->next = first_vm_page_for_families;
        first_vm_page_for_families = new_vm_page_for_families;
        virtual_memory_page_family_curr = &first_vm_page_for_families->virtual_memory_page_family[0];
    }

    strncpy(virtual_memory_page_family_curr->struct_name, struct_name, MM_MAX_STRUCT_NAME);
    virtual_memory_page_family_curr->struct_size = struct_size;
    virtual_memory_page_family_curr->first_page = NULL;
}

virtual_memory_page_family_t *lookup_page_family_by_name(char *struct_name){

    virtual_memory_page_family_t *vm_page_family_curr = NULL;
    virtual_memory_page_families_t *vm_page_families_curr = NULL;

    for(vm_page_families_curr = first_vm_page_for_families; vm_page_families_curr; vm_page_families_curr = vm_page_families_curr->next){
        Iterate_Page_Families_Begin(vm_page_families_curr, vm_page_family_curr){
            if(strncmp(vm_page_family_curr->struct_name, struct_name, MM_MAX_STRUCT_NAME) == 0){
                return vm_page_family_curr;
            }
        } Iterate_Page_Families_End(vm_page_families_curr, vm_page_family_curr);
    }
    return NULL;
}

/* Prints all the families registered with LMM */
void memory_manager_print_registered_page_families(){
    virtual_memory_page_family_t *vm_page_family_curr = NULL;
    virtual_memory_page_families_t *vm_page_families_curr = NULL;

    for(vm_page_families_curr = first_vm_page_for_families; vm_page_families_curr; vm_page_families_curr = vm_page_families_curr->next){
        Iterate_Page_Families_Begin(vm_page_families_curr, vm_page_family_curr){
            printf("Page Family : %s, Size = %u\n", vm_page_family_curr->struct_name, vm_page_family_curr->struct_size);
        } Iterate_Page_Families_End(vm_page_families_curr, vm_page_family_curr);
    }
}
