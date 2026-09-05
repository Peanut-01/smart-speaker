#if !defined(_ALSA_H)
#define _ALSA_H

#include <alsa/asoundlib.h>

#define RECORD_DEVICE "hw:2,0"
#define PLAYBACK_DEVICE "default"
#define RATE 44100
#define CHANNELS 1
#define PERIOD_BUFFER 1024

int init_alsa();

#endif // _ALSA_H
