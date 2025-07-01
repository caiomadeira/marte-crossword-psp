#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspaudiolib.h>

static struct {
    short *buffer;          
    unsigned int size_bytes;  
    unsigned int position;     
    int is_playing;        
} g_sound;


void audio_callback(void *buf, unsigned int length, void *userdata)
{

    if (!g_sound.is_playing) {
        memset(buf, 0, length * 2 * sizeof(short));
        return;
    }

    short *output_buffer = (short *)buf;
    int i;

    for (i = 0; i < length; i++) {
        if (g_sound.position < g_sound.size_bytes) {
            output_buffer[2 * i]     = g_sound.buffer[g_sound.position / 2];     // Canal Esquerdo
            output_buffer[2 * i + 1] = g_sound.buffer[g_sound.position / 2 + 1]; // Canal Direito
            g_sound.position += 4; 
        } else {
            output_buffer[2 * i]     = 0;
            output_buffer[2 * i + 1] = 0;
        }
    }

    if (g_sound.position >= g_sound.size_bytes) {
        g_sound.is_playing = 0;
    }
}

int init_native_audio(const char *filename) {
    memset(&g_sound, 0, sizeof(g_sound));

    FILE *file = fopen(filename, "rb");
    if (!file) { return -1; }

    fseek(file, 44, SEEK_SET); // pula o cabeçalho do WAV

    long start_pos = ftell(file);
    fseek(file, 0, SEEK_END);
    g_sound.size_bytes = ftell(file) - start_pos;
    fseek(file, start_pos, SEEK_SET);

    g_sound.buffer = (short*)malloc(g_sound.size_bytes);
    if (!g_sound.buffer) {
        fclose(file);
        return -1;
    }

    fread(g_sound.buffer, 1, g_sound.size_bytes, file);
    fclose(file);

    // inicializa a pspaudiolib e regista a callback.
    pspAudioInit();
    // o canal 0 eh gerido pela função `audio_callback`.
    pspAudioSetChannelCallback(0, audio_callback, NULL);

    return 0;
}

void trigger_native_sound(void) {
    g_sound.position = 0;
    g_sound.is_playing = 1;
}

void cleanup_native_audio(void) {
    if (g_sound.buffer) {
        free(g_sound.buffer);
    }
    pspAudioEnd();
}