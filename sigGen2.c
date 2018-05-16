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
    counter++;
}

int main(int argc, char *argv[])
{
	pid_t pid;
    int fd, i, byteCount;
    char buffer[10];
    memset(buffer, 0, 10);
	
	// 시그널 핸들러 등록
    signal(SIGINT, sigHandler);

    pause();
	// test.txt파일에 pid값 읽어오기
    fd = open("./test.txt", O_RDWR);
    byteCount = read(fd,buffer,4);
    if(byteCount==0)
		printf("Can't read test.txt file.\n");

    pid = atoi(buffer);
    printf("pid : %d\n", pid);
    close(fd);
    
    kill(pid, SIGINT);
    
    for(i=0;i<4;i++)
    {
		pause();
		//sleep(1);
		kill(pid, SIGINT);
	}

    
    return 0;
}
