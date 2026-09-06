#include <stdio.h>
#include "alsa.h"
#include "sherpa.h"
#include "sherpa-onnx/c-api/c-api.h"


int running = 1; // 控制主循环的运行状态
extern snd_pcm_t *pcmp; // ALSA PCM设备句柄
extern snd_pcm_uframes_t frams_per_buffer; // ALSA缓冲区帧数
extern unsigned int target_rate; // 重采样后的采样率
extern unsigned int sample_rate; // 实际采样率
extern const SherpaOnnxOnlineStream *asr_stream; // Sherpa ASR音频流
extern const SherpaOnnxOnlineRecognizer *asr_recognizer; // Sherpa ASR识别器


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
        if (resample_buffer == NULL)
        {
            fprintf(stderr, "resample_buffer 内存分配失败\n");
            return -1;
        }

        resample_linear(buffer, frames_read, resample_buffer, resample_frames);

        float *float_buffer = malloc(resample_frames * CHANNELS * sizeof(float));

        if (float_buffer == NULL)
        {
            fprintf(stderr, "float_buffer内存分配失败\n");
            return -1;
        }

        // 转换成浮点数并且归一化
        for (int i = 0; i < resample_frames; i++)
        {
            // 归一化到[-1, 1]
            float_buffer[i] = resample_buffer[i] / 32768.0f;
        }

        // 把数据提交到音频流
        SherpaOnnxOnlineStreamAcceptWaveform(asr_stream, target_rate, float_buffer, resample_frames);
        // 开始识别
        while (SherpaOnnxIsOnlineStreamReady(asr_recognizer, asr_stream))
        {
            SherpaOnnxDecodeOnlineStream(asr_recognizer, asr_stream);
        }

        // 读取数据
        const SherpaOnnxOnlineRecognizerResult *r = SherpaOnnxGetOnlineStreamResult(asr_recognizer, asr_stream);

        // 端点检测
        if (SherpaOnnxOnlineStreamIsEndpoint(asr_recognizer, asr_stream))
        {
            if (r && r->text && strlen(r->text) > 0)
            {
                printf("识别结果---> %s\n", r->text);
                // 清空音频流数据
                SherpaOnnxOnlineStreamReset(asr_recognizer, asr_stream);
            }
        }

        // 清空结果
        SherpaOnnxDestroyOnlineRecognizerResult(r);

        free(resample_buffer); // 释放重采样缓冲区
        free(float_buffer); // 释放浮点数缓冲区
    }
    
    free(buffer); // 释放原始缓冲区

    return 0;
}
