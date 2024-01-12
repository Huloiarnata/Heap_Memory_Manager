/* Libraries & Macros */
#include <stdint.h>

#define MM_MAX_STRUCT_NAME 32 //Max size of structure name.
#ifndef __MM__

typedef struct vm_page_{
    struct vm_page_ *next;
    struct vm_page_ *prev;
    struct vm_page_family_ *pg_family; /*back pointer*/
    //block_meta_data_t block_meta_data;
    char page_memory[0];
} vm_page_t;

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

