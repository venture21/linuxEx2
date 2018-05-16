#include<stdio.h>
#include<stdlib.h>

#define mem_size 100

int global = 10;
int bssData;

int main(void)
{
    int i=20;
    char temp[10]="test";
    char *ptr;
    ptr = malloc(mem_size);

    printf("stack: i addr = %p\n", &i);
    printf("data : global = %p\n", &global);
    printf("bss  : bssData = %p\n", &bssData);
    printf("heap : malloc = %p\n", ptr);
    printf("text : %p\n",main);

    return 0;
}

