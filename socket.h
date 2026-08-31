#if !defined(_SOCKET_H)
#define _SOCKET_H

#define PORT 8008
#define IP "127.0.0.1"

int init_socket();
void socket_recv_data(char *msg);
void socket_get_music(const char *singer);
void socket_start_play();
void socket_stop_play();
void socket_suspend_play();
void socket_continue_play();
void socket_next_play();
void socket_prior_play();
void socket_volumn_up();
void socket_volumn_down();
void socket_set_mode(int mode);

#endif // _SOCKET_H
