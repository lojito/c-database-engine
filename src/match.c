#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "match.h"


static char *get_round_description(rounds round);
    
status display_matches(){
    int number_of_records = 0;
    
    status error_code = db_get_number_of_records(MATCH_TABLE, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    MATCH_TEAMS match_teams[number_of_records * sizeof(MATCH_TEAMS)];
    error_code = db_inner_join(TEAM_TABLE, MATCH_TABLE, match_teams);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    rounds previous_round = NO_VALID_ROUND;
    rounds current_round;

    printf("\n%s\n\n", GAME_SCORES);
    
    for (int i = 0; i < number_of_records; i++){    
        current_round = match_teams[i].match.round_id;
        if (current_round != previous_round){
            printf("******************* %s *******************\n\n",get_round_description(current_round));
            previous_round = current_round;
        }   

        printf("On %-10s %-20s %u-%-4u %s\n\n", match_teams[i].match.date, match_teams[i].team1.name, match_teams[i].match.team1_goals, match_teams[i].match.team2_goals, match_teams[i].team2.name);
    }
    
    return E_SUCCESS;
}

status display_number_of_soccer_matches(){
    int number_of_records = 0;
    
    status error_code = db_get_number_of_records(MATCH_TABLE, &number_of_records);
    CHECK_DB_ENGINE_ERROR(error_code)
    
    printf("\n%s:\n", NUMBER_OF_SOCCER_MATCHES);
    printf("%s: %d\n", NUMBER_OF_SOCCER_MATCHES_IN_DB, number_of_records);

    return E_SUCCESS;    
}
 
static char *get_round_description(const rounds round){
    char *str_round;
        
    switch(round){
        case FIRST_LEG:
            str_round = ROUND_16_Leg_1_of_2;
            break;
        case SECOND_LEG:
            str_round = ROUND_16_Leg_2_of_2;
            break;
        case QUATER_FINALS:
            str_round = QUATER_FINAL;
            break;
        case SEMI_FINALS:
            str_round = SEMI_FINAL;
            break;
        case FINAL:
            str_round = BIG_FINAL;
            break;
        default:
            str_round = INVALID_ROUND;
            break;          
    }

    return str_round;
}