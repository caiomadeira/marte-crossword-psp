#ifndef APP_H
#define APP_H

#include "common.h"
#include "screen.h"

typedef struct app_t {
	SDL_Window *window;
    SDL_Renderer *renderer;
    SceCtrlData pad;
    // TTF_Font *font;
    // TTF_Font *hint_font;
    SceCtrlData prev_pad;
    /* Para gerenciamento das telas */
    Screen* current_screen;
    void* screen_data; // permite que cada tela tenha sua própria struct de dados (textura, pos de btns, etc)
    int running;
} app_t;

void set_screen(app_t *app, Screen * new_screen);

#endif