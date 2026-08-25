#if !defined(_DEVICE_H)
#define _DEVICE_H

#define CARD_NAME "hw:AudioPCI"
#define ELEM_NAME "Master"

int device_set_volume(int volume);
int device_get_volume(int *value);

#endif // _DEVICE_H
