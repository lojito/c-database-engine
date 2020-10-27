#include <stdio.h>
#include <stdlib.h>

#include "player.h"

typedef struct{
    char name[NAME_MAX_LENGTH];
} PLAYER_RESULT_SET;

status display_players(){
    int number_of_records = 0;
    PLAYER_RESULT_SET *p_players = NULL;
    
    status error_code = db_select_all_records("name", PLAYER_TABLE, (void**)&p_players, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)

    printf("\n%-30s\n", PLAYER_TEXT);
    printf("------------------------------\n");
    for (int i = 0; i < number_of_records; i++){
        printf("%-30s\n", p_players[i].name);
    }

    free(p_players);
    return E_SUCCESS;
}

status display_players_and_teams(){
    int number_of_records = 0;
    
    status error_code = db_get_number_of_records(PLAYER_TABLE, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    PLAYER_TEAM players_team[number_of_records * sizeof(PLAYER_TEAM)];
    error_code = db_inner_join(TEAM_TABLE, PLAYER_TABLE, players_team);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    printf("\n%-30s%s\n", PLAYER_TEXT, TEAM_TEXT);
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < number_of_records; i++){
        printf("%-30s%s\n", players_team[i].player.name, players_team[i].team.name);
    }
    
    return E_SUCCESS;
}