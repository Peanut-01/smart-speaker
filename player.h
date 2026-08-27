#if !defined(_PLAYER_H)
#define _PLAYER_H

#include <sys/types.h>
#include <unistd.h>

#define SHMKEY 1000
#define SHMSIZE 4096

#define SEQUENCE 1
#define CIRCLE 2

#define ONLINE_MODE 1
#define OFFLINE_MODE 2

#define ONLINE_URL "http://180.76.142.171/music/"

typedef struct Shm
{
    char cur_music[128];
    char cur_singer[128];
    int cur_mode;
    pid_t parent_pid;
    pid_t child_pid;
    pid_t grand_pid;
} Shm;

int init_shm();
void player_start_play();
void player_play_music(char *name) ;
void child_process(char *name);
void parent_get_shm(Shm *s);
void child_quit_process(int sig);
void player_stop_play();
void player_continue_play();
void player_suspend_play();
void player_next_play();
void player_prior_play();

#endif // _PLAYER_H
