#ifndef menu_h
#define menu_h

#include "common.h"

typedef enum {
    GAMESCREEN_ENTRY_CREDITS,
    GAMESCREEN_MENU,
    GAMESCREEN_SELECT_COLLECTION, // player select games
    GAMESCREEN_CHANGE_THEME,
    GAMESCREEN_AUTHOR_SCREEN,
} GameScreen;

#define GAMESCREEN_OPTIONS 5;

void mainMenu(GameScreen* currentScreen, app_state* app, int* option);

#endif