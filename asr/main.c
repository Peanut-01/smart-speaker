#include <stdio.h>
#include "alsa.h"


int main()
{
    // 初始化
    if (init_alsa() == -1)
    {
        printf("ALSA初始化失败\n");
        return -1;
    }
    printf("ALSA初始化成功\n");
    
    return 0;
}
