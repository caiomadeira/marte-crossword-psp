#ifndef player_h
#define player_h

#include "common.h"

typedef struct player {
    char name[40];
    float score;
} Player;

Player* init_player(void);

#endif