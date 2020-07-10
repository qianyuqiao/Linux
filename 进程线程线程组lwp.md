### 1.pid到底是什么？
```
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

struct message
{
    int i;
    int j;
};

void *hello(void* str)
{
    printf("child, the tid=%lu, pid=%d\n",pthread_self(),syscall(SYS_gettid));
//    printf("the arg.i is %d, arg.j is %d\n",str->i,str->j);
    printf("child, getpid()=%d\n",getpid());
    while(1);
}

int main(int argc, char *argv[])
{
    struct message test;
    pthread_t thread_id;
    test.i=10;
    test.j=20;
    pthread_create(&thread_id,NULL,hello,(void*)(&test));
    printf("parent, the tid=%lu, pid=%d\n",pthread_self(),syscall(SYS_gettid));
    printf("parent, getpid()=%d\n",getpid());
    pthread_join(thread_id,NULL);
    return 0;
}

```
结果:
```
parent, the tid=140102008936192, pid=668
child, the tid=140102000600832, pid=669
a = 10
child, getpid()=668
parent, getpid()=668
```
可以看出，getpid()用于标识一个线程组，但是pid不能标识进程，只能标识线程，tgid才能标识进程
