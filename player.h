#if !defined(_PLAYER_H)
#define _PLAYER_H

#include <sys/types.h>
#include <unistd.h>

#define SHMKEY 1000
#define SHMSIZE 4096

#define SEQUENCE 1
#define CIRCLE 2

typedef struct Shm
{
    char cur_music[128];
    int cur_mode;
    pid_t child_pid;
    pid_t grand_pid;
} Shm;

int init_shm();

#endif // _PLAYER_H
