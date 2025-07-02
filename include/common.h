#ifndef common_h
#define common_h

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h> // Necessário para a função chdir()
#include <string.h> // Necessário para a função strrchr()
#include <ctype.h>
#include <stddef.h> // para o NULL
#include "assets.h"
// -------------------------
// PSP
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include<pspaudio.h>
#include<pspaudiolib.h>


// REDEFINING FUNCTIONS NAME
#define readButtonState sceCtrlReadBufferPositive
#define readButtonStateContinuous sceCtrlPeekBufferPositive

// COLORS ABGR
#define RAW_BLUE 0x0FFFF0000
#define RAW_RED 0xFF0000FF
#define RAW_GREEN 0x0FF00FF00
#define RAW_WHITE 0x0FFFFFFFF
#define RAW_BLACK 0xFF0000000
#define RAW_PINK 0xFFFF00FF
#define RAW_DARK_GREEN 0xFF408020
#define RAW_DARK_RED 0xFF102080
#define RAW_CLEAR_GRAY 0xFF554433

/* STRUCT PROTOTYPES */
typedef enum {
    SCREEN_MENU,
    SCREEN_NEW_GAME
} GameState;

/* CONSTANTS */
#define WINDOW_NAME "PSP GAME"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 272
#define INIT_OPEN_GL_WINDOW false

/* FUNCTIONS */
void printDebug(const char* str, int delay);
void getBinaryPath(int argc, char *argv[]);

#endif