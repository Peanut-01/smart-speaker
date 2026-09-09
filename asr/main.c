#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include "alsa.h"
#include "sherpa.h"
#include "kws.h"
#include "sherpa-onnx/c-api/c-api.h"


enum Appstate
{
    STATE_KWS,
    STATE_ASR
};


int running = 1; // 控制主循环的运行状态
enum Appstate cur_state = STATE_KWS; // 当前应用状态，初始为唤醒词识别
int asr_fd; // 用于与ASR进程通信的文件描述符

extern snd_pcm_t *pcmp; // ALSA PCM设备句柄
extern snd_pcm_uframes_t frams_per_buffer; // ALSA缓冲区帧数
extern unsigned int target_rate; // 重采样后的采样率
extern unsigned int sample_rate; // 实际采样率
extern const SherpaOnnxOnlineStream *asr_stream; // Sherpa ASR音频流
extern const SherpaOnnxOnlineRecognizer *asr_recognizer; // Sherpa ASR识别器


void quit_handler(int sig)
{
    running = 0;
    printf("退出程序...\n");
}


void clean_up()
{
    // 关闭PCM设备
    if (pcmp)
    {
        snd_pcm_close(pcmp);
        pcmp = NULL;
    }

    // 销毁Sherpa ASR识别器和音频流
    if (asr_stream)
    {
        SherpaOnnxDestroyOnlineStream(asr_stream);
    }
    if (asr_recognizer)
    {
        SherpaOnnxDestroyOnlineRecognizer(asr_recognizer);
    }
}


int main()
{
    // 信号处理函数
    signal(SIGINT, quit_handler);

    // 初始化
    if (init_alsa() == -1)
    {
        printf("ALSA初始化失败\n");
        clean_up();
        return -1;
    }
    printf("ALSA初始化成功\n");

    // 初始化语音识别
    if (init_sherpa_asr() == -1)
    {
        printf("语音识别初始化失败\n");
        clean_up();
        return -1;
    }
    printf("语音识别初始化成功\n");

    // 初始化唤醒词识别
    if (init_sherpa_kws() == -1)
    {
        printf("唤醒词识别初始化失败\n");
        clean_up();
        return -1;
    }
    printf("唤醒词识别初始化成功\n");

    // 打开管道
    asr_fd = open("/home/fifo/asr_fifo", O_WRONLY);
    if (asr_fd == -1)
    {
        fprintf(stderr, "open fifo error\n");
        clean_up();
        return -1;
    }

    printf("\n\n=======关键词识别模式========\n");
    printf("请说出关键词唤醒...\n");

    // 申请内存存放读取的音频数据
    int16_t *buffer = malloc(frams_per_buffer * CHANNELS * sizeof(int16_t));

    if (buffer == NULL)
    {
        fprintf(stderr, "buffer内存分配失败\n");
        clean_up();
        return -1;
    }

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
        size_t resample_frames = (size_t)((double)frames_read * target_rate / sample_rate + 0.5); // 计算重采样后的帧数
        int16_t *resample_buffer = malloc(resample_frames * CHANNELS * sizeof(int16_t));
        if (resample_buffer == NULL)
        {
            fprintf(stderr, "resample_buffer 内存分配失败\n");
            break;
        }

        resample_linear(buffer, frames_read, resample_buffer, resample_frames);

        float *float_buffer = malloc(resample_frames * CHANNELS * sizeof(float));

        if (float_buffer == NULL)
        {
            fprintf(stderr, "float_buffer内存分配失败\n");
            free(resample_buffer);
            break;
        }

        // 转换成浮点数并且归一化
        for (int i = 0; i < resample_frames; i++)
        {
            // 归一化到[-1, 1]
            float_buffer[i] = resample_buffer[i] / 32768.0f;
        }   

        if (cur_state == STATE_ASR)
        {
            if (sherpa_asr(float_buffer, resample_frames))
            {
                cur_state = STATE_KWS;
                printf("\n=======关键词识别模式========\n");
                printf("请说出关键词唤醒...\n");
            }
        }
        else if (cur_state == STATE_KWS)
        {
            if (sherpa_kws(float_buffer, resample_frames))
            {
                cur_state = STATE_ASR;
                printf("\n=======语音识别模式========\n");
                printf("请说话...\n");

                sleep(2);
                snd_pcm_drop(pcmp);  // 清空缓冲区 变成SETUP状态
                snd_pcm_prepare(pcmp);
            }
        }

        free(resample_buffer); // 释放重采样缓冲区
        free(float_buffer); // 释放浮点数缓冲区

        usleep(1000);
    }
    
    free(buffer); // 释放原始缓冲区

    clean_up(); // 清理资源

    return 0;
}
