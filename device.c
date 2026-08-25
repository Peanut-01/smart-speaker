#include "device.h"
#include <stdio.h>
#include <alsa/asoundlib.h>


// 调用 alsa 函数设置系统音量，参数：音量百分比
int device_set_volume(int volume)
{
    snd_mixer_t *mixer;
    // 打开混音设备
    if (snd_mixer_open(&mixer, 0) != 0)
    {
        fprintf(stderr, "snd_mixer_open error");
        return -1;
    }

    // 附加声卡设备
    snd_mixer_attach(mixer, CARD_NAME);

    // 注册混音器
    snd_mixer_selem_register(mixer, NULL, NULL);

    // 加载混音器
    snd_mixer_load(mixer);

    // 设置元素ID
    snd_mixer_selem_id_t *id;
    snd_mixer_selem_id_alloca(&id);
    snd_mixer_selem_id_set_index(id, 0);
    snd_mixer_selem_id_set_name(id, ELEM_NAME);

    snd_mixer_elem_t *elem = snd_mixer_find_selem(mixer, id);
    if (NULL == elem)
    {
        fprintf(stderr, "snd_mixer_find_selem error");
        return -1;
    }

    // 获取音量范围
    long min ,max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

    // 设置音量数值
    long value = (max - min) * volume / 100 + min;
    snd_mixer_selem_set_playback_volume_all(elem, value);

    // printf("设置成功 %ld\n", value);

    snd_mixer_close(mixer);

    return 0;
}


int device_get_volume(int *value)
{
    snd_mixer_t *mixer;
    // 打开混音设备
    if (snd_mixer_open(&mixer, 0) != 0)
    {
        fprintf(stderr, "snd_mixer_open error");
        return -1;
    }

    // 附加声卡设备
    snd_mixer_attach(mixer, CARD_NAME);

    // 注册混音器
    snd_mixer_selem_register(mixer, NULL, NULL);

    // 加载混音器
    snd_mixer_load(mixer);

    // 设置元素ID
    snd_mixer_selem_id_t *id;
    snd_mixer_selem_id_alloca(&id);
    snd_mixer_selem_id_set_index(id, 0);
    snd_mixer_selem_id_set_name(id, ELEM_NAME);

    snd_mixer_elem_t *elem = snd_mixer_find_selem(mixer, id);
    if (NULL == elem)
    {
        fprintf(stderr, "snd_mixer_find_selem error");
        return -1;
    }

    // 获取音量范围
    long min ,max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

    long volume;
    // 获取音量数值
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &volume);

    *value = (volume - min) * 100 / (max - min);

    snd_mixer_close(mixer);

    return 0;
}
