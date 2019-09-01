#include<stdio.h>
int main()
{
    short c[2] = {1, 1};
    short* p1 = c;
    int * p2 = (int*)p1;
    
    printf("取出p1: %d\n", *p1);
    printf("取出p2: %d\n", *p2);
    return 0;
}
