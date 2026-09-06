#include <stdio.h>
#include "alsa.h"


snd_pcm_t *pcmp;
snd_pcm_uframes_t frams_per_buffer = 1024;
unsigned int sample_rate = RATE; // 原采样率


// 初始化ALSA
int init_alsa()
{
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
    unsigned int actual_rate = sample_rate;
    snd_pcm_hw_params_set_rate_near(pcmp, params, &actual_rate, 0);
    if (actual_rate != RATE) {
        fprintf(stderr, "不支持采样率 %uHz, 实际使用 %uHz\n", RATE, actual_rate);
        sample_rate = actual_rate; // 更新采样率为实际使用的值
    }

    // 7.设置缓冲区周期大小：1024
    snd_pcm_uframes_t period_buffer = frams_per_buffer;
    snd_pcm_hw_params_set_period_size_near(pcmp, params, &period_buffer, 0);

    // 8.应用硬件参数
    snd_pcm_hw_params(pcmp, params);

    // 9.准备PCM设备
    snd_pcm_prepare(pcmp);

    return 0;
}


// 线性插值重采样
void resample_linear(const int16_t *input, size_t input_len, int16_t *output, size_t output_len)
{
    if (0 == input_len || 0 == output_len)
    {
        return;
    }

    if (1 == input_len || 1 == output_len)
    {
        output[0] = input[0];
        return;
    }

    double ratio = (double)(input_len - 1) / (output_len - 1);
    
    for (size_t i = 0; i < output_len; ++i)
    {
        
        // 计算输入帧的索引
        double pos = (double)i * input_len / output_len;
        size_t idx = (size_t)pos;
        float deci = pos - idx;

        // 线性插值
        if (idx + 1 < input_len)
        {
            output[i] = (int16_t)((1.0f - deci) * input[idx] + deci * input[idx + 1] + 0.5f);
        }
        else
        {
            output[i] = input[input_len - 1]; // 如果索引超出范围，则直接使用最后一个样本
        }
    }
}
