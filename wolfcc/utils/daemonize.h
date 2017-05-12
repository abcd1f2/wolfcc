#ifndef __WOLFCC_DAEMONIZE_H__
#define __WOLFCC_DAEMONIZE_H__
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class Daemon
{
public:
    static void daemonize(void)
    {
        pid_t p = fork();
        if (p != 0) {
            exit(0);
        }

        int fd = open("/dev/null", O_RDWR, 0);
        if (-1 != fd) {
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > STDERR_FILENO) {
                close(fd);
            }
        }
    }
};

#endif
