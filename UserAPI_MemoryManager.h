/* Provides publicaly exposed structures & APIs of Linux Memory Manager.
Acts as an interface between LMM and Application.*/

#ifndef __UAPI_MM__
#define __UAPI_MM__
#include <stdint.h>

void *xcalloc(char *struct_name, int units);
//void xfree(void* ptr);

#define XCALLOC(units, struct_name)\
    (xcalloc(#struct_name, units))

#define XFREE(ptr)\
    (xfree(ptr))
/* Initialization */
void memory_manager_init();

/* Registration function */
void memory_manager_instantiate_new_page_family(char *struct_name, uint32_t struct_size);

#define MM_Register_Structure(struct_name)\
    (memory_manager_instantiate_new_page_family(#struct_name, sizeof(struct_name)))

void mm_print_memory_usage(char* struct_name);
/* memory_manager_print_registered_page_families() */
void memory_manager_print_registered_page_families();
void mm_print_block_usage();


#endif /* __UAPI_MM__ */