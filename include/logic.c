#include "logic.h"
#include<time.h>

GridArea* newGridArea(int x, int y, int w, int h, int padding) {
    GridArea* gridArea = (GridArea*)malloc(sizeof(GridArea));
    if (gridArea == NULL) return NULL;
    gridArea->x = x;
    gridArea->y = y;
    gridArea->w = w;
    gridArea->h = h;
    gridArea->padding = padding;
    return gridArea;
}

// d_row = pode ser -1 (cima), 1 (baixo) ou 0 eh o deslocamento
// d_col pode ser -1 (esqueda), 1 (direita) ou 0
// O d de d_row e d_col eh relativo ao D-PAD do PSP
void moveGridSelection(Grid* grid, Word* active_word, int d_row, int d_col) {
    if (grid == NULL || active_word == NULL) return;

    int new_row = grid->ai;
    int new_col = grid->aj;
    int len = strlen(active_word->word);
    // aqui eu sou verifico os limites pra 1. linha e 2. coluna
    // apenas movimenta na direcao da orientacao da palavra
    if (active_word->orientation == HORIZONTAL && d_col != 0) {
        new_col += d_col;
        if (new_col >= active_word->pos_final_j && new_col < active_word->pos_final_j + len) {
            grid->aj = new_col;
        }
    } else if (active_word->orientation == VERTICAL && d_row != 0) {
        new_row += d_row;
        if (new_row >= active_word->pos_final_i && new_row < active_word->pos_final_i + len) {
            grid->ai = new_row;
        }
    }
}

Grid* newGrid(int nrow, int ncol, GridArea* gridArea) {
    // 1. Aloca a estrutura principal
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (grid == NULL) return NULL;

    // 2. Inicializa todos os ponteiros como NULL para segurança
    grid->gridArea = NULL;
    grid->list_cells = NULL;
    grid->font = NULL;

    // 3. Atribui os valores
    grid->nrow = nrow;
    grid->ncol = ncol;
    grid->ai = 0;
    grid->aj = 0;
    grid->font_size = 45;
    grid->gridArea = gridArea;

    // 4. Aloca o array de ponteiros para as linhas
    grid->list_cells = (Cell**)malloc(nrow * sizeof(Cell*));
    if (grid->list_cells == NULL) {
        free(grid->gridArea); // Libera a área que já foi associada
        free(grid);           // Libera a struct
        return NULL;
    }

    // 5. Aloca cada linha, com limpeza completa em caso de falha
    for (int i = 0; i < nrow; i++) {
        grid->list_cells[i] = (Cell*)calloc(ncol, sizeof(Cell));
        if (grid->list_cells[i] == NULL) {
            // LIMPEZA CRÍTICA: libera tudo que foi alocado até agora
            for (int j = 0; j < i; j++) {
                free(grid->list_cells[j]); // Libera as linhas bem-sucedidas
            }
            free(grid->list_cells);     // Libera o array de ponteiros
            free(grid->gridArea);       // Libera a área
            free(grid);                 // Libera a struct principal
            return NULL; // Retorna com segurança
        }
    }

    // 6. Inicializa as células
    float cell_w = (float)grid->gridArea->w / ncol;
    float cell_h = (float)grid->gridArea->h / nrow;
    for (int i = 0; i < nrow; i++) {
        for (int j = 0; j < ncol; j++) {
            Cell* cell = &grid->list_cells[i][j];
            cell->w = cell_w - grid->gridArea->padding;
            cell->h = cell_h - grid->gridArea->padding;
            cell->x = grid->gridArea->x + (j * cell_w) + (grid->gridArea->padding / 2.0f);
            cell->y = grid->gridArea->y + (i * cell_h) + (grid->gridArea->padding / 2.0f);
            cell->solution_letter = '\0';
            cell->player_letter = ' ';
        }
    }

    return grid;
}

void moveCellLetterSelection(Grid* grid, const char* direction) {
    if (grid == NULL) return;

    Cell* selected_cell = &grid->list_cells[grid->ai][grid->aj];
    char letter = selected_cell->player_letter;

    // caso: celula vazia
    if (letter < 'A' || letter > 'Z') { 
        selected_cell->player_letter = 'A';
    }
    if (strcmp(direction, "right") == 0) {
        if (letter == 'Z') {
            selected_cell->player_letter = 'A';
        } else {
            selected_cell->player_letter++;
        }
    } else if (strcmp(direction, "left") == 0) {
        if (letter == 'A') {
            selected_cell->player_letter = 'Z';
        } else {
            selected_cell->player_letter--;
        }
    }
}

void print1d(char *v, int size) {
    for(int i = 0; i < size; i++) printf("%c, ", v[i]);
    printf("\n");
}

char* init_letters(int start, int end) {
    int i = 0;
    size_t size = (end - start) + 1;
    printf("Letters array size: %d\n", (int)size);
    char* letters = (char*)malloc((int)size*sizeof(char));
    if (letters != NULL) {
        while(i < size) {
            *(letters + i) = start;
            start++;
            i++;
        }
        print1d(letters, size);
        return letters;
    }
    return NULL;
}

bool canPlaceWordAt(Grid* grid, const char* word, int row, int col, WordOrientation orientation) {
    int len = strlen(word);
    for(int i = 0; i < len; i++) {
        int r = row;
        int c = col;

        if (orientation == HORIZONTAL) {
            c += i;
        } else {
            r += i; // para casos verticais
        }

        // checa se saiu dos limites da grade
        if (r >= grid->nrow || c >= grid->ncol) return false;

        // checa se a celll da grid nao esta vazia e se tem uma letra diferente
        char grid_char = grid->list_cells[r][c].solution_letter; // checa contra a letra da solucao. opera no gabarito
        if (grid_char != '\0' && grid_char != toupper(word[i])) {
            return false; // letra existe
        }
    }
    return true;
}

/*
essa função tenta encontrar um local valido pra colocar a palavra, ou seja, um ponto de intesecção
*/
bool placeWord(Grid* grid, Word* selected_word) {
    int len = strlen(selected_word->word);

    WordOrientation orientations[] = {HORIZONTAL, VERTICAL}; // aqui tento orientações diferentes
    if (rand() % 2 == 1) {
        orientations[0] = VERTICAL;
        orientations[1] = HORIZONTAL;
    }

    // aqui tento encaixar a word em qlqr letra correspondente que JA ESTA na grid
    for(int i = 0; i < len; i++) {
        for(int r = 0; r < grid->nrow; r++) {
            for(int c = 0; c < grid->ncol; c++) {
                if (grid->list_cells[r][c].solution_letter == toupper(selected_word->word[i])) {
                    // tentando colocar nas duas orientações
                    for(int k = 0; k < 2; k++) {
                        WordOrientation orientation = orientations[k];
                        int start_row = r, start_col = c;
                        // calculando a posicao de alinhamento
                        if (orientation == HORIZONTAL) start_col -= i;
                        else start_row -= i;

                        // checa se pode por a palavra
                        if (canPlaceWordAt(grid, selected_word->word, start_row, start_col, orientation)) {
                            // sendo possivel coloca
                            for (int w = 0; w < len; w++) {
                                // calcula a posição correta sem alterar as variáveis originais
                                int place_r = start_row;
                                int place_c = start_col;
                                if (orientation == HORIZONTAL) {
                                    place_c += w;
                                } else {
                                    place_r += w;
                                }
                                // Escreve na posição correta e segura
                                grid->list_cells[place_r][place_c].solution_letter = toupper(selected_word->word[w]);
                            }
                            // Salva o estado da palavra
                            selected_word->is_placed = true;
                            selected_word->pos_final_i = start_row;
                            selected_word->pos_final_j = start_col;
                            selected_word->orientation = orientation;

                            return true; // confirma que a palavra foi posta
                        }

                    }
                }
            }
        }
    }
    return false; // nenhuma palavra foi encaixada
}

/*
Essa funcao orquestra
*/
void populateGridWithWords(Grid* grid, Word words[], int word_count) {
    if (grid == NULL || words == NULL || word_count == 0) return ;

    srand(time(NULL)); // a semente chamo apenas uma vez como de costume

    // coloco a primeira palavra como ancora no centro da grade
    Word* first = &words[0];
    int len = strlen(first->word);
    int start_row = (grid->nrow / 2);
    int start_col = (grid->ncol - len) / 2; // centralizando horizontalmente

    first->is_placed = true;
    first->pos_final_i = start_row;
    first->pos_final_j = start_col;
    first->orientation = HORIZONTAL;

    for(int i = 0; i < len; i++) {
        grid->list_cells[start_row][start_col + i].solution_letter = toupper(first->word[i]);
    }

    for (int i = 1; i < word_count; i++) {
        placeWord(grid, &words[i]);
    }

}

bool checkWordCompletion(Grid* grid, Word* word) {
    if (!grid || !word || !word->is_placed) return false;

    int len = strlen(word->word);
    for(int i = 0; i < len; i++) {
        int r = word->pos_final_i;
        int c = word->pos_final_j;

        if (word->orientation == HORIZONTAL)
            c += i;
        else
            r += i;

        if (toupper(grid->list_cells[r][c].player_letter) != toupper(grid->list_cells[r][c].solution_letter)) {
            return false;
        }
    }
    return true;
}

// game.c

void destroyGrid(Grid* grid) {
    if (grid == NULL) return;

    // Libera as texturas de letras cacheadas
    for (int i = 0; i < 26; i++) {
        if (grid->letter_textures_cache[i]) {
            SDL_DestroyTexture(grid->letter_textures_cache[i]);
        }
    }

    // Libera a fonte do grid
    if (grid->font) {
        TTF_CloseFont(grid->font);
    }

    // Libera a matriz de células de forma segura
    if (grid->list_cells) {
        // Libera cada linha primeiro
        for (int i = 0; i < grid->nrow; i++) {
            if (grid->list_cells[i]) {
                free(grid->list_cells[i]);
            }
        }
        // Depois libera o array de ponteiros
        free(grid->list_cells);
    }

    // Libera a área do grid
    if (grid->gridArea) {
        free(grid->gridArea);
    }

    // Finalmente, libera a própria estrutura do grid
    free(grid);
}