#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

void sigHandler(int signo)
{
    static int counter=0;
    printf("signo : %d, counter : %d\n",signo, counter);
}

int main(int argc, char *argv[])
{
	pid_t pid;
    int fd, byteCount;
    char buffer[10];

	// 시그널 핸들러 등록
    signal(SIGINT, sigHandler);

	// pid값을 읽어와서 test.txt파일에 쓰기
    pid=getpid();
    sprintf(buffer,"%d",pid);
    fd = open("./test.txt", O_RDWR | O_CREAT | O_TRUNC, \
		           S_IRWXU | S_IWGRP | S_IRGRP | S_IROTH);
    byteCount = write(fd,buffer,strlen(buffer));
    close(fd);
    
    pid = atoi(argv[1]);
    printf("send signal proc : %d\n",pid);
    kill(pid, SIGINT);
    pause();
    
    return 0;
}
