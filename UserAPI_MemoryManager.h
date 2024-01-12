/* Provides publicaly exposed structures & APIs of Linux Memory Manager.
Acts as an interface between LMM and Application.*/

#ifndef __UAPI_MM__
#define __UAPI_MM__
#include <stdint.h>

/* Initialization */
void memory_manager_init();

/* Registration function */
void memory_manager_instantiate_new_page_family(char *struct_name, uint32_t struct_size);

/* memory_manager_print_registered_page_families() */
void memory_manager_print_registered_page_families();

#define MM_Register_Structure(struct_name)\
    (memory_manager_instantiate_new_page_family(#struct_name, sizeof(struct_name)))


#endif /*__UAPI_MM__*/