#include <stdio.h> 
#define MAX 15 

int main(void)
{
    float f1,f2,f3;

    while (1)
    {
        char buf[MAX]; 
        fgets(buf, MAX, stdin); 
        printf("string is: %s\n", buf); 
    }

    return 0;
}