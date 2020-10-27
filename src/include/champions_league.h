#ifndef __CHAMPIONS_LEAGUE_H__
#define __CHAMPIONS_LEAGUE_H__

#include "db_engine.h"

#define CHECK_DB_ENGINE_ERROR(error_code) \
        if (error_code != E_SUCCESS){ \
            printf("\n************ %s\n", db_get_error_text(error_code)); \
            return error_code; \
        }

#endif