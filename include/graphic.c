#include "graphic.h"

void drawTextWithFont(const char *text, float x, float y, TTF_Font *font, SDL_Renderer *renderer, SDL_Color fg, const char* quality)
{
    size_t length = strlen(text);
    SDL_Surface *s;

    if (strcmp(quality, "high") == 0)
        s = TTF_RenderText_Blended(font, text, length, fg);
    else if (strcmp(quality, "low") == 0)
        s = TTF_RenderText_Solid(font, text, length, fg);
    else
        s = TTF_RenderText_Solid(font, text, length, fg);
    
    
    if (s != NULL)
    {
        SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
        if (t != NULL)
        {
            SDL_FRect r;
            r.x = x;
            r.y = y;
            r.w = s->w;
            r.h = s->h;
            SDL_RenderTexture(renderer, t, NULL, &r);
            SDL_DestroyTexture(t);
        }
        SDL_DestroySurface(s);
    }
}

int drawRect(float x, float y, float w, float h, SDL_Renderer *renderer, int r, int g, int b, int a, const char* type)
{   
    if (renderer == NULL || type == NULL) {
        perror("No renderer or type instance.\n");
        return -1;
    }

    SDL_FRect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    if (strcmp(type, "filled") == 0)
        SDL_RenderFillRect(renderer, &rect);
    else if (strcmp(type, "border") == 0)
        SDL_RenderRect(renderer, &rect);
    else
        perror("Error: you need to choose between border or filled.\n");

    return 0;
}

void drawHint(const char* text, float x, float y, float rectW, float rectH, TTF_Font* font, SDL_Renderer* renderer) {
    if (text == NULL || font == NULL) return;
    
    SDL_Color SDL_RECT_COLOR = { 0, 0, 0, 255 };
    SDL_Color SDL_RECT_BORDER_COLOR = { 255, 255, 255, 255 };

    drawRect(x, y, rectW, rectH, renderer, SDL_RECT_COLOR.r, SDL_RECT_COLOR.g, SDL_RECT_COLOR.b, SDL_RECT_COLOR.a, "filled");
    drawRect(x + 5, y + 5, rectW - 10, rectH - 10, renderer, SDL_RECT_BORDER_COLOR.r, SDL_RECT_BORDER_COLOR.g, SDL_RECT_BORDER_COLOR.b, SDL_RECT_BORDER_COLOR.a, "border");
    
    int wrap_width = (int)rectW - 20;
    drawWrappedText("Hint: ", x + 10, y + 10, font, SDL_RECT_BORDER_COLOR, renderer, wrap_width);
    drawWrappedText(text, x + 10, y + 30, font, SDL_RECT_BORDER_COLOR, renderer, wrap_width);
}

void drawInfoBox(float x, float y, float rectW, float rectH, TTF_Font* font, SDL_Renderer* renderer) {
    if (font == NULL) return;
    
    SDL_Color SDL_RECT_COLOR = { 0, 0, 0, 255 };
    SDL_Color SDL_RECT_BORDER_COLOR = { 255, 255, 255, 255 };

    drawRect(x, y, rectW, rectH, renderer, SDL_RECT_COLOR.r, SDL_RECT_COLOR.g, SDL_RECT_COLOR.b, SDL_RECT_COLOR.a, "filled");
    drawRect(x + 5, y + 5, rectW - 10, rectH - 10, renderer, SDL_RECT_BORDER_COLOR.r, SDL_RECT_BORDER_COLOR.g, SDL_RECT_BORDER_COLOR.b, SDL_RECT_BORDER_COLOR.a, "border");
}

void drawScore(int score, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Color SDL_RECT_BORDER_COLOR = { 255, 255, 255, 255 };
    float x = (WINDOW_WIDTH / 2) + 20;
    float y = WINDOW_HEIGHT / 2;
    float rectW = (WINDOW_WIDTH / 2) - 50;
    float rectH = (WINDOW_HEIGHT / 2 ) - 15;
    int wrap_width = (int)rectW - 20;
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    drawWrappedText(scoreText, x + 10, y + 10, font, SDL_RECT_BORDER_COLOR, renderer, wrap_width);
}

void drawInfoStr(const char* str, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Color SDL_WHITE = { 255, 255, 255, 255 };
    float x = (WINDOW_WIDTH / 2) + 20;
    float y = (WINDOW_HEIGHT / 2) + 35;
    float rectW = (WINDOW_WIDTH / 2) - 50;
    float rectH = (WINDOW_HEIGHT / 2 ) - 15;
    int wrap_width = (int)rectW - 20;

    drawWrappedText(str, x + 10, y + 10, font, SDL_WHITE, renderer, wrap_width);
}

void drawWrappedText(const char *text, float x, float y, TTF_Font* font, SDL_Color fg, SDL_Renderer* renderer, int wrap_width_px) {
    if (text == NULL || font == NULL) return;
    SDL_Surface *s = TTF_RenderText_Blended_Wrapped(font, text, strlen(text), fg, wrap_width_px);
    if (s != NULL)
    {
        SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
        if (t != NULL)
        {
            SDL_FRect r;
            r.x = x;
            r.y = y;
            r.w = s->w;
            r.h = s->h;
            SDL_RenderTexture(renderer, t, NULL, &r);
            SDL_DestroyTexture(t);
        }
        SDL_DestroySurface(s);
    }
}

void drawGrid(Grid* grid, SDL_Renderer* renderer, SelectionMode selection_mode, Word* selected_word, Word words[], int words_count) {
    if (grid == NULL) return;
    SDL_Color rectBorderColor = { 0, 0, 0, 255 };

    for(int i = 0; i < grid->nrow; i++) {
        for(int j = 0; j < grid->ncol; j++) {
            Cell* cell = &grid->list_cells[i][j];

            if (cell->solution_letter == '\0') {
                drawRect(cell->x, cell->y, cell->w, cell->h, renderer, 0, 0, 0, 255, "filled");
                continue; // Pula para a próxima célula
            }

            // define cell bg color
            SDL_Color rectBackgroundColor;

            bool is_solved = false;
            Word* word_cell_horizontal = findWordAt(i, j, words, words_count, HORIZONTAL);
            Word* word_cell_vertical = findWordAt(i, j, words, words_count, VERTICAL);

            if ((word_cell_horizontal && word_cell_horizontal->is_solved) || (word_cell_vertical && word_cell_vertical->is_solved)) {
                is_solved = true;
            }

            if (is_solved) {
              rectBackgroundColor = (SDL_Color) { 144, 238, 144, 255 };  
            } else { 
                rectBackgroundColor = (SDL_Color) { 255, 255, 255, 255 }; 
            }
            
            if (!is_solved) {
                if (selection_mode == WORD_MODE && selected_word && selected_word->is_placed) {
                    if (findWordAt(i, j, words, words_count, selected_word->orientation) == selected_word) {
                        rectBackgroundColor = (SDL_Color){ 173, 216, 230, 255 };
                    }

                } else if (selection_mode == LETTER_MODE && selected_word && selected_word->is_placed) {
                    if (findWordAt(i, j, words, words_count, selected_word->orientation) == selected_word) {
                        rectBackgroundColor = (SDL_Color){ 211, 211, 211, 255 };
                    }
                }
            }

            if (i == grid->ai && j == grid->aj) 
                rectBackgroundColor = (SDL_Color){ 255, 255, 0, 255 };
            
            drawRect(cell->x, cell->y, cell->w, cell->h, renderer, rectBackgroundColor.r, rectBackgroundColor.g, rectBackgroundColor.b, rectBackgroundColor.a, "filled");
            drawRect(cell->x, cell->y, cell->w, cell->h, renderer, rectBorderColor.r, rectBorderColor.g, rectBorderColor.b, rectBorderColor.a, "border");
            
            char player_letter = cell->player_letter;
            if (player_letter >= 'A' && player_letter <= 'Z') {
                int texture_index = player_letter - 'A';
                SDL_Texture* letter_texture = grid->letter_textures_cache[texture_index];
                if (letter_texture != NULL) {
                    SDL_FRect dest_rect;
                    dest_rect.x = cell->x;
                    dest_rect.y = cell->y;

                    SDL_GetTextureSize(letter_texture, &dest_rect.w, &dest_rect.h); // antes era SDL_GetTextureSize

                    dest_rect.x = cell->x + (cell->w - dest_rect.w) / 2.0f;
                    dest_rect.y = cell->y + (cell->h - dest_rect.h) / 2.0f;
                    SDL_RenderTexture(renderer, letter_texture, NULL, &dest_rect);
                }
            }
        }
    }
}

void drawTime(Uint32 start_time, TTF_Font * font, SDL_Renderer* renderer) {
    if (font == NULL) return;

    float x = (WINDOW_WIDTH / 2) + 20;
    float y = WINDOW_HEIGHT / 2;
    float rectW = ((WINDOW_WIDTH / 2) - 50) - 20;
    int wrap_width = (int)rectW - 20;

    Uint32 current_ticks = SDL_GetTicks();
    Uint32 elapsedMS = current_ticks - start_time;
    Uint32 elapsed_seconds = elapsedMS / 1000;

    // formatando o tewmpo pra min:seg
    int minutes = elapsed_seconds / 60;
    int seconds = elapsed_seconds % 60;
    char timeText[32];

    snprintf(timeText, sizeof(timeText), "Time: %02d:%02d", minutes, seconds);

    SDL_Color SDL_TEXT_COLOR = { 255, 255, 255, 255 };
    drawWrappedText(timeText, x + 10, y + 25, font, SDL_TEXT_COLOR, renderer, wrap_width);
}

void drawInstructionBox(TTF_Font* font, SDL_Renderer* renderer, GameAssets* assets) {
    
    SDL_Color SDL_GRAY = {  128, 128, 128, 200 };
    SDL_Color SDL_WHITE = {  255, 255, 255, 255 };    
    
    float x = 10;
    float y = 10;
    float w = WINDOW_WIDTH;
    float h = WINDOW_HEIGHT;

    drawRect(x, y, w - 20, h - 20, renderer, SDL_GRAY.r, SDL_GRAY.g, SDL_GRAY.b, SDL_GRAY.a, "filled");
    
    x = x + 5;
    y = y + 5;
    float btn_w = 30;
    float btn_h = 30;
    int wrap_width = (int)btn_w - 20;

    #define POSY(y, w, multiplier, offset) (y + w * multiplier) + offset

    drawTextWithFont(CONTROLS_LABEL, 15, y + 5, font, renderer, SDL_WHITE, "high");
    drawTextWithFont(CROSS_BTN_INSTRUCTION, 60, y + strlen(CONTROLS_LABEL) + 40, font, renderer, SDL_WHITE, "high");
    drawTextWithFont(CIRCLE_BTN_INSTRUCTION, 60, y + strlen(CONTROLS_LABEL) + strlen(CROSS_BTN_INSTRUCTION) + 30, font, renderer, SDL_WHITE, "high");
    drawTextWithFont(TRIANGLE_BTN_INSTRUCTION, 60, y + strlen(CONTROLS_LABEL) + strlen(CROSS_BTN_INSTRUCTION) + strlen(CIRCLE_BTN_INSTRUCTION) + 30, font, renderer, SDL_WHITE, "high");
    drawTextWithFont(SQUARE_BTN_INSTRUCTION, 60, y + strlen(CONTROLS_LABEL) + strlen(CROSS_BTN_INSTRUCTION) + strlen(CIRCLE_BTN_INSTRUCTION) + strlen(TRIANGLE_BTN_INSTRUCTION) + 50, font, renderer, SDL_WHITE, "high");

    RenderTexture(assets->cross_btn_texture, x, y + 40, btn_w, btn_h, renderer);
    RenderTexture(assets->circle_btn_texture, x, POSY(y, btn_w, 1, 50), btn_w, btn_h, renderer);
    RenderTexture(assets->triangle_btn_texture, x, POSY(y, btn_w, 2, 60), btn_w, btn_h, renderer);
    RenderTexture(assets->square_btn_texture, x, POSY(y, btn_w, 3, 70), btn_w, btn_h, renderer);

    drawTextWithFont("Start: back to menu.", 60, y + strlen(CONTROLS_LABEL) + strlen(CROSS_BTN_INSTRUCTION) + strlen(CIRCLE_BTN_INSTRUCTION) + strlen(TRIANGLE_BTN_INSTRUCTION) + strlen(SQUARE_BTN_INSTRUCTION) + 40, font, renderer, SDL_WHITE, "high");
}

SDL_Surface* initImage(const char* image) {
    SDL_Surface *img_surface = IMG_Load(image);
    if (!img_surface) {
        printDebug(SDL_GetError(), 5000);
        return NULL;
    }
    return img_surface;
}

SDL_Texture* createImageTexture(SDL_Surface *img_surface, SDL_Renderer* renderer) {
    if (img_surface == NULL) return NULL;
    
    SDL_Texture *img_texture = SDL_CreateTextureFromSurface(renderer, img_surface);
    SDL_DestroySurface(img_surface);

    if (!img_texture) {
        printDebug(SDL_GetError(), 5000);
        return NULL; 
    }
    return img_texture;
}

void RenderTexture(SDL_Texture* texture, float x, float y, float w, float h, SDL_Renderer* renderer) {
    SDL_FRect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;
    SDL_RenderTexture(renderer, texture, NULL, &dest);
}