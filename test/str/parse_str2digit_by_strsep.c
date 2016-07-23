#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static int parse_str2digit(char *buf, int val[])
{
	int count = 0;
	char *cur_ptr = NULL;
	char *next_ptr = buf;
	int len = strlen(buf);
	int temp;

	while(next_ptr) {
		next_ptr = skip_spaces(next_ptr);

		printf("begin: cur_ptr = [%s~%p], next_ptr = [%s~%p]\n",
			cur_ptr, cur_ptr, next_ptr, next_ptr);

		cur_ptr = strsep(&next_ptr, ",");
		if (strstr(cur_ptr, "0x")) {
			sscanf(cur_ptr, "0x%x", &temp);
			printf("val = 0x%x\n", temp);
		} else if (isdigit(*cur_ptr)) {
			sscanf(cur_ptr, "%d", &temp);
			printf("val = %d\n", temp);
		} else {
			/* it's not decimal/hexadecimal */
			printf("-------------END OF NONE(%p-%p=%ld), count = %d------------\n",
				next_ptr, buf, next_ptr - buf, count);
			break;
		}
		val[count++] = temp;

		printf("cur_ptr = [%s], next_ptr = [%s], remain len = %ld(%d)\n\n",
			cur_ptr, next_ptr, next_ptr - buf, len);

		/* End of ',' */
		if (next_ptr - buf >= len) {
			printf("-------------END OF ','(%p-%p=%ld), count = %d------------\n",
				next_ptr, buf, next_ptr - buf, count);
			break;
		}
	}

	return count;
}

int main(int argc, char *argv[])
{
	int i;
	int count;
	char *str;
	int val[32] = { 0 };

	if(argc != 2) {
		printf("usage: %s string\n", argv[0]);
		printf("e.g. %s \"0x12, 0x34,7 , 9 \"\n", argv[0]);
		return -1;
	}

	str = strdup(argv[1]);

	count = parse_str2digit(str, val);
	for(i = 0; i < count; i++)
		printf("val[%d] = %d(0x%x)\n", i, val[i], val[i]);

	free(str);

	return 0;
}

