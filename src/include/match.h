#ifndef __MATCHES_H__
#define __MATCHES_H__

#include "champions_league.h"

#define GAME_SCORES "Game scores:"

#define ROUND_16_Leg_1_of_2  "Round 16 Leg 1 of 2"
#define ROUND_16_Leg_2_of_2  "Round 16 Leg 2 of 2"
#define QUATER_FINAL         "Quater-final"
#define SEMI_FINAL           "Semi-final"
#define BIG_FINAL            "Final"
#define INVALID_ROUND        "Invalid round"

#define NUMBER_OF_SOCCER_MATCHES        "Number of soccer matches"
#define NUMBER_OF_SOCCER_MATCHES_IN_DB  "The number of soccer matches in the database is"

status display_matches();

status display_number_of_soccer_matches();
	
#endif