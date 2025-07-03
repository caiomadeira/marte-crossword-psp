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
void drawInfoBox(float x, float y, float rectW, float rectH, TTF_Font* font, SDL_Renderer* renderer);
void drawScore(int score, TTF_Font* font, SDL_Renderer* renderer);
void drawTime(Uint32 start_time, TTF_Font * font, SDL_Renderer* renderer);
void drawInstructionBox(TTF_Font* font, SDL_Renderer* renderer, GameAssets* assets);
SDL_Texture* createImageTexture(SDL_Surface *img_surface, SDL_Renderer* renderer);
SDL_Surface* initImage(const char* image);
void RenderTexture(SDL_Texture* texture, float x, float y, float w, float h, SDL_Renderer* renderer);
void drawInfoStr(const char* str, TTF_Font* font, SDL_Renderer* renderer);

#endif