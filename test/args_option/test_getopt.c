#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Test Case:
 *			./optget -a 123 -b789 -c -d
 *			./optget -c -d -a 123 -b789
 *			./optget y -x -a 123 -b789 -c -d -e
 */

int main(int argc, char *argv[])
{
	int ch;
	opterr = 1;//getopt output error message to stderr

	printf("usage: getopt -a xxx -bxxx -c -d -e\n");

	while ((ch = getopt(argc, argv, "a:b::cde")) != -1) {//':'-->args contain space, '::'-->args do not contain space
		printf("	-- optind: [%d]\n", optind - 1);//the original value is 1, optind increased by 1 when getopt() was called
		printf("	-- optarg: '%s'\n", optarg);//optarg point to the pointer of params
		printf("	-- ch: '%c'\n", ch);

		switch (ch) {
		case 'a':
			printf("option a: '%s'\n", optarg);
			break;
		case 'b':
			printf("option b: '%s'\n", optarg);
			break;
		case 'c':
			printf("option c\n");
			break;
		case 'd':
			printf("option d\n");
			break;
		case 'e':
			printf("option e\n");
			break;
		default:
			printf("warnning: nknow option: '%c' !!!\n", ch);
		}

		printf("option + '%c'/[%d]\n", optopt, optopt);//optopt contains the invalid option if the option is not in optstring
	}

	return 0;
}


