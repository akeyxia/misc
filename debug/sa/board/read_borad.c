#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

enum {
	BOARD_TYPE_A0 = 1,
	BOARD_TYPE_A1,
};

/*---------------------------------------------------------------------------
 *            get_board_type()
 *---------------------------------------------------------------------------
 *
 * Synopsis: get board type from board identification file.
 *
 * Return:   int
 */
static int get_board_type(void)
{
	int ret;
	int fd;
	char buf[4] = { 0 };
	int board_type;
	char *board_type_path = "/sys/devices/board_info/board";

	ret = access(board_type_path, 0);
	if(0 != ret)
	{
		printf("Warning, board identification file(%s) does not exist", board_type_path);
		goto DEFAULT_A0;
	}

	fd = open(board_type_path, O_RDONLY);
	if(fd < 0)
	{
		printf("open board identification file(%s) failed", board_type_path);
		goto DEFAULT_A0;
	}

	ret = read(fd, buf, 4);
	if (ret < 0)
	{
		printf("read board identification file(%s) failed", board_type_path);
		goto DEFAULT_A0;
	}

	board_type = atoi(buf);

	printf("board type value: [%d]", board_type);

	return board_type;

DEFAULT_A0:
	printf("use default A0 board type");
	
	return BOARD_TYPE_A0;
}

int main(int argc, char *argv[])
{
	int board_type;

	board_type = get_board_type();

	printf("board_type = [%d]", board_type);

	return 0;
}

