#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "team.h"

typedef struct{
    char name[NAME_MAX_LENGTH];
} TEAM_RESULT_SET;

status display_teams(){
    int number_of_records = 0;
    TEAM_RESULT_SET *p_teams = NULL;
    
    status error_code = db_select_all_records("name", TEAM_TABLE, (void**)&p_teams, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)

    printf("\n%-30s\n", TEAM_TEXT);
    printf("------------------------------\n");
    for (int i = 0; i < number_of_records; i++){
        printf("%-30s\n", p_teams[i].name);
    }

    free(p_teams);
    return E_SUCCESS;
}

status display_teams_and_managers(){
    int number_of_records = 0;
    
    status error_code = db_get_number_of_records(TEAM_TABLE, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    TEAM_MANAGER teams_manager[number_of_records * sizeof(TEAM_MANAGER)];
    error_code = db_inner_join(MANAGER_TABLE, TEAM_TABLE, teams_manager);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    printf("\n%-30s%s\n", TEAM_TEXT, MANAGER_TEXT);
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < number_of_records; i++){
        printf("%-30s%s\n", teams_manager[i].team.name, teams_manager[i].manager.name);
    }
    
    return E_SUCCESS;
}
