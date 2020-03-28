### 父进程杀死子进程
```
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
using namespace std;
int main()
{
    pid_t pid;
    pid = fork();
    int ret;
    if (pid < 0)
    {
        cout << "create subprocess failed" << endl;
        exit(-1);
    }
    else if (pid > 0)
    {
        sleep(1);
        cout << "parent process" << endl;
        kill(pid, 9);
        cout << "kill " << pid << endl;
        exit(0);
    }
    else
    {
        cout << "child process" << endl;
        sleep(20);
        exit(0);
    }
    return 0;
}

```
