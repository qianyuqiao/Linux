#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include<string.h>
#include<sys/stat.h>

int main()
{
    int fd;
    char *start;
    //char buf[100];
    char *buf;
    
    /*打开文件*/
    fd = open("/dev/memdev0",O_RDWR);
    if (fd < 0)
    {
        printf("open memdev0 failed\n");
        exit(1);
    }       

    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat("/dev/memdev0", &statbuff) < 0){
        printf("file size is %ld\n", filesize);
    } else {
        filesize = statbuff.st_size;
        printf("file size is %ld\n", filesize);
    }
    

    buf = (char *)malloc(100);
    memset(buf, 0, 100);
    start=mmap(NULL,100,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
    /* 读出数据 */
    strcpy(buf,start);
    sleep (1);
    printf("buf 1 = %s\n",buf);    

    /* 写入数据 */
    strcpy(start,"Buf Is Not Null!");
    
    memset(buf, 0, 100);
    strcpy(buf,start);
    sleep (1);
    printf("buf 2 = %s\n",buf);

       
    munmap(start,100); /*解除映射*/
    free(buf);
    close(fd);  
    return 0;    
}
