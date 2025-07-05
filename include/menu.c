#include "app.h"
#include "graphic.h"
#include "screen.h"
#include "audio.h"

/*
As funções são estáticas (privadas no caso) pra cada tela
*/

static void menu_init(app_t *app);
static void menu_handle_events(app_t *app);
static void menu_update(app_t *app);
static void menu_render(app_t *app);
static void menu_destroy(app_t *app);

Screen MENU_SCREEN = {
    .init = menu_init,
    .handle_events = menu_handle_events,
    .update = menu_update,
    .render = menu_render,
    .destroy = menu_destroy
};

typedef struct {
    SDL_Texture *background_texture;
    int menu_option;
} MenuData;

#define TOTAL_MENU_OPTIONS 3

void set_screen(app_t *app, Screen * new_screen) {
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
    MenuData *data = (MenuData*)malloc(sizeof(MenuData));
    if (!data) {
        app->running = 0; // // 0 quer dizer falha ao alocar e encerra o jogo
        return;
    }

    SDL_Surface* bg_surface = initImage("img/background2.png");
    data->background_texture = createImageTexture(bg_surface, app->renderer);
    data->menu_option = 0; // a opção começa no play
    app->screen_data = data;
}

static void menu_handle_events(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;
    if (!data) {
        app->running = 0;
        return;
    }

    readButtonState(&app->pad, 1);
    if ((app->pad.Buttons & PSP_CTRL_DOWN) && !(app->prev_pad.Buttons & PSP_CTRL_DOWN)) {
        data->menu_option = (data->menu_option + 1) % TOTAL_MENU_OPTIONS;
    }

    if ((app->pad.Buttons & PSP_CTRL_UP) && !(app->prev_pad.Buttons & PSP_CTRL_UP)) {
        data->menu_option = (data->menu_option - 1 + TOTAL_MENU_OPTIONS) % TOTAL_MENU_OPTIONS;
        
    }

    if ((app->pad.Buttons & PSP_CTRL_CROSS) && !(app->prev_pad.Buttons & PSP_CTRL_CROSS)) {
        if (data->menu_option == 0) {
            set_screen(app, &GAME_SCREEN);
        } else if (data->menu_option == 2) {
            app->running = 0;
        }
    }
}

/*

Semelhante ao app_iterate
*/
static void menu_update(app_t *app) {
    // MenuData *data = (MenuData*)app->screen_data;
    // if (data->menu_option == 0) {
    //     drawTextWithFont(">Play", 20, 20, app->font, app->renderer, (SDL_Color) { 255, 255, 255, 255 }, "high");
    // } else if (data->menu_option == 1) {
    //     drawTextWithFont(">Sair", 20, 60, app->font, app->renderer, (SDL_Color) { 255, 255, 255, 255 }, "high");
    // }
}

static void menu_render(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    SDL_RenderTexture(app->renderer, data->background_texture, NULL, NULL);

    // const char* option1 = "  Play";
    // const char* option2 = " Options";
    // const char* option3 = " Sair";

    // if (data->menu_option == 0) {
    //     option1 = "> Play";
    // } else if (data->menu_option == 1) {
    //     option2 = "> Options";
    // } else if (data->menu_option == 2) {
    //     option3 = "> Sair";
    // }

    // drawTextWithFont(option1, 20, WINDOW_HEIGHT - (WINDOW_HEIGHT / 2), app->font, app->renderer, (SDL_Color){ 255, 255, 255, 255 }, "high");
    // drawTextWithFont(option2, 20, WINDOW_HEIGHT - (WINDOW_HEIGHT / 2) + 30, app->font, app->renderer, (SDL_Color){ 255, 255, 255, 255 }, "high");
    // drawTextWithFont(option3, 20, WINDOW_HEIGHT - (WINDOW_HEIGHT / 2) + (30*2), app->font, app->renderer, (SDL_Color){ 255, 255, 255, 255 }, "high");

    SDL_RenderPresent(app->renderer);
}

static void menu_destroy(app_t *app) {
    MenuData *data = (MenuData*)app->screen_data;
    if (data) {
        SDL_DestroyTexture(data->background_texture);
        free(data);
    }
    app->screen_data = NULL;
}