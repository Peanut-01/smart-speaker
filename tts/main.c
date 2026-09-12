#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <stdlib.h>
#include "tts.h"
#include "alsa.h"
#include "sherpa-onnx/c-api/c-api.h"


int running = 1;
int tts_fd = 0;

extern const SherpaOnnxOfflineTts *tts;
extern snd_pcm_t *pcmp;
extern int play_flag;


void clean_up()
{
    if (pcmp)
    {
        snd_pcm_close(pcmp);
    }

    if (tts)
    {
        SherpaOnnxDestroyOfflineTts(tts);
    }

    if (tts_fd > 0)
    {
        close(tts_fd);
    }

    exit(0);
}


// 退出程序
void quit_handler(int sig)
{
    printf("程序准备退出...\n");
    running = 0;
    clean_up();
}


// 结束语音合成
void stop_handler(int sig)
{
    play_flag = 0;
    // 清空缓冲区数据
    snd_pcm_drop(pcmp);
}


int main()
{
    signal(SIGINT, quit_handler);
    signal(SIGUSR1, stop_handler);  // 结束语音合成

    if (init_sherpa_tts() == -1)
    {
        printf("tts 初始化失败\n");
        clean_up();
        return -1;
    }
    printf("tts 初始化成功\n");

    if (init_alsa_playback() == -1)
    {
        printf("alsa 初始化失败\n");
        clean_up();
        return -1;
    }
    printf("alsa 初始化成功\n");

    // 打开管道
    tts_fd = open("/home/fifo/tts_fifo", O_RDONLY);
    if (-1 == tts_fd)
    {
        perror("open fifo");
        clean_up();
        return -1;
    }

    char buf[1024] = {0};

    while (running)
    {
        // 读取管道数据
        size_t size = read(tts_fd, buf, sizeof(buf));
        if (-1 == size)
        {
            perror("read");
            continue;
        }
        else if (0 == size)
        {
            sleep(1);
            continue;
        }

        play_flag = 1;    // 允许语音合成
        
        snd_pcm_prepare(pcmp);  // PREPARED状态
        
        // tts实例 需要合成的文本 说话的声音 语速 用于播放的回调函数
		SherpaOnnxOfflineTtsGenerateWithCallback(tts, buf, 1, 1.0, play_callback);

        // 等待缓冲区数据播放完成
        snd_pcm_drain(pcmp);    // SETUP状态

        memset(buf, 0, sizeof(buf));
        
    }
    

    return 0;
}