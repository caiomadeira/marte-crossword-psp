#include "app.h"
#include "graphic.h"
#include "screen.h"
#include "player.h"

static void game_init(app_t *a);
static void game_handle_events(app_t *a);
static void game_update(app_t *a);
static void game_render(app_t *a);
static void game_destroy(app_t *a);

Screen GAME_SCREEN = {
    .init = game_init,
    .handle_events = game_handle_events,
    .update = game_update,
    .render = game_render,
    .destroy = game_destroy
};

typedef struct {
    SDL_Texture *background_texture;
    Player *player;
    Grid *grid;
    Word* selected_word;
    WordOrientation active_orientation;
    SelectionMode selection_mode;
    Uint32 start_time;
    const char *current_hint;
    GameAssets assets;
} GameData;

static void updateCurrentHint(GameData* data) {
    if (!data || !data->grid) return;

    int r = data->grid->ai;
    int c = data->grid->aj;

    if (data->grid->list_cells[r][c].solution_letter == '\0') {
        data->current_hint = NULL;
        return;
    }

    Word* preferred_word = findWordAt(r, c, words, words_count, data->active_orientation);
    Word* alternative_word = findWordAt(r, c, words, words_count, (data->active_orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL);
    if (preferred_word) {
        data->current_hint = preferred_word->hint;
    } else if (alternative_word) {
        data->current_hint = alternative_word->hint;
    } else {
        data->current_hint = NULL;
    }
}

static void navigateInWordMode(GameData* data, int d_row, int d_col) {
    if (!data || !data->selected_word) return;

    // Tenta primeiro trocar para uma palavra que cruza na posição atual
    if ((d_row != 0 && data->selected_word->orientation == HORIZONTAL) || 
        (d_col != 0 && data->selected_word->orientation == VERTICAL)) {
        
        WordOrientation target_orientation = (data->selected_word->orientation == HORIZONTAL) ? VERTICAL : HORIZONTAL;
        Word* intersecting_word = findWordAt(data->grid->ai, data->grid->aj, words, words_count, target_orientation);

        if (intersecting_word) {
            data->selected_word = intersecting_word;
            data->active_orientation = intersecting_word->orientation;
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
        if (words[i].is_placed && words[i].orientation == data->selected_word->orientation) {
            if (&words[i] == data->selected_word) {
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
    data->selected_word = potential_next_words[next_index];
    data->grid->ai = data->selected_word->pos_final_i;
    data->grid->aj = data->selected_word->pos_final_j;
    data->active_orientation = data->selected_word->orientation;
}

static game_init(app_t *a) {
    GameData *data = (GameData*)malloc(sizeof(GameData));
    if (!data) {
        app->running 0;
        return;
    }

    SDL_Surface *cross_btn_surface = initImage(CROSS_BUTTON_PNG);
    a->assets.cross_btn_texture = createImageTexture(cross_btn_surface, a->renderer);
    if (!a->assets.cross_btn_texture) return SDL_APP_FAILURE;

    SDL_Surface *square_btn_surface = initImage(SQUARE_BUTTON_PNG);
    a->assets.square_btn_texture = createImageTexture(square_btn_surface, a->renderer);
    if (!a->assets.square_btn_texture) return SDL_APP_FAILURE;

    SDL_Surface *triangle_btn_surface = initImage(TRIANGLE_BUTTON_PNG);
    a->assets.triangle_btn_texture = createImageTexture(triangle_btn_surface, a->renderer);
    if (!a->assets.triangle_btn_texture) return SDL_APP_FAILURE;

    SDL_Surface *circle_btn_surface = initImage(CIRCLE_BUTTON_PNG);
    a->assets.circle_btn_texture = createImageTexture(circle_btn_surface, a->renderer);
    if (!a->assets.circle_btn_texture) return SDL_APP_FAILURE;

    a->player = init_player();
    if (a->player == NULL) {
        printDebug(SDL_GetError(), 5000);
        return SDL_APP_FAILURE;
    }

    int result = sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, data->player->name, 128);
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
    data->grid = newGrid(10, 10, gridArea);
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
}

static void game_handle_events(app_t *a) {
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

}

static void game_update(app_t *a) {
    // --- ATUALIZAÇÃO DE ESTADO ---
    updateCurrentHint(a);

}

static void game_render(app_t *a) {
    GameData *data = (GameData*)a->screen_data;
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
}

static void game_destroy(app_t *app) {
    GameData *data = (GameData*)app->screen_data;
    if (data) {
        SDL_DestroyTexture(data->background_texture);
        free(data->player);
        free(data->grid)
        free(data);
    }
    app->screen_data = NULL;
}