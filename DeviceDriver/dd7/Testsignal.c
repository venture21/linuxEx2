#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
int fd = -1;
char buf[BUFSIZ];

void signal_handler (int signum)                        /* 시그널 처리를 위한 핸들러 */
{
    printf("Signal is Catched!!!\n");
    if(signum == SIGUSR1) {                                     /* SIGIO이면 애플리케이션을 종료한다. */
        printf ("SIGUSR1\r\n");
        
        if(read(fd, buf, strlen(buf)) != 0)
            printf("Success : read( )\n");
            printf("Read Data : %s\n", buf);
        
        
        close(fd);
    }

    return;
}

int main(int argc, char** argv)
{

    char i = 0;

    memset(buf, 0, BUFSIZ);

    signal(SIGUSR1, signal_handler);                      /* 시그널 처리를 위한 핸들러를 등록한다. */

    printf("GPIO Set : %s\n", argv[1]);
    fd = open("/dev/gpioled", O_RDWR);
    sprintf(buf, "%s:%d", argv[1], getpid());

    write(fd, buf, strlen(buf));

    if(read(fd, buf, strlen(buf)) != 0)
        printf("Success : read( )\n");

    printf("Read Data : %s\n", buf);

    printf("My PID is %d.\n", getpid());

    while(1);                                                         /* pause(); */

    close(fd);

    return 0;
}


