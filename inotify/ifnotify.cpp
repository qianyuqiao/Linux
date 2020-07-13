/*
*   File:  ifnotify.cpp
*   Date:  2019.12.10
*   Author: Qianyuqiao
*
*   A simple tester of fanotify in the Linux kernel.5.0.4 and higher
*
*   This program is released in the Public Domain.
*
*   Compile with:
*     $> g++ -o ifnotify ifnotify.cpp
*
*   Run as:
*     $> ./ifnotify /home/tmp /home/tmp2 ............
*   Output as:
*     2019-12-10 21:57:25 8499 [dir open] /home/tmp/
*     2019-12-10 21:57:25 8499 [dir access] /home/tmp/
*     2019-12-10 21:57:25 8499 [dir close] /home/tmp/
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
#include <sys/fanotify.h>
#include <deque>
#include <time.h>
#include <unordered_map>
using namespace std;
/* Structure to keep track of monitored directories */
typedef struct
{
    /* Path of the directory */
    char *path;
} monitored_t;
#define FD_POLL_MAX 3
/* Size of buffer to use when reading fanotify events */
#define FANOTIFY_BUFFER_SIZE 8192

/* Enumerate list of FDs to poll */
enum
{
    FD_POLL_SIGNAL = 0,
    FD_POLL_FANOTIFY
};

/* Setup fanotify notifications (FAN) mask. All these defined in fanotify.h. */
static uint64_t event_mask =
    (FAN_ACCESS | /* File accessed */
     FAN_MODIFY | /* File modified */
     FAN_CLOSE |  /* Writtable file closed */
     FAN_OPEN |   /* File was opened */
     FAN_ONDIR |  /* We want to be reported of events in the directory */
     FAN_CREATE |
     FAN_DELETE |
     FAN_MOVED_FROM |
     FAN_MOVED_TO |
     FAN_ATTRIB |
     FAN_EVENT_ON_CHILD); /* We want to be reported of events in files of the directory */

static unordered_map<string, deque<string>> Q;
/* Array of directories being monitored */
static monitored_t *monitors;
static int n_monitors;

void get_time(char *res)
{
    char hour_str[4];
    char min_str[4];
    char sec_str[4];
    char month_str[4];
    char day_str[4];
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    int hour = (p->tm_hour + 8) % 24;
    int min = p->tm_min;
    int sec = p->tm_sec;
    int year = p->tm_year + 1900;
    int month = p->tm_mon + 1;
    int day = p->tm_mday;

    if (hour < 10)
        sprintf(hour_str, "0%d", hour);
    else
        sprintf(hour_str, "%d", hour);
    if (min < 10)
        sprintf(min_str, "0%d", min);
    else
        sprintf(min_str, "%d", min);
    if (sec < 10)
        sprintf(sec_str, "0%d", sec);
    else
        sprintf(sec_str, "%d", sec);
    if (month < 10)
        sprintf(month_str, "0%d", month);
    else
        sprintf(month_str, "%d", month);
    if (day < 10)
        sprintf(day_str, "0%d", day);
    else
        sprintf(day_str, "%d", day);

    sprintf(res, "%d-%s-%s %s:%s:%s", year, month_str, day_str, hour_str, min_str, sec_str);
}

static void handle_events(int fd, int *wd, int argc, const char **argv, int pid)
{
    /* Some systems cannot read integer variables if they are not
   properly aligned. On other systems, incorrect alignment may
   decrease performance. Hence, the buffer used for reading from
   the inotify file descriptor should have the same alignment as
   struct inotify_event. */

    char buf[4096]
        __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;

    /* Loop while events can be read from inotify file descriptor. */

    for (;;)
    {

        /* Read some events. */
        char ctime[50];
        get_time(ctime);
        string tt = ctime;
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

            string isdir = (event->mask & IN_ISDIR ? "dir" : "file");

            string path = "";
            string event_name = "";
            for (i = 1; i < argc; ++i)
            {
                if (wd[i] == event->wd)
                {
                    path = argv[i];
                    break;
                }
            }
            path += string(1, '/');
            /* Print the name of the file */
            if (event->len)
                event_name = (string)event->name;

            /* Print event type */
            if (event->mask & IN_OPEN)
                cout << tt << " " << pid << " [" << isdir << " open]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_ACCESS)
                cout << tt << " " << pid << " [" << isdir << " access]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_MODIFY)
                cout << tt << " " << pid << " [" << isdir << " modify]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_MOVED_FROM)
                cout << tt << " " << pid << " [" << isdir << " moved_from]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_MOVED_TO)
                cout << tt << " " << pid << " [" << isdir << " moved_to]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_CREATE)
                cout << tt << " " << pid << " [" << isdir << " create]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_DELETE)
                cout << tt << " " << pid << " [" << isdir << " delete]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_ATTRIB)
                cout << tt << " " << pid << " [" << isdir << " attrib]"
                     << " " << path << event_name << endl;
            if (event->mask & IN_CLOSE)
                cout << tt << " " << pid << " [" << isdir << " close]"
                     << " " << path << event_name << endl;

            /* Print the name of the watched directory */
        }
    }
}

static char *get_program_name_from_pid(int pid, char *buffer, size_t buffer_size)
{
    int fd;
    ssize_t len;
    char *aux;

    /* Try to get program name by PID */
    sprintf(buffer, "/proc/%d/cmdline", pid);
    if ((fd = open(buffer, O_RDONLY)) < 0)
        return NULL;

    /* Read file contents into buffer */
    if ((len = read(fd, buffer, buffer_size - 1)) <= 0)
    {
        close(fd);
        return NULL;
    }
    close(fd);

    buffer[len] = '\0';
    aux = strstr(buffer, "^@");
    if (aux)
        *aux = '\0';

    return buffer;
}

static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
    ssize_t len;

    if (fd <= 0)
        return NULL;

    sprintf(buffer, "/proc/self/fd/%d", fd);
    if ((len = readlink(buffer, buffer, buffer_size - 1)) < 0)
        return NULL;

    buffer[len] = '\0';
    return buffer;
}

static void shutdown_fanotify(int fanotify_fd)
{
    int i;

    for (i = 0; i < n_monitors; ++i)
    {
        /* Remove the mark, using same event mask as when creating it */
        fanotify_mark(fanotify_fd, FAN_MARK_REMOVE, event_mask, AT_FDCWD, monitors[i].path);
        free(monitors[i].path);
    }
    free(monitors);
    close(fanotify_fd);
}

static int initialize_fanotify(int argc, const char **argv)
{
    int i;
    int fanotify_fd;

    /* Create new fanotify device */
    if ((fanotify_fd = fanotify_init(FAN_CLOEXEC | FAN_REPORT_FID, O_RDONLY | O_CLOEXEC | O_LARGEFILE)) < 0)
    {
        fprintf(stderr, "Couldn't setup new fanotify device: %s\n", strerror(errno));
        return -1;
    }

    /* Allocate array of monitor setups */
    n_monitors = argc - 1;
    monitors = (monitored_t *)malloc(n_monitors * sizeof(monitored_t));

    /* Loop all input directories, setting up marks */
    for (i = 0; i < n_monitors; ++i)
    {
        monitors[i].path = strdup(argv[i + 1]);
        /* Add new fanotify mark */
        if (fanotify_mark(fanotify_fd,
                          FAN_MARK_ADD,
                          event_mask,
                          AT_FDCWD,
                          monitors[i].path) < 0)
        {
            fprintf(stderr,
                    "Couldn't add monitor in directory '%s': '%s'\n",
                    monitors[i].path,
                    strerror(errno));
            return -1;
        }
    }

    return fanotify_fd;
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

    /* Initialize fanotify FD and the marks */
    if ((fanotify_fd = initialize_fanotify(argc, argv)) < 0)
    {
        fprintf(stderr, "Couldn't initialize fanotify\n");
        exit(EXIT_FAILURE);
    }

    /* Setup polling */
    fds[FD_POLL_SIGNAL].fd = signal_fd;
    fds[FD_POLL_SIGNAL].events = POLLIN;
    fds[FD_POLL_FANOTIFY].fd = fanotify_fd;
    fds[FD_POLL_FANOTIFY].events = POLLIN;

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

    wd = (int *)calloc(argc, sizeof(int));
    if (wd == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    /* Mark directories for events
  - file was opened
  - file was closed */

    for (i = 1; i < argc; i++)
    {
        wd[i] = inotify_add_watch(fd, argv[i], IN_OPEN | IN_CLOSE | IN_MOVE | IN_CREATE | IN_DELETE | IN_MODIFY | IN_ACCESS | IN_ATTRIB);
        if (wd[i] == -1)
        {
            fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
            perror("inotify_add_watch");
            exit(EXIT_FAILURE);
        }
    }

    fds[2].fd = fd;
    fds[2].events = POLLIN;
    deque<int> DQ;
    int pid = -1;
    int last_pid = -1;
    struct signalfd_siginfo fdsi;

    /* Now loop */
    for (;;)
    {

        /* Block until there is something to be read */
        if (poll(fds, FD_POLL_MAX, -1) < 0)
        {
            fprintf(stderr,
                    "Couldn't poll(): '%s'\n",
                    strerror(errno));
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

            fprintf(stderr,
                    "Received unexpected signal\n");
        }

        /* fanotify event received? */
        if (fds[FD_POLL_FANOTIFY].revents & POLLIN)
        {
            char buffer[FANOTIFY_BUFFER_SIZE];
            ssize_t length;

            /* Read from the FD. It will read all events available up to
            * the given buffer size. */
            if ((length = read(fds[FD_POLL_FANOTIFY].fd,
                               buffer,
                               FANOTIFY_BUFFER_SIZE)) > 0)
            {
                struct fanotify_event_metadata *metadata;

                metadata = (struct fanotify_event_metadata *)buffer;
                while (FAN_EVENT_OK(metadata, length))
                {
                    //event_process (metadata);
                    pid = (int)metadata->pid;

                    if (pid != last_pid)
                    {
                        DQ.push_front(pid);
                        last_pid = pid;
                    }

                    if (metadata->fd > 0)
                        close(metadata->fd);
                    metadata = FAN_EVENT_NEXT(metadata, length);
                }
            }
        }

        if (fds[2].revents & POLLIN)
        {

            /* Inotify events are available */
            if (DQ.size())
            {
                int back_pid = DQ.back();
                DQ.pop_back();
                handle_events(fd, wd, argc, argv, back_pid);
            }
        }
    }

    /* Clean exit */
    shutdown_fanotify(fanotify_fd);
    shutdown_signals(signal_fd);
    close(fd);
    free(wd);
    return EXIT_SUCCESS;
}
