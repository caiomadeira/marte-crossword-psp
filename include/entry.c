#include "app.h"
#include "graphic.h"
#include "screen.h"
#include "audio.h"

static void entry_init(app_t *app);
static void entry_update(app_t *app);
static void entry_render(app_t *app);
static void entry_destroy(app_t *app);

Screen ENTRY_SCREEN = {
    .init = entry_init,
    .handle_events = NULL,
    .update = entry_update,
    .render = entry_render,
    .destroy = entry_destroy
};

typedef struct {
    SDL_Texture* logo_texture;
    Uint32 start_time;
    TTF_Font* entry_font;
} EntryData;

#define SPLASH_SCREEN_DURATION 2000

static void entry_init(app_t *app) {
    EntryData *data = (EntryData*)calloc(1, sizeof(EntryData));
    if (!data) {
        app->running = 0;
        return;
    }

    data->entry_font = TTF_OpenFont(GAME_OVER_TTF, 30);
    if (data->entry_font == NULL) {
        app->running = 0;
        return;
    }

    SDL_Surface* logo_surface = initImage(CROSS_BUTTON_PNG);
    if (logo_surface) {
        data->logo_texture = createImageTexture(logo_surface, app->renderer);
    }

    data->start_time = SDL_GetTicks();
    app->screen_data = data;
}

static void entry_update(app_t *app) {
    EntryData *data = (EntryData*)app->screen_data;
    if (!data) {
        app->running = 0;
        return;
    }

    if (SDL_GetTicks() - data->start_time > SPLASH_SCREEN_DURATION) {
        set_screen(app, &MENU_SCREEN);
    }
}

static void entry_render(app_t *app) {
    EntryData *data = (EntryData*)app->screen_data;
    if (!data) {
        app->running = 0;
        return;
    }

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255); // Fundo preto
    SDL_RenderClear(app->renderer);

    if (data->logo_texture) {
        RenderTexture(data->logo_texture, 100, 50, 280, 172, app->renderer);
    }

    drawTextWithFont("By Caio Madeira", 120, 230, data->entry_font, app->renderer, (SDL_Color){255,255,255,255}, "high");
    SDL_RenderPresent(app->renderer);
}

static void entry_destroy(app_t *app) {
    EntryData *data = (EntryData*)app->screen_data;
    if (data) {
        if (data->logo_texture) {
            SDL_DestroyTexture(data->logo_texture);
        }
        TTF_CloseFont(data->entry_font);
        free(data);
    }

    app->screen_data = NULL;
}