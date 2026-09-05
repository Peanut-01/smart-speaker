#include <stdio.h>
#include "alsa.h"


// 初始化ALSA
int init_alsa()
{
    snd_pcm_t *pcmp;
    int ret;
    // 1.打开PCM设备
    ret = snd_pcm_open(&pcmp, RECORD_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (ret != 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(ret));
        return -1;
    }

    // 2.初始化硬件参数结构体（申请内存）
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcmp, params);

    // 3.设置访问模式：多声道交错存储
    snd_pcm_hw_params_set_access(pcmp, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    // 4.设置采样格式：16位小端
    ret = snd_pcm_hw_params_set_format(pcmp, params, SND_PCM_FORMAT_S16_LE);
    if (ret != 0) {
        fprintf(stderr, "不支持S16_LE格式: %s\n", snd_strerror(ret));
        return -1;
    }

    // 5.设置声道数：单声道
    snd_pcm_hw_params_set_channels(pcmp, params, CHANNELS);

    // 6.设置采样率：44100Hz
    unsigned int actual_rate = RATE;
    snd_pcm_hw_params_set_rate_near(pcmp, params, &actual_rate, 0);
    if (actual_rate != RATE) {
        fprintf(stderr, "不支持采样率 %dHz, 实际使用 %dHz\n", RATE, actual_rate);
        return -1;
    }

    // 7.设置缓冲区周期大小：1024
    snd_pcm_uframes_t period_buffer = PERIOD_BUFFER;
    snd_pcm_hw_params_set_period_size_near(pcmp, params, &period_buffer, 0);

    // 8.应用硬件参数
    snd_pcm_hw_params(pcmp, params);

    // 9.准备PCM设备
    snd_pcm_prepare(pcmp);

    return 0;
}