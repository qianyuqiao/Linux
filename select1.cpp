#include<iostream>
#include<sys/select.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

using namespace std;
void printFDSET(fd_set target_sets)
{
    int uIndex = 0;
    int model_num = __NFDBITS;
    for(uIndex = 0; uIndex < 10/*FD_SETSIZE/model_num*/; uIndex++)
    {
        printf("0x%016lx\t", __FDS_BITS(&target_sets)[uIndex]);
        if(uIndex % 4 == 3)
            printf("\n");
    }
    printf("\n");

    return;
}

int main()
{
    fd_set set1;
    FD_ZERO(&set1);
    FD_SET(31, &set1);
    printFDSET(set1);
    return 0;
}
