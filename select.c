#include <sys/select.h>
#include <unistd.h>

fd_set READSET;

int init_select() 
{
    FD_ZERO(&READSET);
    FD_SET(STDIN_FILENO, &READSET);
    return 0;
}
