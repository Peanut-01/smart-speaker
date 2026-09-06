#include <stdio.h>
#include "alsa.h"
#include "sherpa.h"


int running = 1; // 控制主循环的运行状态
extern snd_pcm_t *pcmp; // ALSA PCM设备句柄
extern snd_pcm_uframes_t frams_per_buffer; // ALSA缓冲区帧数
extern unsigned int target_rate; // 重采样后的采样率
extern unsigned int sample_rate; // 实际采样率


int main()
{
    // 初始化
    if (init_alsa() == -1)
    {
        printf("ALSA初始化失败\n");
        return -1;
    }
    printf("ALSA初始化成功\n");

    // 初始化语音识别
    if (init_sherpa_asr() == -1)
    {
        printf("语音识别初始化失败\n");
        return -1;
    }
    printf("语音识别初始化成功\n");

    // 申请内存存放读取的音频数据
    int16_t *buffer = malloc(frams_per_buffer * CHANNELS * sizeof(int16_t));

    while (running)
    {
        // 读取数据
        snd_pcm_sframes_t frames_read = snd_pcm_readi(pcmp, buffer, frams_per_buffer);
        if (frames_read == -EPIPE)
        {
            // 缓冲区溢出
            snd_pcm_prepare(pcmp);
            fprintf(stderr, "缓冲区溢出\n");
            continue;
        }
        else if (frames_read < 0)
        {
            fprintf(stderr, "读取音频数据失败: %s\n", snd_strerror(frames_read));
            continue;
        }

        // 重采样
        size_t resample_frames = frames_read * target_rate / sample_rate + 0.5; // 计算重采样后的帧数
        int16_t *resample_buffer = malloc(resample_frames * CHANNELS * sizeof(int16_t));
        resample_linear(buffer, frames_read, resample_buffer, resample_frames);
        // 转换成浮点数并且归一化
        for (int i = 0; i < resample_frames; i++)
        {
            // 归一化到[-1, 1]
            resample_buffer[i] = resample_buffer[i] / 32768.0f;
        }

        free(resample_buffer); // 释放重采样缓冲区
    }
    
    free(buffer); // 释放原始缓冲区

    return 0;
}
