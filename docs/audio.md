### Audio.c

By Caio Madeira  

The code it self implements a simple audio engine for PSP using ```pspaudiolib```. The approach is "callback based".

#### What is a callback?  
Em vez de você "forçar" o som para o hardware, você __registra__ a função.
O próprio hardware do psp, usando a pspaudiolib, chama (calls back) a função ```audio_callback``` automaticamente sempre que precisa de mais dados de áudio pra tocar. Isso acontece em intervalos regulares e __numa thread separada__, garantindo uma reprodução continua sem interrupções no jogo.  

### TROUBLESHOOTING

### Audio tocando mais lento e mais grave (uma oitava abaixo):
O possível motivo disso é uma discordância entre a taxa de amostragem (Sample Rate) do meu arquivo .WAV (usei o test.wav) e taxa de amostragem que a bibliotea ```pspaudiolib``` opera.

```
fseek(file, 44, SEEK_SET);
```
Esse trecho de código pula todo o cabeçalho do arquivo. Ao fazer isso. eu assumo que todo o arquivo WAV tem um cabeçalho fixo de __44 bytes__ e acabo ignorando as informações contidas nele, tais como:  
    - Taxa de Amostragem (Sample Rate): Em hertz (hz), diz quantas amostras de audio são tocadas por SEGUNDO. (valores comuns: 22050hz, 44100hz, 48000hz).
    - Número de canais (Num channels): Diz se o som é ***Mono*** (1 canal) ou ***Stereo*** (2 canais).
    - Bits por amostra (Bits per sample): Indica a qualidade da amostra (ex: 8-bit ou 16-bit).

PS: O PSP possivelmente tem uma taxa de amostragem máxima de 44100 Hz e 16-bit.