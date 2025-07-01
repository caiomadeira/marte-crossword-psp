#include "logic.h"
#include<time.h>

void printGrid(Grid* grid, int size) {
    printf("=========== Grid Layout ==============\n");
    for(int i = 0; i < grid->nrow * grid->ncol; i++) {
        if (grid->list_cells[i] != NULL) {
            printf("Cell[%d]: x=%.f, y=%.f, letter=%c\n", i, grid->list_cells[i]->x, grid->list_cells[i]->y, grid->list_cells[i]->current_letter);
        }
    }
    printf("=================================\n");
}

Cell* newCell(int x, int y, int w, int h, char letter) {
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    if (cell != NULL) {
        cell->w = w;
        cell->h = h;
        cell->x = x;
        cell->y = y;
        cell->current_letter = letter;
    }
    return cell;
}

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
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (grid == NULL) return NULL;

    grid->nrow = nrow;
    grid->ncol = ncol;
    grid->ai = 0;
    grid->aj = 0;
    grid->font = NULL;
    grid->font_size = 45;

    // alloc 2D matrix of cells
    grid->list_cells = (Cell**)SDL_malloc(nrow*sizeof(Cell*));
    if (grid->list_cells == NULL) {
        SDL_free(grid);
        return NULL;
    }
    for(int i = 0; i < nrow; i++) {
        grid->list_cells[i] = (Cell*)SDL_calloc(ncol, sizeof(Cell));
        if (grid->list_cells[i] == NULL) {
            // codigo pra limpeza que nao quero escrever agora
        }
    }

    // define a gridArea (struct com os dados das dimensoes)
    grid->gridArea = gridArea;
    float cell_w = grid->gridArea->w / ncol;
    float cell_h = grid->gridArea->h / nrow;

    for(int i = 0; i < nrow; i++) {
        for(int j = 0; j < ncol; j++) {
            Cell* cell = &grid->list_cells[i][j];

            // relativo ao tamanho da celula (- padding)
            cell->w = cell_w - grid->gridArea->padding;
            cell->h = cell_h - grid->gridArea->padding;

            // relativo a posicao com base no indice e no tamanho da celula
            cell->x = grid->gridArea->x + (j * cell_w) + (grid->gridArea->padding / 2.0f);
            cell->y = grid->gridArea->y + (i * cell_h) + (grid->gridArea->padding / 2.0f);

            cell->current_letter = ' ';
        }
    }

    // grid->total_words = 5; // TODO: Pensar em algum calculo pra definir quantas palavras cabem a partir do m x n da matriz
    // grid->words = (Word**)malloc(grid->total_words*sizeof(Word*)); // calloc ja inicia toda a memoria com zeros
    // if (grid->words == NULL) return NULL;

    // // define o tamanho maximo que uma palavra pode alcançar baseado na maior dimensao da matriz
    // int max_word_len = 0;
    // if (grid->ncol > grid->nrow) {
    //     max_word_len = grid->ncol;
    // } else {
    //     max_word_len = grid->nrow;
    // }

    // for(int i = 0; i < max_word_len; i++) {
    //     grid->words[i] = NULL; // inicializo todas as palavras com null
    // }

    return grid;
}

void moveCellLetterSelection(Grid* grid) {
    if (grid == NULL) return;

    Cell* selected_cell = &grid->list_cells[grid->ai][grid->aj];
    char letter = selected_cell->current_letter;

    if (letter == 'Z') selected_cell->current_letter = 'A';
    else if (letter >= 'A' && letter < 'Z') selected_cell->current_letter++;
    else selected_cell->current_letter = 'A';
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

// static char* getLongestWordIndex(Word words[], int word_count) {

//     int maxLength = 0;
//     int maxIndex = 0;
//     int i;
//     for(i = 1; i < word_count; i++) {
//         Word* word = &words[i];
//         int length = strlen(word->word);
//         if (length > maxLength) {
//             maxLength = length;
//             maxIndex = i;
//         }
//     }
//     return words[maxIndex].word;
// }

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
        if (r >= grid->nrow || c >= grid->ncol) {
            return false;
        }

        // checa se a celll da grid nao esta vazia e se tem uma letra diferente
        char grid_char = grid->list_cells[r][c].current_letter;
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
                if (grid->list_cells[r][c].current_letter == toupper(selected_word->word[i])) {
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
                                // if (orientation == HORIZONTAL) {
                                //     grid->list_cells[start_row][start_col + w].current_letter = toupper(selected_word->word[w]);
                                // } else {
                                //     grid->list_cells[start_row + w][start_col].current_letter = toupper(selected_word->word[w]);
                                // }
                                if (orientation == HORIZONTAL) {
                                    start_col += w;
                                } else {
                                    start_row += w;
                                }

                                if (grid->list_cells[start_row][start_col].current_letter == '\0') {
                                    grid->list_cells[start_row][start_col].current_letter = ' ';
                                }
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

    for(int i = 0; i < grid->nrow; i++) {
        for (int j = 0; j < grid->ncol; j++) {
            grid->list_cells[i][j].current_letter = '\0'; // limpando a grade/iniciando as cells c caractere nulo
        }
    }

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
        //grid->list_cells[start_row][start_col + i].current_letter = toupper(first->word[i]);
        grid->list_cells[start_row][start_col + i].current_letter = ' ';
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

        if (toupper(grid->list_cells[r][c].current_letter) != toupper(word->word[i])) {
            return false;
        }
    }
    return true;
}