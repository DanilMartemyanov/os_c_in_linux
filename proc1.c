#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

void get_time(const char *process_name, pid_t pid) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *now = localtime(&tv.tv_sec);
    printf("[%s | PID=%d] Время: %02d:%02d:%02d:%03ld\n",
           process_name, pid, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec / 1000);
}

int main() {
    pid_t pid1, pid2;

    pid1 = fork();

    if (pid1 == 0) {
        printf("\n[Дочерний процесс 1] PID=%d, Родительский PID=%d\n", getpid(), getppid());
        get_time("Дочерний процесс 1", getpid());
    } else {
        pid2 = fork();

        if (pid2 == 0) {
            printf("\n[Дочерний процесс 2] PID=%d, Родительский PID=%d\n", getpid(), getppid());
            get_time("Дочерний процесс 2", getpid());
        } else {
            printf("\n[Родительский процесс] PID=%d\n", getpid());
            get_time("Родительский процесс", getpid());
            system("ps -x");
        }
    }

    return 0;
}
