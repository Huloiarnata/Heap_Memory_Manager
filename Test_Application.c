#include "UserAPI_MemoryManager.h"
#include <stdint.h>
#include <stdio.h>
/* Structure Samples to test Linux Memory Manager */
typedef struct emp_{
    char name[32];
    uint32_t emp_id;
}emp_t;

typedef struct student_ {

    char name[32];
    uint32_t rollno;
    uint32_t marks_phys;
    uint32_t marks_chem;
    uint32_t marks_maths;
    struct student_ *next;
} student_t;

int main(int argc, char **argv){
    memory_manager_init();
    MM_Register_Structure(emp_t);
    MM_Register_Structure(student_t);
    memory_manager_print_registered_page_families();

    int wait;
    emp_t *emp1 = XCALLOC(1, emp_t);
    emp_t *emp2 = XCALLOC(1, emp_t);
    emp_t *emp3 = XCALLOC(1, emp_t);

    student_t *stud1 = XCALLOC(1, student_t);
    student_t *stud2 = XCALLOC(1, student_t);

    printf(" \nSCENARIO 1 : *********** \n");
    mm_print_memory_usage(0);
    mm_print_block_usage();


    scanf("%d", &wait); 

    XFREE(emp1);
    XFREE(emp3);
    XFREE(stud2);
    printf(" \nSCENARIO 2 : *********** \n");
    mm_print_memory_usage(0);
    mm_print_block_usage();


    scanf("%d", &wait); 
    
    XFREE(emp2);
    XFREE(stud1);
    printf(" \nSCENARIO 3 : *********** \n");
    mm_print_memory_usage(0);
    mm_print_block_usage();
    return 0;
}