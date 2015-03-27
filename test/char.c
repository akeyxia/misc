#include <stdio.h>
#include <stdlib.h>

#define RAW(fmt, arg...) \
	printf(fmt, ## arg)
#define DBG(fmt, arg...) printf("dbg %s: " fmt "\r", __FUNCTION__, ## arg)

#define _NOSTRPRT(_n) case _n: printf("%s\n", (# _n)); break

typedef enum{
	PLAYER_STATE_NONE = 0x00,
	PLAYER_STATE_STOP = 0x30,
	PLAYER_STATE_PAUSE = 0x40,
	PLAYER_STATE_PLAY = 0x50,
	PLAYER_STATE_SEARCH = 0x60,
	PLAYER_STATE_RECORDING = 0x70,
	/* Power up, release reset, initialization */
	PLAYER_STATE_TRANSTION = 0x80,
}player_state_values_t;

typedef struct player_state{
	unsigned char state;
}__attribute((packed)) player_state_t;

int main(int argc, char *argv[])
{
	char buf[] = {0x52,0x25,0xc2,0x8d,0xc3,0x81,0x20,0x34,0x28,0x74,0x4e,0x20,0x01,0xc3,0xbc,0x20};
	int len = sizeof(buf);
	int i;
	player_state_values_t state = PLAYER_STATE_PLAY;

	DBG("Start");
	RAW("<<");
	for(i = 0; i < len; i++)
		RAW("%c ", buf[i]);
	RAW(">>\n");
	DBG("End");

	switch(state) {
	_NOSTRPRT(PLAYER_STATE_PLAY);
	default:
		break;
	}

	return 0;
}

