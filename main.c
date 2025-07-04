#include "include/common.h"
#include "include/player.h"
#include "include/audio.h"
#include "include/graphic.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "include/app.h"
#include "include/screen.h"
 
// COLORS SDL
SDL_Color SDL_WHITE = { 255, 255, 255, 255 };
SDL_Color SDL_PURPLE = { 192, 92, 255, 255 };

int init_SDL(void) {
    // Init SDL_INIT_VIDEO
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }

    // INIT AUDIO
    if (init_native_audio(TEST_WAV) != 0) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }

    // INIT FONT
    if (TTF_Init() == -1) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }
    return 0;
}

int init_window(app_t *a) {
    a->window = SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, INIT_OPEN_GL_WINDOW); // janela de renderizacao c open_gl
    if (a->window == NULL) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }
    return 0;
}

int init_renderer(app_t *a) {
    a->renderer = SDL_CreateRenderer(a->window, NULL);
    if (a->renderer == NULL) {
        printDebug(SDL_GetError(), 5000);
        SDL_DestroyWindow(a->window);
        SDL_free(a);
        SDL_Quit();
        return -1;
    }
    // para renderização transparente
    SDL_SetRenderDrawBlendMode(a->renderer, SDL_BLENDMODE_BLEND);
    return 0;
}

int init_fonts(app_t *a) {
    // Init all fonts
    a->font = TTF_OpenFont(GAME_OVER_TTF, 60);
    if (a->font == NULL) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }

    a->hint_font = TTF_OpenFont(GAME_OVER_TTF, 30);
    if (a->hint_font == NULL) {
        printDebug(SDL_GetError(), 5000);
        return -1;
    }
    return 0;
}

/*
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *  ON READY
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    app_t *a;
	if (init_SDL() != 0) return SDL_APP_FAILURE;

    // Using calloc to empty memory
	a = (app_t*)SDL_calloc(1, sizeof(app_t));
	if (a == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    // Limpa o pad previo
    if (init_window(a)) return SDL_APP_FAILURE;
    if (init_renderer(a)) return SDL_APP_FAILURE;
    if (init_fonts(a)) return SDL_APP_FAILURE;

    *appstate = (void *)a;

    getBinaryPath(argc, argv);
    SDL_zero(a->prev_pad);

    // Inicializando o controle nativo
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    a->running = 1; //indica que o app está rodando

    // defino a tela inicial do jogo
    set_screen(a, &MENU_SCREEN);

	return SDL_APP_CONTINUE;
}

/*
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *  GAME LOOP
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_t *a = (app_t *)appstate;
    a->prev_pad = a->pad;

    // Delego tarefas

    // handl events eh delegado pra tela atual
    if (a->current_screen && a->current_screen->handle_events) {
        a->current_screen->handle_events(a);
    }

    // logic screen delegado tb
    if (a->current_screen && a->current_screen->update) {
        a->current_screen->update(a);
    }

    // delego render
    if (a->current_screen && a->current_screen->render) {
        a->current_screen->render(a);
    }

    if (a->pad.Buttons & PSP_CTRL_START) return SDL_APP_SUCCESS;
    if (!a->running) return SDL_APP_SUCCESS;
    
    return SDL_APP_CONTINUE;
}

/*
::::::::::::::::::::::::::::::::::::::
LÓGICA DE INTERAÇÃO COM EVENTOS SDL
::::::::::::::::::::::::::::::::::::::
*/
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	app_t *a = (app_t *)appstate;

    if (a != NULL) {
        if (a->current_screen && a->current_screen->destroy) {
            a->current_screen->destroy(a);
        }

        TTF_CloseFont(a->font);
        TTF_CloseFont(a->hint_font);
        SDL_DestroyRenderer(a->renderer);
        SDL_DestroyWindow(a->window);
        cleanup_native_audio();
        SDL_free(a);
    }
    TTF_Quit();
	SDL_Quit();
}