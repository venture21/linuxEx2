#include<signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int s, sig;
    if(argc !=3 || strcmp(argv[1], "--help")==0)
	printf("%s pid sig-num\n", argv[0]);
    sig = atoi(argv[2]);
    printf("argv[1]=%d\n",sig);
    s = kill(atoi(argv[1]),sig);
    //s = kill(-1,sig);
    // 파라메터 입력이 정상적이지 않은 경우
    if(sig!=0)
    {
		// 정상적으로 kill함수가 호출되지 않은 경우
		if(s==-1)
			printf("Error:system call kill()\n");
    } else 
    {
		// 정상적으로 kill함수가 호출된 경우
		if(s==0)
			printf("Process exists and we can send it a signal\n");
    }
    return 0;
}


