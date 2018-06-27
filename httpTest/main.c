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

int testKbhit(void)
{
	int	i=0;
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
}

void *gpiofunction(void *arg)
{
	int ret =0;
	return (void*)ret;
}

void *clnt_connection(void *arg)
{
	int ret =0;
	return (void*)ret;
}

int main(int argc, char *argv[])
{
	int i;
	int serv_sock;
	pthread_t thread;
	struct sockaddr_in serv_addr, clnt_addr;
	unsigned int clnt_addr_size;
	socklen_t optlen;
	int option;
	pthread_t ptGpio;

	//testKbhit();
/*
	if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
*/
	wiringPiSetup();
	
	//pthread_create(&ptGpio, NULL, gpiofunction, NULL);
	
	// 1. Socket()
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
	{
		perror("Error : socket()");
		return -1;
	}
	optlen=sizeof(option);
    option=TRUE;    
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);	
	
	// 2. bind
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);
	//serv_addr.sin_port = htons(atoi(argv[1]));
	if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1)
	{
		perror("Error:bind()");
		return -1;
	}
	
	// 3. listen
	// 최대 10대의 클라이언트의 동시접속을 처리가능하도록 큐를 생성한다.
	if(listen(serv_sock, 10)==-1)
	{
		perror("Error:listen()");
		return -1;  
	}
	
	while(1)
	{
			int clnt_sock;
			
			clnt_addr_size = sizeof(clnt_addr);
			clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
			printf("Client IP : %s:%s\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
			
			//pthread_create(&thread, NULL, clnt_connection, &clnt_sock);
	};
END:
		printf("Good Bye!\n");
			
		return 0;
}
	
	

 
