#include "app.h"
#include "graphic.h"
#include "screen.h"
#include "audio.h"

/*
As funções são estáticas (privadas no caso) pra cada tela
*/

static void menu_init(app_t *app);
static void menu_handle_events(app_t *app);
// static void menu_update(app_t *app);
static void menu_render(app_t *app);
static void menu_destroy(app_t *app);

Screen MENU_SCREEN = {
    .init = menu_init,
    .handle_events = menu_handle_events,
    .update = NULL,
    .render = menu_render,
    .destroy = menu_destroy
};

typedef struct {
    SDL_Texture *background_texture;
    int menu_option;
    char* option_str;
    TTF_Font* menu_font;
} MenuData;

#define TOTAL_MENU_OPTIONS 3
const char* MENU_OPTIONS[TOTAL_MENU_OPTIONS] = {
    "Play", "Options", "Quit"
};

void set_screen(app_t *app, Screen * new_screen) {
    if (app->renderer == NULL) {
        printDebug("fatal: renderer is NULL", 3000);
        app->running = 0;
        return;
    }
    /* Se já existe uma tela chama a funcao destroy pra limpar tudo */
    if (app->current_screen != NULL && app->current_screen->destroy != NULL) {
        app->current_screen->destroy(app);
    }

    // aponta o ponteiro p/ nova tela
    app->current_screen = new_screen;
    app->screen_data = NULL;

    // se rolar tudo certo, chama a funcao de inicializacao
    if (app->current_screen != NULL && app->current_screen->init != NULL) {
        app->current_screen->init(app);
    }
}

static void menu_init(app_t *app) {
    MenuData *data = (MenuData*)calloc(1, sizeof(MenuData));
    if (!data) {
        app->running = 0; // // 0 quer dizer falha ao alocar e encerra o jogo
        return;
    }

    SDL_Surface* bg_surface = initImage("img/background2.png");
    if (bg_surface) {
        data->background_texture = createImageTexture(bg_surface, app->renderer);
        SDL_DestroySurface(bg_surface);
    } else {
        printDebug("Erro ai carregar background.", 3000);
        data->background_texture = NULL;
    }

    data->menu_font = TTF_OpenFont(GAME_OVER_TTF, 60);
    if (data->menu_font == NULL) {
        app->running = 0;
        return;
    }

    if (!data->background_texture) {
        free(data);
        app->running = 0;
        return;
    }

    data->menu_option = 0; // a opção começa no play
    app->screen_data = data;
}

static void menu_handle_events(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;
    if (!data) {
        app->running = 0;
        return;
    }

    // readButtonState(&app->pad, 1);
    if ((app->pad.Buttons & PSP_CTRL_DOWN) && !(app->prev_pad.Buttons & PSP_CTRL_DOWN)) {
        data->menu_option = (data->menu_option + 1) % TOTAL_MENU_OPTIONS;
    }

    if ((app->pad.Buttons & PSP_CTRL_UP) && !(app->prev_pad.Buttons & PSP_CTRL_UP)) {
        data->menu_option = (data->menu_option - 1 + TOTAL_MENU_OPTIONS) % TOTAL_MENU_OPTIONS;
        
    }

    if ((app->pad.Buttons & PSP_CTRL_CROSS) && !(app->prev_pad.Buttons & PSP_CTRL_CROSS)) {
        switch (data->menu_option)
        {
            case 0:
                set_screen(app, &GAME_SCREEN); break;
            case 1: break;
            case 2:
                app->running = 0;
        }
        
    }
}

/* Semelhante ao app_iterate */
// static void menu_update(app_t *app)

static void menu_render(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;
    if (!data) {
        app->running = 0;
        return;
    }

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    if (data->background_texture) {
        SDL_RenderTexture(app->renderer, data->background_texture, NULL, NULL);
    }

    SDL_Color white = { 0, 0, 0, 255 };

    int start_y = 120;
    int step_y = 30;

    for(int i = 0; i < TOTAL_MENU_OPTIONS; i++) {
        char buffer[50];
        if (i == data->menu_option) {
            snprintf(buffer, sizeof(buffer), "> %s", MENU_OPTIONS[i]);
        } else {
            snprintf(buffer, sizeof(buffer), "  %s", MENU_OPTIONS[i]);
        }
        
        // CORREÇÃO 3: Usamos a posição Y calculada, que muda a cada iteração.
        int current_y = start_y + (i * step_y);
        drawTextWithFont(buffer, 40, current_y, data->menu_font, app->renderer, white, "high");
    }
    SDL_RenderPresent(app->renderer);
}

static void menu_destroy(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;
    if (data) {
        if (data->background_texture) {
           SDL_DestroyTexture(data->background_texture);
        }
        TTF_CloseFont(data->menu_font);
        free(data);
    }
    app->screen_data = NULL;
}