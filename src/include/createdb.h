#ifndef __CREATE_DATABASE_H__
#define __CREATE_DATABASE_H__

#include "champions_league.h"

#define CREATING_TABLES                "\n************ Creating the database tables...\n"
#define TABLES_CREATED_SUCCESSFULLY    "\n************ The tables have been created successfully.************\n"
#define ERROR_TEXT_ABORT               "\n************ ERROR: Aborting creating tables!\n"

status create_tables();

#endif