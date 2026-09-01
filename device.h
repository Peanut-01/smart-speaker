#if !defined(_DEVICE_H)
#define _DEVICE_H

#define CARD_NAME "hw:AudioPCI"
#define ELEM_NAME "Master"

typedef enum {
	STATE_IDLE,
	STATE_FIRST_PRESS,
	STATE_FIRST_RELEASE,
	STATE_SECOND_PRESS
} BUTTON_STATE;

int device_set_volume(int volume);
int device_get_volume(int *value);
int init_button();
void button_handler(int sig);
void device_read_button();

#endif // _DEVICE_H
