#include "UserAPI_MemoryManager.h"
#include <stdint.h>
/* Structure Samples to test Linux Memory Manager */
typedef struct emp_{
    char name[32];
    uint32_t emp_id;
}emp_t;

typedef struct student_{
    char name[20];
    uint32_t roll_num;
    uint32_t marks[3];
    struct student_ *next;
}student_t;

int main(int argc, char **argv){
    memory_manager_init();
    MM_Register_Structure(emp_t);
    MM_Register_Structure(student_t);
    memory_manager_print_registered_page_families();

    XCALLOC(1, emp_t);
    XCALLOC(1, emp_t);
    XCALLOC(1, emp_t);
    
    XCALLOC(1, student_t);
    XCALLOC(1, student_t);

    // printing after i/p from user.
    scanf("\n");
    mm_print_memory_usage(0);
    mm_print_block_usage();
    return 0;
}