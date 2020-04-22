#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

static const int maxLength = 100;
static int lengthLeft = maxLength;
static int load_fd = -1;
static Window root;
static Display *dpy = NULL;
static int must_quit = 0;
char **localargv;

pthread_cond_t condition;
pthread_mutex_t mutex;
int triggered;

char*
load_status(char* status)
{
    static char str[5];
    int n, i, done = 0;
    const char* label = "load:";

    for(i = 0; label[i] && lengthLeft > 1; ++i)
    {
        *status = label[i];
        ++status;
        --lengthLeft;
    }

    if (load_fd >= 0)
    {
        lseek(load_fd, 0, SEEK_SET);
        while (!done)
        {
            n = read(load_fd, (void*)str, 5);
            for(i = 0; i < 5 && lengthLeft > 1; ++i)
            {
                if(str[i] == ' ')
                {
                    done = 1;
                    break;
                }
                *status = str[i];
                ++status;
                --lengthLeft;
            }
        }
    }
    return status;
}

char*
timedate_status(char* status)
{
    char str_date[256];
    int length = 256;
    time_t rawtime;
    struct tm timeinfo;
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    tzset();
    if (localtime_r(&(ts.tv_sec), &timeinfo) == NULL)
        return status;
    if (lengthLeft < length)
        length = lengthLeft - 1;
    strftime(status, length, "%a %d %b %H:%M", &timeinfo);
    while (*status)
    {
        ++status;
        --lengthLeft;
    }
    return status;
}

char*
separator_status(char* status)
{
    const char* separator = " \u239C ";
    int i = 0;

    while (separator[i] && lengthLeft > 1)
    {
        *status = separator[i++];
        --lengthLeft;
        ++status;
    }
    return status;
}

void
wait_until (struct timespec time)
{
    pthread_mutex_lock(&mutex);
    if (!triggered)
    {
        do
        {
            if (pthread_cond_timedwait(&condition, &mutex, &time) != 0)
            {
                pthread_mutex_unlock(&mutex);
                return ;
            }
        }
        while(!triggered);
    }
    triggered = 0;
    pthread_mutex_unlock(&mutex);
}

void
sleep_to_next()
{
    struct timespec ts;
    static const long unit_sec = 60;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec = (ts.tv_sec / unit_sec + 1) * unit_sec;
    ts.tv_nsec = 0;
    wait_until(ts);
}

void
setrootname(char* status)
{
    XStoreName(dpy, root, status);
    XSync(dpy, False);
}

void
build_status(char* status)
{
    char *pstatus = status;

    lengthLeft = maxLength;
    pstatus = load_status(pstatus);
    pstatus = separator_status(pstatus);
    pstatus = timedate_status(pstatus);
    *pstatus = 0;
}

void
update_status()
{
    char status[maxLength];

    build_status(status);
    setrootname(status);
}

void
signal_wait()
{
    pthread_mutex_lock (&mutex);
    if (!triggered)
    {
        triggered = 1;
        pthread_cond_broadcast (&condition);
    }
    pthread_mutex_unlock (&mutex);
}

void
clean_exit()
{
    must_quit = 1;
    signal_wait();
}

void
init_signals()
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_wait;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        printf("failed to set HUP signal\n");
    sa.sa_handler = clean_exit;
    if (sigaction(SIGTERM, &sa, NULL) < 0)
        printf("failed to set TERM signal\n");
    if (sigaction(SIGINT, &sa, NULL) < 0)
        printf("failed to set INT signal\n");
}

void
init_pthread()
{
    pthread_cond_init(&condition, NULL);
    pthread_mutexattr_t atts;
    pthread_mutexattr_init(&atts);
    pthread_mutexattr_setprotocol(&atts, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&mutex, &atts);
    pthread_mutexattr_destroy(&atts);
    triggered = 0;
}

int
init_x11()
{
    int screen;

    dpy = XOpenDisplay(NULL);
    if (!dpy)
        return 0;
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    return 1;
}

void
init()
{
    if (!init_x11())
    {
        fprintf(stderr, "Failed to open DISPLAY\n");
        exit(1);
    }
    load_fd = open("/proc/loadavg",  O_RDONLY);
    init_signals();
    init_pthread();
}

void
main(int argc, char *argv[])
{
    localargv = argv;
    init();
    while (!must_quit)
    {
        update_status();
        sleep_to_next();
    }
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    XCloseDisplay(dpy);
    close(load_fd);
}
