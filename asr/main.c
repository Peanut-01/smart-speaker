#include <stdio.h>
#include "alsa.h"
#include "sherpa.h"


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

    return 0;
}
