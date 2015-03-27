#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM	(10)
#define STR	"abc"
int value;

void func(int val)
{
	value = val;
#undef NUM
#define NUM	value
}

int User_GetTime()
{       
	    struct timeval tv;
		    int  dwTime;
			        
			    gettimeofday(&tv, NULL);
				        
				    dwTime = ((tv.tv_sec % 1000) * 1000000) + (tv.tv_usec % 1000000);
					            
					    return dwTime;
}

void define_foo(int val)
{
	int value = val;

#undef NUM
#define NUM value
	printf("define NUM: %d\n", NUM);

}

int main(int argc, char *argv[])
{
	printf("%d\n", NUM);
	printf("%d\n", NUM);
	//func(100);
	printf("%d\n", NUM);
	//func(1000);
	printf("%d\n", NUM);

	printf("time: %d\n", User_GetTime());
	sleep(1);
	printf("time: %d\n", User_GetTime());

	printf("angle = %f\n", atanf((125.80f - 96.90f) / (55.90f - 49.75f)));

	printf("define NUM: %d\n", NUM);
#undef NUM
#define NUM 5
	printf("define NUM: %d\n", NUM);
	define_foo(atoi(argv[1]));

	return 0;
}

