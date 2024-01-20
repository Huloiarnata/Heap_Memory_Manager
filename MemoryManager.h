/* Libraries & Macros */
#ifndef __MM__
#define MM_MAX_STRUCT_NAME 32 //Max size of structure name.

#include <stdint.h>
#include "gluethread/glthread.h"

// enum for memory occupied or free.
typedef enum{
    MM_FALSE,
    MM_TRUE
} vm_bool_t;


/*
Structure to store registration information from application in userspace.
This will help kernel to know how much memory to allocate or release for application.
Registration Info: Structure Name, Structure Size.
*/
typedef struct virtual_memory_page_family{
    char  struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
    vm_page_t *first_page;
} virtual_memory_page_family_t;


/* 
Structure to store array of vm_page_family structure items.
These structure are in linked list structure so as to navigate from one page to another
when searching or memory occupied in one page.
*/
typedef struct virtual_memory_page_families{
    struct virtual_memory_page_families *next;
    virtual_memory_page_family_t virtual_memory_page_family[0];
} virtual_memory_page_families_t;

typedef struct block_meta_data{
    vm_bool_t is_free;
    uint32_t offset;
    uint32_t block_size;
    glthread_t priority_thread_glue;
    struct block_meta_data *next;
    struct block_meta_data *prev;


}block_meta_data_t;

typedef struct vm_page_{
    struct vm_page_ *next;
    struct vm_page_ *prev;
    struct vm_page_family_ *pg_family; /*back pointer*/
    block_meta_data_t block_meta_data;
    char page_memory[0];
} vm_page_t;

#define offset_of(container_structure, field_name)  \
    ((size_t)&(((container_structure *)0)->field_name))

#define MM_GET_PAGE_FROM_META_BLOCK(block_meta_data_ptr)    \
    ((void * )((char *)block_meta_data_ptr - block_meta_data_ptr->offset))

#define NEXT_META_BLOCK(block_meta_data_ptr)                \
    (block_meta_data_ptr->next_block)

#define NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr)        \
    (block_meta_data_t *)((char *)(block_meta_data_ptr + 1) \
        + block_meta_data_ptr->block_size)

#define PREV_META_BLOCK(block_meta_data_ptr)    \
    (block_meta_data_ptr->prev_block)

// This macros handles allocation of free space upon xmalloc.
#define memory_manager_bind_block_for_allocation(allocated_meta_block, free_meta_block){\
    free_meta_block->prev_block = allocated_meta_block;\
    free_meta_block->next_block = allocated_meta_block->next_block;\
    allocated_meta_block->next_block = free_meta_block;\
    if(free_meta_block->next_block){\
        free_meta_block->next_block->prev_block = free_meta_block;\
    }\
}


/* Max Number of family in vm_Families */
#define MAX_Families_PER_VM_PAGE   \
(SYSTEM_PAGE_SIZE - sizeof(virtual_memory_page_families_t*)/\
sizeof(virtual_memory_page_family_t))

/* Looping Macro Begin for iterating family within Families */
#define Iterate_Page_Families_Begin(virtual_memory_page_Families_ptr, curr){ \
    uint32_t _count = 0; \
    for(curr = (virtual_memory_page_family_t *)&virtual_memory_page_Families_ptr->virtual_memory_page_family[0]; \
        curr->struct_size && _count < MAX_Families_PER_VM_PAGE; \
        curr++, _count++){

#define Iterate_Page_Families_End(virtual_memory_page_Families_ptr, curr) }}


#endif/**/

