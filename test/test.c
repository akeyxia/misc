#include <stdio.h>
#include <stdlib.h>


void foo(unsigned char c)
{
	unsigned char sc;

	sc = c;

	printf("sc = %d\n", sc);
}

struct rds_group { 
      unsigned int blockA;
       unsigned int blockB;
       unsigned int blockC;
       unsigned int blockD;
#define BLOCK_A_FIND    (0x01)
#define BLOCK_B_FIND    (0x02)
#define BLOCK_C_FIND    (0x04)
#define BLOCK_D_FIND    (0x08)
#define BLOCK_ALL_FIND  (0x0F)
        unsigned int findblock;
};

struct rds_netlink_msg{	
	int rds_num;
	int freq;
	struct rds_group group;
};

static unsigned short coefs[8] = {
	0x0001, 0x0002, 0x0003, 0x0004,
	0x0005, 0x0006, 0x0007, 0x0008,
};

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof((x)[0]))

static int scratchWrite(int size, int *pData)
{
	int i;
	int j;
	unsigned int data;

	for(i = 0; i < size; i++) {
		//data = 0;
		//for(j = 1; j >=0; j--)
		//  data |= (pData[i] & (0xFF << (j * 8))) >> (j * 8);;

		//printf("data = 0x%X\n", data);
		printf("data = 0x%X\n", *(unsigned short *)(pData + i));
	}

	return 0;
}

static int writeMem(int size, unsigned short *pData)
{
	int i;

	for(i = 0; i < 2; i++)
		scratchWrite(4, (pData + i*4));

	return 0;
}

int main(int argc, char *argv[])
{
	char c = -10;
	unsigned char uc;
	int i;
	unsigned int ui;
	unsigned int t = 10;
	double value;
	char input[256];
	char *endptr = NULL;
	unsigned short (*p_coefs)[2];

	char *ptr;
	char *str = "123";
	char buf[] = "123";
	printf("size: str = %d/%d(%d), buf = %d(%d)\n",
		sizeof(str), sizeof(ptr), strlen(str), sizeof(buf), strlen(buf));
	
	printf("short = %d, int = %d, float = %d, double = %d\n",
		sizeof(short), sizeof(int), sizeof(float), sizeof(double));

/*
	writeMem(4, coefs);

	p_coefs[0] = coefs;
	printf("%d, %d\n", ARRAY_SIZE(coefs), ARRAY_SIZE(p_coefs[0]));
*/
	#ifndef __ARM__
	printf("Not defined __ARM__");
	#pragma message("pramgma message: STR")
	//#error No defined STR
	#endif

	if (argc == 2) {
		value = atof(argv[1]);
		printf("str = %s, value = %lf\n", argv[1], value);
	}

	uc = c;
	i = c;
	ui = c;

	printf("rds msg size = [%ld%%], [%+d], [%+d], [%d]\n", sizeof(struct rds_netlink_msg), 5, -9, (t-0x10)*2);

	printf("c = %d, uc = %d, i = %d, ui = %d\n", c, uc, i, ui);
	printf("i = %d, ui = %d\n", (unsigned char)i, (unsigned char)ui);

	foo(c);
	foo(uc);

	return 0;
}

