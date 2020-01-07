/*
usage : 
1. 编译: 
    g++ sync_log.cpp -o sync_log -std=c++11
2. 执行:
    ./sync_log 源目录路径  源目录要实时同步到的目标目录路径
3. 说明：
    本代码支持对源目录的原有的子目录进行同步，但不支持对源目录新增的子目录进行同步

*/
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <dirent.h>

using namespace std;
/* Structure to keep track of monitored directories */
typedef struct
{
    /* Path of the directory */
    char *path;
} monitored_t;
#define FD_POLL_MAX 2
/* Size of buffer to use when reading fanotify events */
#define FANOTIFY_BUFFER_SIZE 8192

/* Enumerate list of FDs to poll */
enum
{
    FD_POLL_SIGNAL = 0,
    FD_POLL_FANOTIFY
};

/* Setup fanotify notifications (FAN) mask. All these defined in fanotify.h. */


//static unordered_map<string, deque<string>> Q;
/* Array of directories being monitored */
static monitored_t *monitors;
static int n_monitors;

static bool exec(string check_command)
{
    char return_value[1024];
    FILE* ptr = NULL;
    if((ptr = popen(check_command.c_str(), "r")) == NULL)
    {
        return false;
    }
    memset(return_value, 0, sizeof(return_value));
    while ((fgets(return_value, sizeof(return_value), ptr))!= NULL)
    {
        cout << return_value;
    }
    pclose(ptr);
    return true;
}

static void listdir(char* path, vector<string>& Q)
{
     string pathstr = "";
     Q.push_back(path);
     struct dirent* ent = NULL;
     DIR *pDir = opendir(path);
     string cur = "";
     char tmp[255];
     while ((ent=readdir(pDir)))
     {
         if (ent->d_reclen==24 && ent->d_type == 4)
         {
			memset(tmp, 0, sizeof(tmp));
            cur = ent->d_name;
            if (cur == "." || cur == "..") continue;
			pathstr = path;
            pathstr += "/" + cur;
            strcpy(tmp, pathstr.c_str());
            listdir(tmp, Q);
         }
     }
}

static void handle_events(int fd, int *wd, int argc, const char **argv, vector<string>& dirs, string cmd)
{
    /* Some systems cannot read integer variables if they are not
   properly aligned. On other systems, incorrect alignment may
   decrease performance. Hence, the buffer used for reading from
   the inotify file descriptor should have the same alignment as
   struct inotify_event. */
	int dirs_size = dirs.size();
    char buf[4096]
        __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;

    for (;;)
    {

        /* Read some events. */
        len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

         /* If the nonblocking read() found no events to read, then
        it returns -1 with errno set to EAGAIN. In that case,
        we exit the loop. */

        if (len <= 0)
            break;

        /* Loop over all events in the buffer */

        for (ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {

            event = (const struct inotify_event *)ptr;

            if (event->mask & IN_ISDIR) continue;	
        
            if ((event->mask & IN_CREATE) || (event->mask & IN_MODIFY)) {
			    bool res = exec(cmd);
			    if (res) cout << "sync sucess!" ;
				else cout << "sync err!";
			}


            /* Print the name of the watched directory */
        }
    }
	//bool res = exec(cmd);
    //if (res) cout << "sync " << path << " to " << dest << " sucess!" ;
    //else cout << "sync " << path << " to " << dest << " err!";

}

static bool init_sync(string cmd) 
{
    bool res = exec(cmd);
    if (res) {
		cout << "init sync success" << endl;
		return true;	
	}

    else {
		cout << "init sync failed" << endl;
		return false;
	}
}
static void shutdown_signals(int signal_fd)
{
    close(signal_fd);
}

static int initialize_signals(void)
{
    int signal_fd;
    sigset_t sigmask;

    /* We want to handle SIGINT and SIGTERM in the signal_fd, so we block them. */
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);

    if (sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0)
    {
        fprintf(stderr,
                "Couldn't block signals: '%s'\n",
                strerror(errno));
        return -1;
    }

    /* Get new FD to read signals from it */
    if ((signal_fd = signalfd(-1, &sigmask, 0)) < 0)
    {
        fprintf(stderr,
                "Couldn't setup signal FD: '%s'\n",
                strerror(errno));
        return -1;
    }

    return signal_fd;
}

int main(int argc, const char **argv)
{
    int signal_fd;
    int fanotify_fd;
    struct pollfd fds[FD_POLL_MAX];

    /* Input arguments... */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s directory1 [directory2 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Initialize signals FD */
    if ((signal_fd = initialize_signals()) < 0)
    {
        fprintf(stderr, "Couldn't initialize signals\n");
        exit(EXIT_FAILURE);
    }


    /* Setup polling */
    fds[FD_POLL_SIGNAL].fd = signal_fd;
    fds[FD_POLL_SIGNAL].events = POLLIN;

    char buf;
    int fd, i, poll_num;
    int *wd;
    nfds_t nfds;

    /* Create the file descriptor for accessing the inotify API */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
    {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    /* Allocate memory for watch descriptors */
    vector<string> dirs;
	char dir0[100];
	strcpy(dir0, argv[1]);
	
	listdir(dir0, dirs);
	cout << "dirs: " << endl;
	for (string s: dirs) cout << s << endl;
    wd = (int *)calloc(dirs.size(), sizeof(int));
    if (wd == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    /* Mark directories for events
  - file was opened
  - file was closed */

    for (i = 0; i < dirs.size(); i++)
    {
		wd[i] = inotify_add_watch(fd, (dirs[i]).c_str(), IN_MODIFY | IN_CREATE);
		if (wd[i] == -1)
		{
			fprintf(stderr, "Cannot watch '%s'\n", (dirs[i]).c_str());
			perror("inotify_add_watch");
			exit(EXIT_FAILURE);
		}
     }

    fds[1].fd = fd;
    fds[1].events = POLLIN;
    struct signalfd_siginfo fdsi;
	string src = argv[1];
	string dst = argv[2];
	string cmd = "rsync -av " + src + " " + dst;
	if (!init_sync(cmd)) 
	{
		perror("init sync failed, source directory or destination directory may be invalid");
		exit(EXIT_FAILURE);
	}
    /* Now loop */
    for (;;)
    {

        /* Block until there is something to be read */
        if (poll(fds, FD_POLL_MAX, -1) < 0)
        {
            fprintf(stderr, "Couldn't poll(): '%s'\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Signal received? */
        if (fds[FD_POLL_SIGNAL].revents & POLLIN)
        {

            if (read(fds[FD_POLL_SIGNAL].fd,
                     &fdsi,
                     sizeof(fdsi)) != sizeof(fdsi))
            {
                fprintf(stderr,
                        "Couldn't read signal, wrong size read\n");
                exit(EXIT_FAILURE);
            }

            /* Break loop if we got the expected signal */
            if (fdsi.ssi_signo == SIGINT ||
                fdsi.ssi_signo == SIGTERM)
            {
                break;
            }

            fprintf(stderr, "Received unexpected signal\n");
        }

        if (fds[1].revents & POLLIN)
        {
            /* Inotify events are available */
            handle_events(fd, wd, argc, argv, dirs, cmd);
		}
    }

    /* Clean exit */
    shutdown_signals(signal_fd);
    close(fd);
    free(wd);
    return EXIT_SUCCESS;
}
