#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "main.h"

void print_hello()
{
	printf("Hello World! %d\n", hello_var);

#ifdef DEBUG_FLAG
	printf("Debug print! \n");
#endif
}
