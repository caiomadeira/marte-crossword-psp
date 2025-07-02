#pragma once 

#include <SDL3/SDL.h>

#define ROAST_CHICKEN_TTF "font/roasted-chicken.ttf"
#define GAME_OVER_TTF "font/game_over.ttf"
#define TEST_WAV "audio/finish.wav"
#define CORRECT_WAV "audio/correct.wav"
#define BACKGROUND_PNG "img/background.png"

// SPRITES
#define CROSS_BUTTON_PNG "img/xbtn.png"
#define CIRCLE_BUTTON_PNG "img/obtn.png"
#define TRIANGLE_BUTTON_PNG "img/trianglebtn.png"
#define SQUARE_BUTTON_PNG "img/squarebtn.png"

// TODO: Dar um jeito de tirar essas texturas daqui
typedef struct {
    SDL_Texture *cross_btn_texture;
    SDL_Texture *circle_btn_texture;
    SDL_Texture *triangle_btn_texture;
    SDL_Texture *square_btn_texture;
} GameAssets;
