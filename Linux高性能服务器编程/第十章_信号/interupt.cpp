// 原文链接 https://www.cnblogs.com/black-mamba/p/6876320.html
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define BUFSIZE (1024)

void sig_usr(int signo)
{
    int nRemainSecond = 0;

    if (signo == SIGUSR1)
    {
        printf("received SIGUSR1=%d\n", SIGUSR1);
        nRemainSecond = sleep(50);
        printf("over...nRemainSecond=%d\n", nRemainSecond);
    }
    else if (signo == SIGUSR2)
    {
        printf("received SIGUSR2=%d\n", SIGUSR2);
    }
    
}

int main(int argc, char** argv)
{
    int nSize = 0;
    char acBuf[BUFSIZE] = {0};
    struct sigaction act, oact;
    sigset_t oldmask;

    act.sa_handler = sig_usr;

    sigemptyset(&act.sa_mask);
    //sigaddset(&act.sa_mask, SIGUSR2);
    sigaddset(&act.sa_mask, SIGQUIT);

    act.sa_flags = 0|SA_INTERRUPT;
    sigaction(SIGUSR1, &act, &oact);
    signal(SIGUSR2, sig_usr);

    while(1)
    {
        memset(acBuf, '\0', BUFSIZE);
        nSize = read(STDIN_FILENO, acBuf, BUFSIZE); 
        if (errno == EINTR)
            printf("interrupt, size=%d\n", nSize);

        if (1 == nSize && acBuf[0] == 10)
            ;
        else if (nSize != -1)
        {
            printf("nSize=%d, acBuf=%s", nSize, acBuf);
        }
    }

    return 0;
}
