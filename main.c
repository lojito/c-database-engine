#include "main.h"

int main(){
    int loop = 1;
    char c = '\0';
    
    printf(WELCOME_MESSAGE);

    while( loop ) {
        printf("\n%s", ENTER_COMMAND);
        printf("\nCommands: "DISPLAY_MANAGERS_COMMAND", "DISPLAY_PLAYERS_COMMAND", "DISPLAY_PLAYERS_TEAMS_COMMAND", "DISPLAY_TEAMS_COMMAND", "DISPLAY_TEAMS_MANAGERS_COMMAND", "DISPLAY_GAMES_COMMAND", "DISPLAY_TOTAL_MATCHES_COMMAND", "CREATE_TABLES_COMMAND" : ");

        scanf(" %c", &c);
        switch(c) {
            case 'm':
                display_managers();
                break;
            case 'p':
                display_players();
                break;
            case 'r':
                display_players_and_teams();
                break;              
            case 't':
                display_teams();
                break;
            case 'u':
                display_teams_and_managers();
                break;                
            case 'g':
                display_matches();
                break;
            case 'n':
                display_number_of_soccer_matches();
                break;
            case 'c': ;
                status error_code = create_tables();
                if (error_code != E_SUCCESS){
                    printf(ERROR_TEXT_ABORT);
                }    
                break;          
            case 'q':
                printf("%s\n", QUITING_COMMAND);
                loop = 0;
                break;
            default:
                printf("%s\n", INVALID_COMMAND);
                break;
        }
    }
    return 0;   
}   