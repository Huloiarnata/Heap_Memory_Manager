/* Including Libraries. */
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>   // Include <unistd.h> for getpagesize()
#include <sys/mman.h>
#include "MemoryManager.h"
#include "css.h"

/* Global variable to hold systems page size */
static size_t SYSTEM_PAGE_SIZE = 0;
/* Global variable to hold pointer to first vm page for Families */
static virtual_memory_page_families_t *first_vm_page_for_families = NULL;

/* Allocate new VM Page */
vm_page_t *allocate_vm_page(virtual_memory_page_family_t* virtual_memory_page_family){
    vm_page_t *vm_page = memory_manager_get_new_vm_from_kernel(1);

    MARK_VM_PAGE_EMPTY(vm_page);

    vm_page->block_meta_data.block_size = memory_manager_max_space_allocatable_memory(1);
    vm_page->block_meta_data.offset = offset_of(vm_page_t, block_meta_data);
    init_glthread(&vm_page->block_meta_data.priority_thread_glue);
    vm_page->next = NULL;
    vm_page->prev = NULL;

    vm_page->pg_family = virtual_memory_page_family;
    if(!virtual_memory_page_family->first_page){
        virtual_memory_page_family->first_page = vm_page;
        return vm_page;
    }

    vm_page->next = virtual_memory_page_family->first_page;
    virtual_memory_page_family->first_page->prev = vm_page;
    virtual_memory_page_family->first_page = vm_page;
    return vm_page;
}
/* For handling spliting and handlinsg internal fragmentation, partial, no, full splits in memory */
vm_bool_t mm_split_free_data_block_for_allocation(virtual_memory_page_family_t* vm_page_family, block_meta_data_t* block_meta_data, uint32_t size){
    block_meta_data_t* next_block_meta_data = NULL;

    assert(block_meta_data->is_free == MM_TRUE);
    
    if(block_meta_data->block_size<size){
        return MM_FALSE;
    }
    uint32_t remaining_size = block_meta_data->block_size - size;

    block_meta_data->is_free = MM_FALSE;
    block_meta_data->block_size = size;
    remove_glthread(&block_meta_data->priority_thread_glue);

    /* No split */
    if(!remaining_size){
        return MM_TRUE;
    }

    /* Partial split: soft internal fragmentation */
    else if(sizeof(block_meta_data_t)<remaining_size && remaining_size< (sizeof(block_meta_data_t)+ vm_page_family->struct_size)){
        next_block_meta_data = NEXT_META_BLOCK_BY_SIZE(block_meta_data);
        next_block_meta_data->is_free = MM_TRUE;
        next_block_meta_data->block_size = remaining_size - sizeof(block_meta_data_t);
        next_block_meta_data->offset = block_meta_data->offset + sizeof(block_meta_data_t) + block_meta_data->block_size;
        init_glthread(&next_block_meta_data->priority_thread_glue);
        mm_add_free_block_meta_data_to_free_block_list(vm_page_family, next_block_meta_data);
        memory_manager_bind_block_for_allocation(block_meta_data, next_block_meta_data);
    }
    /* Partial split: hard internal fragmentation */
    else if(remaining_size< sizeof(block_meta_data_t)){}

    /* full split */
    else{
        next_block_meta_data = NEXT_META_BLOCK_BY_SIZE(block_meta_data);
        next_block_meta_data->is_free = MM_TRUE;
        next_block_meta_data->block_size = remaining_size - sizeof(block_meta_data_t);
        next_block_meta_data->offset = block_meta_data->offset + sizeof(block_meta_data_t) + block_meta_data->block_size;
        init_glthread(&next_block_meta_data->priority_thread_glue);
        mm_add_free_block_meta_data_to_free_block_list(vm_page_family, next_block_meta_data);
        memory_manager_bind_block_for_allocation(block_meta_data, next_block_meta_data);
    }

    return MM_TRUE;
}

/* Allocating free data block for application to use */
static block_meta_data_t *mm_allocate_free_data_block(virtual_memory_page_family_t* vm_page_family, uint32_t requested_size){
    vm_bool_t status  = MM_FALSE;
    vm_page_t *vm_page = NULL;
    block_meta_data_t* block_meta_data= NULL;

    block_meta_data_t* biggest_block_meta_data = mm_get_biggest_free_block_page_family(vm_page_family);
    if(!biggest_block_meta_data || biggest_block_meta_data->block_size<requested_size){
        vm_page = mm_family_new_page_add(vm_page_family); //when empty.

        status = mm_split_free_data_block_for_allocation(vm_page_family, &vm_page->block_meta_data, requested_size);
        if(status){
            return &vm_page->block_meta_data;
        }
        return NULL; 
    }
    if(status){
        return biggest_block_meta_data;
    }
    return NULL;
}

/* To delete and free VM Page */
void memory_manager_page_delete_and_free(vm_page_t* vm_page){
    virtual_memory_page_family_t* virtual_memory_page_family = vm_page->pg_family;
    if(virtual_memory_page_family->first_page == vm_page){
        virtual_memory_page_family->first_page = vm_page->next;
        if(vm_page->next){
            vm_page->next->prev = NULL;
        }
        vm_page->next = NULL;
        vm_page->prev = NULL;
        memory_manager_release_vm_to_kernel((void*)vm_page, 1);
        return;
    }
    if(vm_page->next){
        vm_page->next->prev = vm_page->prev;
        vm_page->prev->next = vm_page->next;
        memory_manager_release_vm_to_kernel((void*)vm_page, 1);
    }
}

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
        init_glthread(&first_vm_page_for_families->virtual_memory_page_family[0].free_block_priority_list_head);
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
/* Block merging of free data blocks in consecutive manner. */
static void memory_manager_union_free_blocks(block_meta_data_t *first, block_meta_data_t * second){
    assert(first->is_free==MM_TRUE && second->is_free==MM_TRUE);
    first->block_size+=sizeof(block_meta_data_t) + second->block_size;
    first->next = second->next;
    if(second->next){
        second->next->prev = first;
    } 
}

/* Checks if VM Page is empty or not ie data block empty*/
vm_bool_t memory_manager_vm_page_empty(vm_page_t *vm_page){
    if(vm_page->next == NULL && vm_page->prev == NULL && vm_page->block_meta_data.is_free == MM_TRUE){
        return MM_TRUE;
    }
    return MM_FALSE;
}

/* Returns size of free Data block of an empty page */
static inline uint32_t memory_manager_max_space_allocatable_memory(int units){
    return(uint32_t)((SYSTEM_PAGE_SIZE*units) - offset_of(vm_page_t, page_memory));
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

/* Free block comparison function for comparing free data block size and arranging them. */
static int free_data_blocks_comparison_funct(void *_block_meta_data1, void *_block_meta_data2){
    block_meta_data_t *block_meta_data1 = (block_meta_data_t*)_block_meta_data1;
    block_meta_data_t *block_meta_data2 = (block_meta_data_t*)_block_meta_data2;

    if(block_meta_data1->block_size>block_meta_data2->block_size){
        return -1;
    }else if(block_meta_data1->block_size<block_meta_data2->block_size){
        return 1;
    }
    return 0;
}

static vm_page_t * mm_family_new_page_add(virtual_memory_page_family_t *vm_page_family){

    vm_page_t *vm_page = allocate_vm_page(vm_page_family);

    if(!vm_page)
        return NULL;

    /* The new page is like one free block, add it to the
     * free block list*/
    mm_add_free_block_meta_data_to_free_block_list(
            vm_page_family, &vm_page->block_meta_data);

    return vm_page;
}

/* Function invoked from user space for dynamic memory allocation */
void *xcalloc(char* struct_name, int units){
    //check for existence.
    virtual_memory_page_family_t* pg_family = lookup_page_family_by_name(struct_name);
    if(!pg_family){
        printf("Error! Structure not registered with memory manager.\n");
        return NULL;
    }

    if(units*pg_family->struct_size> MAX_PAGE_ALLOCATABLE_MEMORY(1)){
        printf("Error! Memory requested exceeds page size\n");
        return NULL;
    }

    block_meta_data_t *free_block_meta_data = NULL;
    free_block_meta_data = mm_allocate_free_data_block(pg_family, units*pg_family->struct_size);
    if(free_block_meta_data){
         memset((char *)(free_block_meta_data + 1), 0, 
         free_block_meta_data->block_size);
         return  (void *)(free_block_meta_data + 1);
    } 
    return NULL;
}

static void memory_manager_add_free_block_meta_data_to_free_block_list(virtual_memory_page_family_t* vm_page_family, block_meta_data_t* free_block){
    assert(free_block->is_free==MM_TRUE);
    glthread_priority_insert(&vm_page_family->free_block_priority_list_head,
            &free_block->priority_thread_glue,
            free_data_blocks_comparison_funct,
            offset_of(block_meta_data_t, priority_thread_glue));
}