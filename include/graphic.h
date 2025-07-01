#ifndef graphic_h
#define graphic_h
#include "logic.h"
#include "common.h"
#include "player.h"

void drawTextWithFont(const char *text, float x, float y, TTF_Font *font, SDL_Renderer *renderer, SDL_Color fg, const char* quality);
int drawRect(float x, float y, float w, float h, SDL_Renderer *renderer, int r, int g, int b, int a, const char* type);
void drawGrid(Grid* grid, SDL_Renderer* renderer, SelectionMode selection_mode, Word* selected_word, Word words[], int words_count);
void drawHint(const char* text, float x, float y, float rectW, float rectH, TTF_Font* font, SDL_Renderer* renderer);
void drawWrappedText(const char *text, float x, float y, TTF_Font* font, SDL_Color fg, SDL_Renderer* renderer, int wrap_width_px);
void drawInfo(Player* player, float x, float y, float rectW, float rectH, TTF_Font* font, SDL_Renderer* renderer);
void updateDrawInfo(Player* player, TTF_Font* font, SDL_Renderer* renderer);

#endif