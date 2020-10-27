#ifndef __DB_ENGINE_H__
#define __DB_ENGINE_H__

#define MANAGER_TABLE     "manager"
#define PLAYER_TABLE      "player"
#define TEAM_TABLE        "team"
#define MATCH_TABLE       "match"
#define ROUND_TABLE       "round"

#define NAME_MAX_LENGTH   32
#define DATE_MAX_LENGTH   24

typedef unsigned int u_int;

typedef struct{
    u_int id;
    char name[NAME_MAX_LENGTH];
} MANAGER;

typedef struct{
    u_int id;
    char name[NAME_MAX_LENGTH];
    u_int manager_id;
} TEAM;

typedef struct{
    TEAM team;
    MANAGER manager;
} TEAM_MANAGER;

typedef struct{
    u_int id;
    char name[NAME_MAX_LENGTH];
    u_int team_id;
} PLAYER;

typedef struct{
    PLAYER player;
    TEAM team;
} PLAYER_TEAM;

typedef enum{
    FIRST_LEG, SECOND_LEG, QUATER_FINALS, SEMI_FINALS, FINAL, NO_VALID_ROUND
} rounds;

typedef struct{
    u_int id;
    char name[NAME_MAX_LENGTH];
} ROUND;
    
typedef struct{
    u_int id;
    u_int team1_id;
    u_int team1_goals;
    u_int team2_id;
    u_int team2_goals;
    char date[DATE_MAX_LENGTH];
    u_int round_id;
} MATCH;

typedef struct{
    MATCH match;
    TEAM  team1;
    TEAM  team2;
} MATCH_TEAMS;

typedef enum{
    E_SUCCESS,
    E_OPENING_FILE,
    E_READING_FILE,
    E_WRITING_FILE,
    E_INVALID_FIELD,
    E_INVALID_ID,
    E_SELECT_NOT_FOUND,
    E_TABLE_FIELDS_NOT_FOUND,
    E_MEMORY_EXHAUSTED,
    E_TABLE_NOT_FOUND,
    E_ID_NOT_FOUND,
    E_FIELD_LIST_EMPTY
} status;


status db_get_number_of_records(const char *, int *);

status db_inner_join(const char *, const char *, void *);

status db_select(const char *, const char *, const char *, void *, void *);

status db_select_all_records(const char *, const char *, void**, int *);

status db_get_all_records(const char *, void *);

status db_get_record_by_id(const char *, const u_int, void *);

status db_import(const char *, const char *);

char *db_get_error_text(const status);
    
#endif