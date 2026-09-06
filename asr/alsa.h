#if !defined(_ALSA_H)
#define _ALSA_H

#include <alsa/asoundlib.h>

#define RECORD_DEVICE "hw:2,0"
#define PLAYBACK_DEVICE "default"
#define RATE 44100
#define CHANNELS 1

int init_alsa();
void resample_linear(const int16_t *input, size_t input_len, int16_t *output, size_t output_len);

#endif // _ALSA_H
