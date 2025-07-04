#ifndef SCREEN_H
#define SCREEN_H


struct app_t; // forward declaraction da struct principal do app SDL pra evitar dependencia circular

// aqui caso  importante: defino uma tela atraves de ponteiros para funções
typedef struct Screen {
    void (*init)(struct app_t *app); 
    void (*handle_events)(struct app_t *app);
    void (*update)(struct app_t *app);
    void (*render)(struct app_t *app);
    void (*destroy)(struct app_t *app);
} Screen;

extern Screen ENTRY_SCREEN;
extern Screen MENU_SCREEN;
extern Screen GAME_SCREEN;

#endif