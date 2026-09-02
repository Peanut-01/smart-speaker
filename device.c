#include "device.h"
#include "player.h"
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <sys/select.h>
#include <linux/input.h>
#include <sys/time.h>


int g_buttonfd = 0;
BUTTON_STATE state = STATE_IDLE;
struct itimerval tv;
struct timeval old, new;

extern fd_set READSET;
extern int g_maxfd;
extern int g_start_flag;
extern int g_suspend_flag;


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


// 获取音量，单位%
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

    *value = (*value + 5) / 10 * 10;

    snd_mixer_close(mixer);

    return 0;
}


// 初始化按键
int init_button()
{
    g_buttonfd = open("/dev/input/event1", O_RDONLY);
	if (-1 == g_buttonfd)
	{
		perror("open button");
		return -1;
	}

    // 加入 fd 集合，并更新 g_maxfd
    FD_SET(g_buttonfd, &READSET);
    g_maxfd = (g_maxfd > g_buttonfd) ? g_maxfd : g_buttonfd;

    return 0;
}


// 读取按键
void device_read_button()
{
    struct input_event ev;

    int ret = read(g_buttonfd, &ev, sizeof(ev));
    if (-1 == ret)
    {
        perror("read");
        return;
    }

    if (ev.type != EV_KEY)
    {
        return;
    }

    // 判断按键状态
    if (ev.value == 1)	// 被按下
    {
        if (state == STATE_IDLE)
        {
            gettimeofday(&old, NULL);
            state = STATE_FIRST_PRESS;
        }
        else if (state == STATE_FIRST_RELEASE)
        {
            //printf("双击\n");
            player_next_play();

            state = STATE_IDLE;

            tv.it_value.tv_sec = 0;
            tv.it_value.tv_usec = 0;
            tv.it_interval.tv_sec = 0;
            tv.it_interval.tv_usec = 0;

            // 取消定时器
            setitimer(ITIMER_REAL, &tv, NULL);
        }
        // printf("按键按下\n");
    }
    else if (ev.value == 0)	// 松开
    {
        if (state == STATE_FIRST_PRESS)
        {
            gettimeofday(&new, NULL);

            if ((new.tv_sec * 1000 + new.tv_usec / 1000) - (old.tv_sec * 1000 + old.tv_usec / 1000) > 300)
            {
                state = STATE_IDLE;
                player_prior_play();
                //printf("长按\n");
            }
            else
            {
                // 启动定时器，判断是短按还是双击
                state = STATE_FIRST_RELEASE;
                tv.it_value.tv_sec = 0;
                tv.it_value.tv_usec = 300 * 1000; // 300毫秒
                tv.it_interval.tv_sec = 0;
                tv.it_interval.tv_usec = 0;

                setitimer(ITIMER_REAL, &tv, NULL);
            }
        }
    }
}


// 按键处理函数
void button_handler(int sig)
{
    if (g_start_flag == 0)
    {   
        // 开始播放
        player_start_play();
    }
    else if (g_start_flag == 1 && g_suspend_flag == 0)
    {
        // 暂停播放
        player_suspend_play();
    }
    else if (g_start_flag == 1 && g_suspend_flag == 1)
    {
        // 继续播放
        player_continue_play();
    }

    printf("短按\n");
    state = STATE_IDLE;
}
