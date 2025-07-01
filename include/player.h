#ifndef player_h
#define player_h

#include "common.h"

typedef struct player {
    char name[40];
    int score;
} Player;

Player* init_player(void);

#endif