#include <stdio.h>

#include "createdb.h"

status create_tables(){
    
    printf(CREATING_TABLES);
    
    status error_code = db_import(MANAGER_TABLE, "import/managers.txt");
    CHECK_DB_ENGINE_ERROR(error_code)

    error_code = db_import(TEAM_TABLE, "import/teams.txt");
    CHECK_DB_ENGINE_ERROR(error_code)

    error_code = db_import(ROUND_TABLE, "import/rounds.txt");
    CHECK_DB_ENGINE_ERROR(error_code)
    
    error_code = db_import(MATCH_TABLE, "import/matches.txt");
    CHECK_DB_ENGINE_ERROR(error_code)
    
    error_code = db_import(PLAYER_TABLE, "import/players.txt");
    CHECK_DB_ENGINE_ERROR(error_code)
    
    printf(TABLES_CREATED_SUCCESSFULLY);
    
    return E_SUCCESS;
}  