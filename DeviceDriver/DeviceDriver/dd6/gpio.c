#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void signal_handler(int signum)
{
	printf("user app : signal is catched\n");
	if(signum==SIGIO)
	{
		printf("SIGIO\n");
		//exit(1);
	}
}

int main(int argc, char** argv)
{
	char buf[BUFSIZ];
	char i = 0;
	int fd=-1;
	int count;
	memset(buf, 0, BUFSIZ);

	signal(SIGIO, signal_handler);

	printf("GPIO Set : %s\n", argv[1]);
	fd = open("/dev/gpioled", O_RDWR);
	if(fd<0)
	{
		printf("Error : open()\n");
		return -1;
	}
	
	sprintf(buf,"%s:%d",argv[1],getpid());
	count = write(fd,buf,strlen(buf));
	if(count<0)
		printf("Error : write()\n");

	//count = read(fd, buf, strlen(argv[1]));
	//printf("Read data : %s\n", buf);
	while(1);
	close(fd);
	return 0;
}
