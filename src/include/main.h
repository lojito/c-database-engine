#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>

#include "manager.h"
#include "team.h"
#include "player.h"
#include "match.h"
#include "createdb.h"

#define ENTER_COMMAND                    "Enter a command or 'q' to quit."
#define DISPLAY_MANAGERS_COMMAND         "'m'=managers"
#define DISPLAY_TEAMS_COMMAND            "'t'=teams"
#define DISPLAY_TEAMS_MANAGERS_COMMAND   "'u'=teams and their managers"
#define DISPLAY_GAMES_COMMAND            "'g'=game scores"
#define DISPLAY_TOTAL_MATCHES_COMMAND    "'n'=total of soccer matches"
#define DISPLAY_PLAYERS_COMMAND          "'p'=players"
#define DISPLAY_PLAYERS_TEAMS_COMMAND    "'r'=players and their teams"
#define CREATE_TABLES_COMMAND            "'c'=create database"
#define QUITING_COMMAND                  "Quitting...\n\n\nHappy EUFA Champions League 2019-2020!"
#define INVALID_COMMAND                  "Invalid command."

#define WELCOME_MESSAGE "\nWelcome to the UEFA Champions League 2019-2020!\n"

#endif