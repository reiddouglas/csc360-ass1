#include <stdio.h>
#include "hello.h"

int main(void)
{
	int x = 5;
	int y = 3;
	int z;
	
	z = add(x,y);
	printf("X + Y = %d\n", z);
	printf("X - Y = %d\n", sub(x,y));
}