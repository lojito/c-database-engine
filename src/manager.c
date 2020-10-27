#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manager.h"

typedef struct{
    char name[NAME_MAX_LENGTH];
} MANAGER_RESULT_SET;

status display_managers(){
    int number_of_records = 0;
    MANAGER_RESULT_SET *p_managers = NULL;
    
    status error_code = db_select_all_records("name", MANAGER_TABLE, (void**)&p_managers, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)    
    
    printf("\n%s\n", MANAGER_TEXT);
    printf("------------------------------\n");
    
    for (int i = 0; i < number_of_records; i++){
        printf("%s\n",p_managers[i].name);
    }
  
    free(p_managers);
    return E_SUCCESS;    
}