#include "include/common.h"
#include "include/player.h"
#include "include/audio.h"
#include "include/graphic.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

// COLORS SDL
SDL_Color SDL_WHITE = { 255, 255, 255, 255 };
SDL_Color SDL_PURPLE = { 192, 92, 255, 255 };

typedef struct app {
	SDL_Window *window;
	SDL_GLContext context;
    GameState currentstate;
    SceCtrlData pad;
    Player *player;
    // TODO: Dar um jeito de tirar essas texturas daqui
    SDL_Texture *background_texture;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *hint_font;
    SceCtrlData prev_pad;
    Grid* grid;
    const char* current_hint;
    WordOrientation active_orientation;
    SelectionMode selection_mode;
    Word* selected_word;
    Uint32 start_time;
    GameAssets assets;
} app_t;

// inicializa com as palavras
/*
    Preciso usar delcarações externas pra acessar o banco.
    TODO: INvestigar o porque
*/

// TODO: Separate score, time, etc in other functions draws
void navigateInWordMode(app_t* a, int d_row, int d_col);

void updateCurrentHint(app_t* a) {
    if (!a || !a->grid) return;

    int r = a->grid->ai;
    int c = a->grid->aj;

    if (a->grid->list_cells[r][c].solution_letter == '\0') {
        a->current_hint = NULL;
        return;
    }

    Word* preferred_word = findWordAt(r, c, words, words_count, a->active_orientation);
    Word* alternative_word = findWordAt(r, c, words, words_count, (a->active_orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL);
    if (preferred_word) {
        a->current_hint = preferred_word->hint;
    } else if (alternative_word) {
        a->current_hint = alternative_word->hint;
    } else {
        a->current_hint = NULL;
    }
}

/*
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *  ON READY
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	app_t *a;
    getBinaryPath(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    if (init_native_audio(TEST_WAV) != 0) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }
    
    if (TTF_Init() == -1) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

	a = (app_t*)SDL_calloc(1, sizeof(app_t)); // usando calloc p/ zerar a memoria
	if (a == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    SDL_zero(a->prev_pad);

    a->window = SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, INIT_OPEN_GL_WINDOW); // janela de renderizacao c open_gl
    if (a->window == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    a->renderer = SDL_CreateRenderer(a->window, NULL);
    if (a->renderer == NULL) {
        printDebug(SDL_GetError(), 5000);
        SDL_DestroyWindow(a->window);
        SDL_free(a);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    // para renderização transparente
    SDL_SetRenderDrawBlendMode(a->renderer, SDL_BLENDMODE_BLEND);

    a->font = TTF_OpenFont(GAME_OVER_TTF, 60);
    if (a->font == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    a->hint_font = TTF_OpenFont(GAME_OVER_TTF, 30);
    if (a->hint_font == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    // Inicializando o controle nativo
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    SDL_Surface *bg_surface = IMG_Load(BACKGROUND_PNG);
    if (!bg_surface) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    // aqui importante: converte a superficie numa texture otimizada pra a GPU e guarda em um buffer
    a->background_texture = SDL_CreateTextureFromSurface(a->renderer, bg_surface);
    SDL_DestroySurface(bg_surface); // libera a superficie da mem. ja que n precisamos mais
    if (!a->background_texture) {
        printDebug(SDL_GetError(), 5000);
        SDL_DestroyRenderer(a->renderer);
        SDL_DestroyWindow(a->window);
        SDL_free(a);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    SDL_Surface *cross_btn_surface = initImage(CROSS_BUTTON_PNG);
    a->assets.cross_btn_texture = createImageTexture(cross_btn_surface, a->renderer);
    if (!a->assets.cross_btn_texture) {  }// ... colocar free's }

    SDL_Surface *square_btn_surface = initImage(SQUARE_BUTTON_PNG);
    a->assets.square_btn_texture = createImageTexture(square_btn_surface, a->renderer);
    if (!a->assets.square_btn_texture) {  }

    SDL_Surface *triangle_btn_surface = initImage(TRIANGLE_BUTTON_PNG);
    a->assets.triangle_btn_texture = createImageTexture(triangle_btn_surface, a->renderer);
    if (!a->assets.triangle_btn_texture) {  }

    SDL_Surface *circle_btn_surface = initImage(CIRCLE_BUTTON_PNG);
    a->assets.circle_btn_texture = createImageTexture(circle_btn_surface, a->renderer);
    if (!a->assets.circle_btn_texture) {  }

    a->player = init_player();
    if (a->player == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    int result = sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, a->player->name, 128);
    if (result != 0) {
        strcpy(a->player->name, "Player");
    }

    #define CENTRALIZED false
    int grid_width, grid_height, grid_pos_x, grid_pos_y, padding = 0;
    if (CENTRALIZED) {
        grid_width = WINDOW_WIDTH / 2;
        grid_height = WINDOW_HEIGHT - 15;
        grid_pos_x = (WINDOW_WIDTH - grid_width) / 2;
        grid_pos_y = (WINDOW_HEIGHT - grid_height) / 2;
        padding = 2;
    } else {
        grid_width = WINDOW_WIDTH / 2;
        grid_height = WINDOW_HEIGHT - 15;
        grid_pos_x = 10;
        grid_pos_y = 10;
        padding = 2;
    }
    GridArea* gridArea = newGridArea(grid_pos_x, grid_pos_y, grid_width, grid_height, padding);
    a->grid = newGrid(10, 10, gridArea);
    if (a->grid == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    populateGridWithWords(a->grid, words, words_count);
    // Iniciando o modo
    a->selection_mode = WORD_MODE;
    a->selected_word = &words[0];
    a->active_orientation = HORIZONTAL;

    // coloca a posicao inicial do cursor para o inicio da primeira palavra
    if (a->selected_word && a->selected_word->is_placed) {
        a->grid->ai = a->selected_word->pos_final_i;
        a->grid->aj = a->selected_word->pos_final_j;
    }

    a->grid->font = TTF_OpenFont(GAME_OVER_TTF, a->grid->font_size);
    if (a->grid->font == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    /* LETTERS PRE-RENDERIZATION */
    SDL_Color SDL_BLACK = { 0, 0, 0, 255 };
    for(int i = 0; i < 26; i++) {
        char letter_str[2] = { (char)('A' + i), '\0' };
        SDL_Surface* surface = TTF_RenderText_Blended(a->grid->font, letter_str, strlen(letter_str), SDL_BLACK);
        if (surface) {
            a->grid->letter_textures_cache[i] = SDL_CreateTextureFromSurface(a->renderer, surface);
            SDL_DestroySurface(surface);
        } else {
            a->grid->letter_textures_cache[i] = NULL;
        }
    }
    updateCurrentHint(a); // evita o bug da hint nao aparecer ao iniciar
    // INIT TIMER
    a->start_time = SDL_GetTicks(); // NOTA: SDL_GetTicks() retorna o num de milissegundos desde que a biblioteca SDL foi inicializada.

	*appstate = (void *)a;
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

    // --- LÓGICA DE CONTROLES ---
    readButtonState(&a->pad, 1);
    if (a->pad.Buttons & PSP_CTRL_START) return SDL_APP_SUCCESS;

    if (a->selection_mode == WORD_MODE) {
        // --- CONTROLES DO MODO PALAVRA ---
        int d_row = 0, d_col = 0;
        if ((a->pad.Buttons & PSP_CTRL_UP) && !(a->prev_pad.Buttons & PSP_CTRL_UP))  d_row = -1;
        if ((a->pad.Buttons & PSP_CTRL_DOWN) && !(a->prev_pad.Buttons & PSP_CTRL_DOWN)) d_row = 1;
        if ((a->pad.Buttons & PSP_CTRL_LEFT) && !(a->prev_pad.Buttons & PSP_CTRL_LEFT)) d_col = -1;
        if ((a->pad.Buttons & PSP_CTRL_RIGHT) && !(a->prev_pad.Buttons & PSP_CTRL_RIGHT)) d_col = 1;
        
        if (d_row != 0 || d_col != 0) {
            navigateInWordMode(a, d_row, d_col); // <- CHAMADA DA NOVA FUNÇÃO!
        }

        // Entra no Modo Letra
        if ((a->pad.Buttons & PSP_CTRL_CROSS) && !(a->prev_pad.Buttons & PSP_CTRL_CROSS)) {
            a->selection_mode = LETTER_MODE;
            if (a->selected_word) a->active_orientation = a->selected_word->orientation;
        }

    } else { // LETTER_MODE
        // --- CONTROLES DO MODO LETRA ---
        int d_row = 0, d_col = 0;
        if ((a->pad.Buttons & PSP_CTRL_UP) && !(a->prev_pad.Buttons & PSP_CTRL_UP)) d_row = -1;
        if ((a->pad.Buttons & PSP_CTRL_DOWN) && !(a->prev_pad.Buttons & PSP_CTRL_DOWN)) d_row = 1;
        if ((a->pad.Buttons & PSP_CTRL_RIGHT) && !(a->prev_pad.Buttons & PSP_CTRL_RIGHT)) d_col = 1;
        if ((a->pad.Buttons & PSP_CTRL_LEFT) && !(a->prev_pad.Buttons & PSP_CTRL_LEFT)) d_col = -1;
    
        moveGridSelection(a->grid, a->selected_word, d_row, d_col);
        bool letter_was_changed = false;        
            
        Word* word_to_check = findWordAt(a->grid->ai, a->grid->aj, words, words_count, a->active_orientation);
        
        // apenas permite editar se a palavra nao estiver resolvida
        if (word_to_check && !word_to_check->is_solved) {
            if ((a->pad.Buttons & PSP_CTRL_CROSS) && !(a->prev_pad.Buttons & PSP_CTRL_CROSS)) {
                moveCellLetterSelection(a->grid, "right");
                letter_was_changed = true;
            } else if ((a->pad.Buttons & PSP_CTRL_SQUARE) && !(a->prev_pad.Buttons & PSP_CTRL_SQUARE)) {
                moveCellLetterSelection(a->grid, "left");
                letter_was_changed = true;
            }
        }
        if (letter_was_changed) {
            // verifica se a palavra foi completada
            if (checkWordCompletion(a->grid, word_to_check)) {
                word_to_check->is_solved = true;
                a->player->score += 100;
                trigger_native_sound(); // TODO: Change this trigger audio logic
            }
            
            // checa tb na outra orientação caso tenha uma interseção
            Word* other_word = findWordAt(a->grid->ai, a->grid->aj, words, words_count, (a->active_orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL);
            if (other_word && !other_word->is_solved && checkWordCompletion(a->grid, other_word)) {
                other_word->is_solved = true;
                a->player->score += 100;
                trigger_native_sound();
            }
        }

        // Troca a orientação de escrita na interseção (botão círculo)
        if ((a->pad.Buttons & PSP_CTRL_CIRCLE) && !(a->prev_pad.Buttons & PSP_CTRL_CIRCLE)) {
            a->active_orientation = (a->active_orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL;
            // Troca a palavra selecionada se houver uma na nova orientação
            Word* other_word = findWordAt(a->grid->ai, a->grid->aj, words, words_count, a->active_orientation);
            if (other_word) a->selected_word = other_word;
        }

        // Volta para o Modo Palavra (botão triângulo)
        if ((a->pad.Buttons & PSP_CTRL_TRIANGLE) && !(a->prev_pad.Buttons & PSP_CTRL_TRIANGLE)) {
            a->selection_mode = WORD_MODE;
        }
    }

    // --- ATUALIZAÇÃO DE ESTADO ---
    updateCurrentHint(a);
    // ----------------------------------------------------
    // --- LÓGICA DE RENDERIZAÇÃO ---

    SDL_SetRenderDrawColor(a->renderer, 0, 0, 0, 255);
    SDL_RenderClear(a->renderer);

    SDL_RenderTexture(a->renderer, a->background_texture, NULL, NULL);

    // Draw Grid
    drawGrid(a->grid, a->renderer, a->selection_mode, a->selected_word, words, words_count);
    // Draw Hint
    float x = (WINDOW_WIDTH / 2) + 20;
    float y = 10;
    float rectW = (WINDOW_WIDTH / 2) - 30;
    float rectH = (WINDOW_HEIGHT / 2 ) - 15;
    if (a->current_hint) drawHint(a->current_hint, x, y, rectW, rectH, a->hint_font, a->renderer);

    y = WINDOW_HEIGHT / 2;
    drawInfoBox(x, y, rectW, rectH, a->hint_font, a->renderer);
    drawScore(a->player->score, a->hint_font, a->renderer);
    drawTime(a->start_time, a->hint_font, a->renderer);
    drawInfoStr(a->player->name, a->hint_font, a->renderer);

    if (a->pad.Buttons & PSP_CTRL_LTRIGGER) {
        drawInstructionBox(a->hint_font, a->renderer, &a->assets);
    }

    SDL_RenderPresent(a->renderer); // mostra na tela tudo o que foi desenhado    
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
    // free letter texture cache
    for(int i = 0; i < 26; i++) {
        if (a->grid->letter_textures_cache[i] != NULL) {
            SDL_DestroyTexture(a->grid->letter_textures_cache[i]);
        }
    }

    TTF_CloseFont(a->font);
    TTF_CloseFont(a->hint_font);

	SDL_DestroyWindow(a->window);
    cleanup_native_audio();
	SDL_free(a);
	SDL_Quit();
}

void navigateInWordMode(app_t* a, int d_row, int d_col) {
    if (!a || !a->selected_word) return;

    // Tenta primeiro trocar para uma palavra que cruza na posição atual
    if ((d_row != 0 && a->selected_word->orientation == HORIZONTAL) || 
        (d_col != 0 && a->selected_word->orientation == VERTICAL)) {
        
        WordOrientation target_orientation = (a->selected_word->orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL;
        Word* intersecting_word = findWordAt(a->grid->ai, a->grid->aj, words, words_count, target_orientation);

        if (intersecting_word) {
            a->selected_word = intersecting_word;
            a->active_orientation = intersecting_word->orientation;
            // O cursor (ai, aj) já está na interseção, então não precisa mover.
            return;
        }
    }

    // Se não houver cruzamento, pula para a próxima palavra na mesma orientação
    Word* potential_next_words[words_count];
    int count = 0;
    int current_word_index = -1;

    // 1. Encontra todas as palavras com a mesma orientação da palavra atual
    for (int i = 0; i < words_count; i++) {
        if (words[i].is_placed && words[i].orientation == a->selected_word->orientation) {
            if (&words[i] == a->selected_word) {
                current_word_index = count;
            }
            potential_next_words[count] = &words[i];
            count++;
        }
    }

    if (count <= 1) return; // Nenhuma outra palavra para pular

    // 2. Calcula o novo índice
    int next_index = current_word_index;
    if (d_row > 0 || d_col > 0) { // DOWN ou RIGHT
        next_index = (current_word_index + 1) % count;
    } else if (d_row < 0 || d_col < 0) { // UP ou LEFT
        next_index = (current_word_index - 1 + count) % count;
    }

    // 3. Atualiza o estado do jogo com a nova palavra selecionada
    a->selected_word = potential_next_words[next_index];
    a->grid->ai = a->selected_word->pos_final_i;
    a->grid->aj = a->selected_word->pos_final_j;
    a->active_orientation = a->selected_word->orientation;
}