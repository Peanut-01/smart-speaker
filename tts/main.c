#include <stdio.h>
#include "sherpa-onnx/c-api/c-api.h"
#include "tts.h"
#include "alsa.h"


int running = 1;
extern const SherpaOnnxOfflineTts *tts;


int main()
{
    if (init_sherpa_tts() == -1)
    {
        printf("tts 初始化失败\n");
        return -1;
    }
    printf("tts 初始化成功\n");

    if (init_alsa_playback() == -1)
    {
        printf("alsa 初始化失败\n");
        return -1;
    }
    printf("alsa 初始化成功\n");

    const char *s = "今天天气不错";
    while (running)
    {
        //tts实例 需要合成的文本 说话的声音 语速 用于播放的回调函数
		SherpaOnnxOfflineTtsGenerateWithCallback(tts, s, 1, 1.0, play_callback);
        while (1)
        {
            /* code */
        }
        
    }
    

    return 0;
}