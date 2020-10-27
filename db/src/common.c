#include <string.h>
#include <ctype.h>

#include "common.h"

void to_uppercase(char *tablename_uppercase, const char *tablename){
    strcpy(tablename_uppercase, tablename);
    const char OFFSET = 'a' - 'A';
    
    while (*tablename_uppercase){
        char letter = *tablename_uppercase ;
        *tablename_uppercase = (letter >= 'a' && letter <= 'z') ? letter - OFFSET : letter;
        tablename_uppercase++;
    }
}

char *ltrim(char *str) {
    while(isspace(*str)) str++;
    return str;
}

char *rtrim(char *str){
    char* back = str + strlen(str);
    while(isspace(*--back));
    *(back + 1) = '\0';
    return str;
}

char *trim(char *str){
    return rtrim(ltrim(str));
}

int count_characters(const char *str, char character){
    int counter = 0;
    
    for(int i = 0; str[i] != '\0'; i++){
        if(str[i] == character){
            counter++;
        }    
    }
    
    return counter;
}