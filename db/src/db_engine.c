#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "common.h"
#include "db_engine.h"

#define TABLENAME_MAX_LENGTH    20
#define FIELDNAME_MAX_LENGTH    20
#define SELECT_MAX_LENGTH       20
#define PATH_MAX_LENGTH         40

#define DATA_FOLDER             "db/data/"

#define E_TEXT_MAX_LENGTH   150
#define E_TEXT_OPENING_FILE       "ERROR: The program cannot open the %s table. You can recreate it by using the 'c' command."
#define E_TEXT_READING_FILE       "ERROR: The program encountered a problem while reading the '%s' table."
#define E_TEXT_WRITING_FILE       "ERROR: The program encountered a problem while writing to the '%s' table."
#define E_TEXT_INVALID_ID         "ERROR: Invalid id: %d."
#define E_TEXT_INVALID_FIELD      "ERROR: The field '%s' was not found in the database."
#define E_TEXT_SELECT             "ERROR: %s not found in the database."
#define E_TEXT_FIELDS_NOT_FOUND   "ERROR: Internal error. The fields definitions of the table '%s' were not found in the database."
#define E_TEXT_MEMORY             "ERROR: There is no enough memory to run this program at the moment."
#define E_TEXT_MANAGER_NOT_FOUND  "ERROR: Manager by the name of '%s' not found in the database."
#define E_TEXT_TEAM_NOT_FOUND     "ERROR: Team by the name of '%s' not found in the database."
#define E_TEXT_TABLE_NOT_FOUND    "ERROR: Internal error. The table '%s' was not found in the database."
#define E_TEXT_ID_NOT_FOUND       "ERROR: Internal error. A table was not found in the database."
#define E_TEXT_FIELD_LIST_EMPTY   "ERROR: You must specify at least one field when calling db_select()."
#define E_TEXT_SUCCESS            "No error found."

#define FROM_FIRST_RECORD    0
#define READ_ONLY_ONE_RECORD 1
#define UNREACHABLE_OFFSET   100

#define MAX_NUMBER_OF_FOREIGN_KEYS  2

#define GET_TABLE_ENTRY(tablename, table_entry) \
    TABLE const *table_entry = db_get_table_entry(tablename); \
    if (table_entry == NULL){ \
        strncpy(db_last_error.tblname, (char *)tablename, sizeof(db_last_error.tblname)); \
        return E_TABLE_NOT_FOUND; \
    }

#define GET_TABLE_ENTRY_BY_ID(id, table_entry) \
    TABLE const *table_entry = db_get_table_entry_by_id(id); \
    if (table_entry == NULL){ \
        return E_ID_NOT_FOUND;\
    }

#define GET_FIELD_ENTRY(table_entry, fieldname, field_entry) \
    FIELD const *field_entry = db_get_field_entry(table_entry->id, fieldname); \
    if (field_entry == NULL){ \
       snprintf(db_last_error.fldname, FIELDNAME_MAX_LENGTH, "%s.%s", table_entry->name, (char *)fieldname); \
       return E_INVALID_FIELD;\
    }

#define OPEN_FILE(path, mode) \
    FILE *fp = fopen(path, mode); \
    if (fp == NULL){ \
        strncpy(db_last_error.tblname, (char *)path, sizeof(db_last_error.tblname)); \
        return E_OPENING_FILE; \
    }

#define CLOSE_FILE(fp) \
    fclose(fp); \
    fp = NULL;

#define CHECK_ERROR \
    if (error_code != E_SUCCESS){ \
        return error_code; \
    }

#define GET_NUMBER_OF_RECORDS(tablename) \
    int number_of_records = 0; \
    error_code = db_get_number_of_records(tablename, &number_of_records); \
    CHECK_ERROR

static union{
    u_int id;
    char tblname[TABLENAME_MAX_LENGTH];
    char fldname[FIELDNAME_MAX_LENGTH];
    char select[SELECT_MAX_LENGTH];
} db_last_error;

typedef struct{
    u_int id;
    char name[TABLENAME_MAX_LENGTH];
    int record_size;
    u_int fields_number;
} TABLE;

static TABLE tables[] = {
    {.id = 0, .name = MANAGER_TABLE, .record_size = sizeof(MANAGER), .fields_number = 2},
    {.id = 1, .name = TEAM_TABLE,    .record_size = sizeof(TEAM),    .fields_number = 3},
    {.id = 2, .name = MATCH_TABLE,   .record_size = sizeof(MATCH),   .fields_number = 7},
    {.id = 3, .name = ROUND_TABLE,   .record_size = sizeof(ROUND),   .fields_number = 2},
    {.id = 4, .name = PLAYER_TABLE,  .record_size = sizeof(PLAYER),  .fields_number = 3}    
};

typedef enum{
    INTEGER, STRING
} field_type;

typedef struct {
    u_int id;
    u_int table_id;
    char name[FIELDNAME_MAX_LENGTH];
    u_int field_size;
    u_int offset;
    field_type type;
} FIELD;

static FIELD fields[] = {
    {.id = 0,   .table_id = 0, .name = "id",           .field_size = offsetof(MANAGER,name)      - offsetof(MANAGER,id),         .offset = offsetof(MANAGER,id),         .type = INTEGER},
    {.id = 1,   .table_id = 0, .name = "name",         .field_size = sizeof(MANAGER)             - offsetof(MANAGER,name),       .offset = offsetof(MANAGER,name),       .type = STRING },
    {.id = 2,   .table_id = 1, .name = "id",           .field_size = offsetof(TEAM,name)         - offsetof(TEAM,id),            .offset = offsetof(TEAM,id),            .type = INTEGER},
    {.id = 3,   .table_id = 1, .name = "name",         .field_size = offsetof(TEAM,manager_id)   - offsetof(TEAM,name),          .offset = offsetof(TEAM,name),          .type = STRING },
    {.id = 4,   .table_id = 1, .name = "manager_id",   .field_size = sizeof(TEAM)                - offsetof(TEAM,manager_id),    .offset = offsetof(TEAM,manager_id),    .type = INTEGER},
    {.id = 5,   .table_id = 2, .name = "id",           .field_size = offsetof(MATCH,team1_id)    - offsetof(MATCH,id),           .offset = offsetof(MATCH,id),           .type = INTEGER},
    {.id = 6,   .table_id = 2, .name = "team1_id",     .field_size = offsetof(MATCH,team1_goals) - offsetof(MATCH,team1_id),     .offset = offsetof(MATCH,team1_id),     .type = INTEGER},
    {.id = 7,   .table_id = 2, .name = "team1_goals",  .field_size = offsetof(MATCH,team2_id)    - offsetof(MATCH,team1_goals),  .offset = offsetof(MATCH,team1_goals),  .type = INTEGER},
    {.id = 8,   .table_id = 2, .name = "team2_id",     .field_size = offsetof(MATCH,team2_goals) - offsetof(MATCH,team2_id),     .offset = offsetof(MATCH,team2_id),     .type = INTEGER},
    {.id = 9,   .table_id = 2, .name = "team2_goals",  .field_size = offsetof(MATCH,date)        - offsetof(MATCH,team2_goals),  .offset = offsetof(MATCH,team2_goals),  .type = INTEGER},
    {.id = 10,  .table_id = 2, .name = "date",         .field_size = offsetof(MATCH,round_id )   - offsetof(MATCH,date ),        .offset = offsetof(MATCH,date),         .type = STRING },
    {.id = 11,  .table_id = 2, .name = "round_id",     .field_size = sizeof(MATCH)               - offsetof(MATCH,round_id),     .offset = offsetof(MATCH,round_id),     .type = INTEGER},
    {.id = 12,  .table_id = 3, .name = "id",           .field_size = offsetof(ROUND,name )       - offsetof(ROUND,id),           .offset = offsetof(ROUND,id),           .type = INTEGER},
    {.id = 13,  .table_id = 3, .name = "name",         .field_size = sizeof(ROUND)               - offsetof(ROUND,name),         .offset = offsetof(ROUND,name),         .type = STRING },
    {.id = 14,  .table_id = 4, .name = "id",           .field_size = offsetof(PLAYER,name)       - offsetof(PLAYER,id ),         .offset = offsetof(PLAYER,id),          .type = INTEGER},
    {.id = 15,  .table_id = 4, .name = "name",         .field_size = offsetof(PLAYER,team_id)    - offsetof(PLAYER,name),        .offset = offsetof(PLAYER,name),        .type = STRING },
    {.id = 16,  .table_id = 4, .name = "team_id",      .field_size = sizeof(PLAYER)              - offsetof(PLAYER,team_id),     .offset = offsetof(PLAYER,team_id),     .type = INTEGER}    
};
    
typedef struct{
    u_int id;
    u_int parent_id;
    u_int field_id;
    u_int offset;
    u_int child_id;
} FOREIGNKEY;

static FOREIGNKEY foreign_keys[] = {
    {.id = 0,   .field_id = 4,    .child_id = 1,   .parent_id = 0,   .offset = offsetof(TEAM_MANAGER,  team.manager_id)  },
    {.id = 1,   .field_id = 6,    .child_id = 2,   .parent_id = 1,   .offset = offsetof(MATCH_TEAMS,   match.team1_id )  },
    {.id = 2,   .field_id = 8,    .child_id = 2,   .parent_id = 1,   .offset = offsetof(MATCH_TEAMS,   match.team2_id )  },
    {.id = 3,   .field_id = 11,   .child_id = 2,   .parent_id = 3,   .offset = offsetof(MATCH_TEAMS,   match.team2_id )  },
    {.id = 4,   .field_id = 16,   .child_id = 4,   .parent_id = 1,   .offset = offsetof(PLAYER_TEAM,   player.team_id )  }    
};

static status db_get_fields(const u_int table_id, const char* tablename, FIELD table_fields[]);

static status count_lines(const char *path, u_int* const lines_number);

static status load(const char *path, const u_int fields_number, char *buffer[][fields_number]);

static FOREIGNKEY* db_get_foreign_key_entry(const u_int field_id);

static status db_load_table(const char *tablename, void const * const data, const u_int number_of_records);

static void db_get_foreign_key_offsets(const u_int parent_id, const u_int child_id, u_int foreign_key_offsets[]);

static status db_get_records(const char * const tablename, void * const buffer, const u_int id, const u_int number_of_records_to_read);

static TABLE const *db_get_table_entry(const char * const tablename);

static TABLE const *db_get_table_entry_by_id(const u_int table_id);

static FIELD const *db_get_field_entry(const u_int table_id, const char *filename);

static status get_fields_entries(TABLE const *table_entry, char *fields, FIELD select_field_entries[], int *total_fields_size);

status db_get_number_of_records(const char *tablename, int *number_of_records){
    GET_TABLE_ENTRY(tablename, table_entry)

    char table_path[PATH_MAX_LENGTH] = {'\0'};
    snprintf(table_path, PATH_MAX_LENGTH, "%s%s%s", DATA_FOLDER, table_entry->name, ".bin");

    OPEN_FILE(table_path, "rb")
    fseek(fp, 0, SEEK_END);
    *number_of_records = ftell(fp) / table_entry->record_size;
    CLOSE_FILE(fp)

    return E_SUCCESS;
}

status db_inner_join(const char *parent_tablename, const char *child_tablename, void* buffer){
    GET_TABLE_ENTRY(parent_tablename, parent_entry)
    int parent_record_size = parent_entry->record_size;

    GET_TABLE_ENTRY(child_tablename, child_entry)
    int child_record_size = child_entry->record_size;

    status error_code;
    GET_NUMBER_OF_RECORDS(child_tablename)
    char child_buffer[number_of_records * child_record_size];
    error_code = db_get_all_records(child_tablename, child_buffer);
    CHECK_ERROR

    u_int foreign_key_offsets[] = {UNREACHABLE_OFFSET, UNREACHABLE_OFFSET};
    db_get_foreign_key_offsets(parent_entry->id, child_entry->id, foreign_key_offsets);

    int total_size = parent_record_size + child_record_size;
    if (foreign_key_offsets[1] != UNREACHABLE_OFFSET){
        total_size += parent_record_size;
    }

    char parent1_buffer[parent_record_size];
    char parent2_buffer[parent_record_size];

    for (int i = 0; i < number_of_records; i++){
        char *p_child_current_record = child_buffer + i * child_record_size;
        char *p_buffer_current_record = buffer + total_size * i;
        memcpy(p_buffer_current_record, p_child_current_record, child_record_size);

        char *p_buffer_parent = p_buffer_current_record + child_record_size;
        for (int j = 0; j < MAX_NUMBER_OF_FOREIGN_KEYS; j++){
            if (foreign_key_offsets[j] != UNREACHABLE_OFFSET){
                u_int id = *(u_int*)(p_child_current_record + foreign_key_offsets[j]);
                char *parent_buffer = (j == 1) ? parent1_buffer : parent2_buffer;
                error_code = db_get_record_by_id(parent_tablename, id, parent_buffer);
                CHECK_ERROR
                memcpy(p_buffer_parent, parent_buffer, parent_record_size);
            }
            p_buffer_parent += parent_record_size;
        }
    }

    return E_SUCCESS;
}

status db_select(const char *field, const char *table, const char* where_field, void* where_value, void* result){
    GET_TABLE_ENTRY(table, from_table_entry);
    int from_record_size = from_table_entry->record_size;

    GET_FIELD_ENTRY(from_table_entry, field, select_field_entry);
    field_type select_field_type = select_field_entry->type;
    u_int select_field_offset = select_field_entry->offset;

    GET_FIELD_ENTRY(from_table_entry, where_field, where_field_entry);
    field_type where_field_type = where_field_entry->type;
    u_int where_field_offset = where_field_entry->offset;

    status error_code;
    GET_NUMBER_OF_RECORDS(table)
    char from_table_buffer[number_of_records * from_record_size];
    error_code = db_get_all_records(table, from_table_buffer);
    CHECK_ERROR

    bool found = false;
    for (int i = 0; i < number_of_records; i++){
        char *p_from_table_current_record = from_table_buffer + i * from_record_size;
        if (where_field_type == STRING){
            if (strcmp((char*)(p_from_table_current_record + where_field_offset), (char*)where_value) == 0){
                if (select_field_type == INTEGER){
                    *(int*)result = *(int *)(p_from_table_current_record + select_field_offset);
                    found = true;
                    break;
                }
            }
        }
    }

    if (found){
        return E_SUCCESS;
    } else {
        memset(db_last_error.select, '\0', SELECT_MAX_LENGTH);
        snprintf(db_last_error.select, SELECT_MAX_LENGTH, "%s.%s:%s", table, where_field, (char *)where_value);
        return E_SELECT_NOT_FOUND;
    }
}

status db_select_all_records(const char *fields, const char *table, void** buffer, int *number_of_rec){
    GET_TABLE_ENTRY(table, table_entry);
    int record_size = table_entry->record_size;
    
    char fields_copy[strlen(fields) + 1];
    strcpy(fields_copy, fields);
    int number_of_fields = count_characters(fields, ',') + 1;
    FIELD field_entries[number_of_fields];
    
    int total_fields_size = 0;
    status error_code = get_fields_entries(table_entry, fields_copy, field_entries, &total_fields_size);
    CHECK_ERROR
        
    GET_NUMBER_OF_RECORDS(table)
    char table_buffer[number_of_records * record_size];
    *number_of_rec = number_of_records;
    error_code = db_get_all_records(table, table_buffer);
    CHECK_ERROR
    
    *buffer = malloc(number_of_records * total_fields_size);
    if (buffer == NULL){               
       return E_MEMORY_EXHAUSTED;
    }
    
    u_int offset = 0;
    for (int i = 0; i < number_of_records; i++){
        char *p_table_current_record = table_buffer + i * record_size;
        for (int j = 0; j < number_of_fields; j++){
            u_int file_size = field_entries[j].field_size;
            memcpy((char*)(*buffer) + offset, p_table_current_record + field_entries[j].offset, file_size);
            offset += file_size;
        }   
    }
    
    return E_SUCCESS;

}

status db_get_all_records(const char *tablename, void *buffer){
    status error_code;
    GET_NUMBER_OF_RECORDS(tablename)

    return db_get_records(tablename, buffer, FROM_FIRST_RECORD, number_of_records);
}

status db_get_record_by_id(const char *tablename, const u_int id, void *buffer){
    status error_code;    
    GET_NUMBER_OF_RECORDS(tablename)

    if (id >= number_of_records){
        db_last_error.id = id;
        return E_INVALID_ID;
    }

    return db_get_records(tablename, buffer, id, READ_ONLY_ONE_RECORD);
}

status db_import(const char *tablename, const char* path){
    GET_TABLE_ENTRY(tablename, table_entry)
    u_int fields_number = table_entry->fields_number;
    FIELD table_fields[fields_number];
    status error_code = db_get_fields(table_entry->id, table_entry->name, table_fields);
    CHECK_ERROR

    u_int lines_number = 0;
    error_code = count_lines(path, &lines_number);
    CHECK_ERROR

    char *buffer[lines_number][fields_number];
    error_code = load(path, fields_number, buffer);
    CHECK_ERROR

    char binary_buffer[lines_number * table_entry->record_size];

    char * p_record = binary_buffer;
    for (int i = 0; i < lines_number; i++){
        char * p_field = p_record;
        u_int field_size = 0;
        for (int j = 0; j < fields_number; j++){
            field_size = table_fields[j].field_size;
            if (j == 0){
                *((int *)p_field) = i;
            } else {
                FOREIGNKEY const *foreign_key_entry = db_get_foreign_key_entry(table_fields[j].id);
                if (foreign_key_entry){
                    GET_TABLE_ENTRY_BY_ID(foreign_key_entry->parent_id, parent_table_entry);
                    u_int field_id = 0;
                    error_code = db_select("id", parent_table_entry->name, "name", buffer[i][j], &field_id);
                    CHECK_ERROR
                    *((int *)p_field) = field_id;
                } else {
                    if (table_fields[j].type == INTEGER){
                        *((int *)p_field) = atoi(buffer[i][j]);
                    } else {
                        memset(p_field, '\0', field_size);
                        strncpy(p_field, buffer[i][j], field_size);
                    }
                }
                free(buffer[i][j]);
            }
            p_field += field_size;
        }
        p_record += table_entry->record_size;
    }
        
    db_load_table(tablename, binary_buffer, lines_number);
    CHECK_ERROR

    return E_SUCCESS;
}

char *db_get_error_text(const status error_code){
    static char error_text[E_TEXT_MAX_LENGTH];
    
    switch(error_code){
        case E_OPENING_FILE:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_OPENING_FILE, db_last_error.tblname);
            break;
        case E_READING_FILE:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_READING_FILE, db_last_error.tblname);
            break;
        case E_WRITING_FILE:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_WRITING_FILE, db_last_error.tblname);
            break;
        case E_INVALID_ID:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_INVALID_ID, db_last_error.id);
            break;
        case E_INVALID_FIELD:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_INVALID_FIELD, db_last_error.fldname);
            break;
        case E_SELECT_NOT_FOUND:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_SELECT, db_last_error.select);
            break;
        case E_TABLE_FIELDS_NOT_FOUND:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_FIELDS_NOT_FOUND, db_last_error.tblname);
            break;
        case E_TABLE_NOT_FOUND:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_TABLE_NOT_FOUND, db_last_error.tblname);
            break;     
        case E_MEMORY_EXHAUSTED:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_MEMORY);
            break;                  
        case E_ID_NOT_FOUND:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_ID_NOT_FOUND);
            break;
        case E_FIELD_LIST_EMPTY:
            snprintf(error_text, E_TEXT_MAX_LENGTH, E_TEXT_FIELD_LIST_EMPTY);        
            break;
        default :
            strncpy(error_text, E_TEXT_SUCCESS, E_TEXT_MAX_LENGTH);
    }
    
    return error_text;
}

static status db_get_fields(const u_int table_id, const char* tablename, FIELD table_fields[]){
    u_int index = 0;
    bool found = false;
    for (int i = 0; i < sizeof(fields) / sizeof(fields[0]); i++){
        if (fields[i].table_id == table_id){
            found = true;
            memcpy(&table_fields[index], &fields[i], sizeof(FIELD));
            index++;
        }
    }
    if (found){
        return E_SUCCESS;
    } else {
        strncpy(db_last_error.tblname, tablename, sizeof(db_last_error.tblname));
        return E_TABLE_FIELDS_NOT_FOUND;
    }
}

static status count_lines(const char *path, u_int* const lines_number){
    *lines_number = 0;
    char *p_line = NULL;
    size_t line_size = 0;
    
    OPEN_FILE(path, "r")
    while (getline(&p_line, &line_size, fp) != -1){
        (*lines_number)++;
    }
    CLOSE_FILE(fp)

    return E_SUCCESS;
}
 
static status load(const char *path, const u_int fields_number, char *buffer[][fields_number]){ 
    OPEN_FILE(path, "r")

    char *p_line = NULL;
    size_t line_size = 0;
    ssize_t characters_read = 0;
    int i = 0;

    while ((characters_read = getline(&p_line, &line_size, fp)) != -1){
        if (strlen(p_line) < characters_read){
            p_line[strlen(p_line)] = ' ';
        }
        char *token = strtok(p_line, ",");
        int j = 1;
        while (token != NULL){
            strcpy(token, trim(token));
            buffer[i][j] = calloc(strlen(token) + 1, 1);
            if (buffer[i][j] == NULL){               
                return E_MEMORY_EXHAUSTED;
            }
            strcpy(buffer[i][j], token);
            j++;
            token = strtok(NULL, ",");
        }
        i++;
    }

    CLOSE_FILE(fp)
    free(p_line);
    p_line = NULL;
    
    return E_SUCCESS;
}

static FOREIGNKEY* db_get_foreign_key_entry(const u_int field_id){
    for (int i = 0; i < sizeof(foreign_keys) / sizeof(foreign_keys[0]); i++){
        if (foreign_keys[i].field_id == field_id){
            return &foreign_keys[i];
        }
    }

    return NULL;
}

static status db_load_table(const char *tablename, const void * const data, const u_int number_of_records){
    GET_TABLE_ENTRY(tablename, table_entry)

    char table_path[PATH_MAX_LENGTH] = {'\0'};
    snprintf(table_path, PATH_MAX_LENGTH, "%s%s%s", DATA_FOLDER, table_entry->name, ".bin");

    OPEN_FILE(table_path, "wb")
    int number_of_records_written = fwrite(data, table_entry->record_size, number_of_records, fp);
    CLOSE_FILE(fp)

    if (number_of_records_written != number_of_records){
        strncpy(db_last_error.tblname, (char *)tablename, sizeof(db_last_error.tblname));
        return E_WRITING_FILE;
    }

    return E_SUCCESS;
}

static void db_get_foreign_key_offsets(const u_int parent_id, const u_int child_id, u_int foreign_key_offsets[]){
    unsigned char index = 0;

    for (int i = 0; i < sizeof(foreign_keys) / sizeof(FOREIGNKEY); i++){
        if ((foreign_keys[i].parent_id == parent_id) && (foreign_keys[i].child_id == child_id)){
            foreign_key_offsets[index] = foreign_keys[i].offset;
            index++;
        }
    }
}

static status db_get_records(const char * const tablename, void * const buffer, const u_int id, const u_int number_of_records_to_read){
    GET_TABLE_ENTRY(tablename, table_entry)

    char table_path[PATH_MAX_LENGTH] = {'\0'};
    snprintf(table_path, PATH_MAX_LENGTH, "%s%s%s", DATA_FOLDER, table_entry->name, ".bin");
    int record_size = table_entry->record_size;

    OPEN_FILE(table_path, "rb")
    fseek(fp, id * record_size, SEEK_SET);
    int number_of_records_read = fread(buffer, record_size, number_of_records_to_read, fp);
    CLOSE_FILE(fp)

    if (number_of_records_read != number_of_records_to_read){
        strncpy(db_last_error.tblname, (char *)tablename, sizeof(db_last_error.tblname));
        return E_READING_FILE;
    }

    return E_SUCCESS;
}

static TABLE const *db_get_table_entry(const char * const tablename){
    char tablename_uppercase[TABLENAME_MAX_LENGTH];
    to_uppercase(tablename_uppercase, tablename);
    char current_tablename_uppercase[TABLENAME_MAX_LENGTH];

    for (int i = 0; i < sizeof(tables) / sizeof(TABLE); i++){
        to_uppercase(current_tablename_uppercase, tables[i].name);
        if (strcmp(current_tablename_uppercase, tablename_uppercase) == 0){
            return &tables[i];
        }
    }

    return NULL;
}

static TABLE const *db_get_table_entry_by_id(const u_int table_id){
    for (int i = 0; i < sizeof(tables) / sizeof(TABLE); i++){
        if (tables[i].id == table_id){
            return &tables[i];
        }
    }

    return NULL;
}

static FIELD const *db_get_field_entry(const u_int table_id, const char *filename){
    char fieldname_uppercase[FIELDNAME_MAX_LENGTH];
    to_uppercase(fieldname_uppercase, filename);
    char current_fieldname_uppercase[FIELDNAME_MAX_LENGTH];

    for (int i = 0; i < sizeof(fields) / sizeof(FIELD); i++){
        to_uppercase(current_fieldname_uppercase, fields[i].name);
        if ((fields[i].table_id == table_id) && (strcmp(current_fieldname_uppercase, fieldname_uppercase) == 0)){
                return &fields[i];
        }
    }

    return NULL;
}

static status get_fields_entries(TABLE const *table_entry, char *fields, FIELD select_field_entries[], int *total_fields_size){
    char *field = strtok(fields, ",");
    int i = 0;
    
    *total_fields_size = 0;
    while (field != NULL){
        strcpy(field, trim(field));
        GET_FIELD_ENTRY(table_entry, field, select_field_entry);
        *total_fields_size += select_field_entry->field_size;
        select_field_entries[i] = *select_field_entry;
        i++;
        field = strtok(NULL, ",");
    }
    if (*total_fields_size == 0){
       return E_FIELD_LIST_EMPTY; 
    }    
    return E_SUCCESS;    
}