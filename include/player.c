#include "player.h"

Player* init_player(void) 
{
    Player* player = (Player*)malloc(sizeof(Player));
    if (player == NULL) return NULL;
    player->score = 0;
    
    player->name = (char*)malloc(41*sizeof(char));
    if (player->name == NULL) return NULL;
    strcpy(player->name, "");
    player->name[strlen(player->name)] = '\0';

    return player;
}