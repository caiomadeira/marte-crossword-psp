#include "words.h"

Word words[] = {
    {0, "Some eggs are made of.", "chocolate", .is_solved = false },
    {1, "Can be used to talk.", "telephone", .is_solved = false },
    {2, "One man walked above.", "water", .is_solved = false },
    {3, "A star.", "sun", .is_solved = false },
    {4, "Has wheels and can be fast.", "car", .is_solved = false },
    {5, "Can be a programming language or a natural satelite.", "lua", .is_solved = false },
    {6, "... sunshine of spoteless mind.", "eternal", .is_solved = false },
    {7, "Put your head on", "pillow", .is_solved = false }
};

int words_count = sizeof(words) / sizeof(words[0]);

Word* newWord(int index, char* hint, char* word) {
    Word* w = (Word*)malloc(sizeof(Word));
    if (w == NULL) {
        return NULL;
    }

    w->hint_size = strlen(hint);
    w->hint = (char*)malloc(w->hint_size * 1);
    if (w->hint == NULL) {
        free(w);
        return NULL;
    }
    
    w->word_size = strlen(word);
    if (w->word == NULL) {
        free(w->hint);
        free(w);
        return NULL;
    }
    strcpy(w->hint, hint);
    strcpy(w->word, word);
    w->orientation = 0;
    w->index = index;
    
    return w;
}

void destroyWord(Word* w) {
    if (w == NULL) return;
    free(w->hint);
    free(w->word);
    free(w);
}

// encontra a word que contem a celula c/ uma orientação específica e  retorna um ponteiro p/ word
Word* findWordAt(int r, int c, Word words[], int word_count, WordOrientation orientation) {
    for(int i = 0; i < word_count; i++) {
        Word* current_word = &words[i];

        // checa se a word foi colocada na grade e tem orientação que queremos
        if (current_word->is_placed && current_word->orientation == orientation) {
            int len = strlen(current_word->word);
            if (orientation == HORIZONTAL) {
                // aqui checo se a cell (r,c)) ta dentro dos limites dessa palavra
                if (r == current_word->pos_final_i && c >= current_word->pos_final_j && c < current_word->pos_final_j + len) {
                    return current_word; // achou a palavra
                }
            } else { // orientação vertical
                if (c == current_word->pos_final_j && r >= current_word->pos_final_i && r < current_word->pos_final_i + len) {
                    return current_word;
                }
            }
        }
    }
    return NULL; // nenhuma palavra encontrada nessa posição/orientação
}