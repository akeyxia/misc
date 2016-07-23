#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isspace(c) ((c) == ' ')

char *skip_spaces(const char *str)
{
	while(isspace(*str))
	  ++str;

	return (char *)str;
}

/*
 * parse the string is seprated by comma character
 * e.g: 0x12, 0x34,7, 9
 */
static int parse_str2digit_basic(const char *buf, unsigned int val[])
{
	int count = 0;
	int num;
	int isxdigit;
	int temp;
	const char *str_ptr = buf;

	while(str_ptr != NULL) {
		str_ptr = skip_spaces(str_ptr);

		/* End of ' ' */
		if (str_ptr - buf >= strlen(buf)) {
			printf("-------------END OF ' '(%p-%p=%ld), count = %d------------\n",
				str_ptr, buf, str_ptr - buf, count);
			break;
		}

		if (strstr(str_ptr, "0x") != NULL)
			isxdigit = 1;
		else
			isxdigit = 0;

		printf("---> isxdigit = %d, ptr = [%s~%p]\n", isxdigit, str_ptr, str_ptr);

		if (isxdigit) {
			num = sscanf(str_ptr, "0x%x", &temp);
			val[count] = temp;
			if (num)
				printf("val = 0x%x\n", val[count]);
		/* can still add the judgement of isdigit() */
		} else {
			num = sscanf(str_ptr, "%d", &temp);
			val[count] = temp;
			if (num)
				printf("val = %d\n", val[count]);
		}
		count += num;

		printf("pos: %ld(%ld), num = %d, count = %d\n",
			str_ptr - buf, strlen(buf), num, count);

		str_ptr = strchr(str_ptr, ',');

		printf("new pos: %p-%p=%ld(%ld)\n\n",
			str_ptr, buf, str_ptr - buf, strlen(buf));

		/* End of None */
		if (str_ptr == NULL) {
			printf("-------------END OF NONE(%p-%p=0x%lx), count = %d------------\n",
				str_ptr, buf, buf - str_ptr, count);
			break;
		}

		/* End of ',' */
		if (str_ptr - buf >= (strlen(buf) - 1)) {
			printf("-------------END OF ','(%p-%p=%ld), count = %d------------\n",
				str_ptr, buf, str_ptr - buf, count);
			break;
		} else {
			/* skip ',' */
			str_ptr++;
		}
	}

	return count;
}

int main(int argc, char *argv[])
{
	int i, count;
	unsigned int val[32] = { 0 };

	if(argc != 2) {
		printf("usage: %s string\n", argv[0]);
		printf("e.g. %s \"0x12, 0x34,7 , 9 \"\n", argv[0]);
		return -1;
	}

	count = parse_str2digit_basic(argv[1], val);
	for(i = 0; i <  count; i++)
		printf("val[%d] = %d(0x%x)\n", i, val[i], val[i]);

	return 0;
}

