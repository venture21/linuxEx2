#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define	PORT		8080


int kbhit()
{
	// 터미널에 대한 구조체
	struct termios oldt, newt;
	int ch, oldf;
	
	// 현재 터미널에 설정된 정보를 받아온다.
	tcgetattr(0, &oldt);	
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);	//정규 모드입력과 에코를 해제한다.
	tcsetattr(0, TCSANOW, &newt);		// 현재 터미널에 변경된 설정값을 적용한다.
	oldf = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, oldf | O_NONBLOCK); //Non-blocking모드로 설정한다.
	
	ch = getchar();
	
	tcsetattr(0, TCSANOW, &oldt);		//기존의 값으로 터미널 속성을 바로 변경한다.
	
	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

int main(void)
{
	int i;
	int serv_sock;
	pthread_t thread;
	struct sockaddr_in serv_addr, clnt_addr;
	unsigned int clnt_addr_size;
	
	pthread_t ptGpio;
	
	wiringPiSetup();
	/*
	pthread_create(&ptGpio, NULL, gpiofunction, NULL);
	
	// 1. Socket()
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
	{
		perror("Error : socket()");
		return -1;
	}
	
	// 2. bind
	memset(&serv_add, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);
	if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1)
	{
		perror("Error:bind()");
		return -1;
	}
	*/
	i=0;
	while(1)
	{
		if(kbhit())
		{
			switch(getchar())
			{
				case 'q':
					goto END;
					break;
			};
			i++;
			printf("%20d\t\t\r", i);
			usleep(100);
		}
	}

END:
		printf("Good Bye!\n");
		return 0;
}
	
	

 
