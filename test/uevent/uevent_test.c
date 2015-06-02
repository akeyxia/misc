#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define UEVENT_MSG_LEN 4096
#define NETLINK_CAN     21
#define NETLINK_TBOX    22

struct luther_gliethttp {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *firmware;
	const char *name;
	int value;
    int major;
    int minor;
};

static int open_luther_gliethttp_socket(void);
static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp);
int netlink_type = NETLINK_CAN;

int main(int argc, char* argv[])
{
    int device_fd = -1;
    char msg[UEVENT_MSG_LEN+2];
    int n;
	int cmd = atoi(argv[1]);

	if (cmd == 0) {
		netlink_type = NETLINK_KOBJECT_UEVENT;
		printf("NETLINK_KOBJECT_UEVENT\n");
	} else if(cmd == 1) {
		netlink_type = NETLINK_CAN;
		printf("NETLINK_CAN\n");
	} else if(cmd == 2) {
		netlink_type = NETLINK_TBOX;
		printf("NETLINK_TBOX\n");
	}
    
    device_fd = open_luther_gliethttp_socket();
    printf("device_fd = %d\n", device_fd);
    do {
        while((n = recv(device_fd, msg, UEVENT_MSG_LEN, 0)) > 0) {
            struct luther_gliethttp luther_gliethttp;
            if(n == UEVENT_MSG_LEN) /* overflow -- discard */
                continue;
            msg[n] = '\0';
            msg[n+1] = '\0';
            parse_event(msg, &luther_gliethttp);
        }
    } while(1);

	return 0;
}

static int open_luther_gliethttp_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 64*1024;
    int s;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;
    s = socket(PF_NETLINK, SOCK_DGRAM, netlink_type);
    //if (s0)
      //  return -1;
    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)),0) {
        close(s);
        return -1;
    }

    return s;
}

static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp)
{
    luther_gliethttp->action = "";
    luther_gliethttp->path = "";
    luther_gliethttp->subsystem = "";
    luther_gliethttp->firmware = "";
    luther_gliethttp->name = "";
    luther_gliethttp->major = -1;
    luther_gliethttp->minor = -1;
    luther_gliethttp->value = 0;
    /* currently ignoring SEQNUM */

	printf("========================================================\n");
    while (*msg) {
        printf("%s\n", msg);
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            luther_gliethttp->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            luther_gliethttp->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            luther_gliethttp->subsystem = msg;
        } else if (!strncmp(msg, "FIRMWARE=", 9)) {
            msg += 9;
            luther_gliethttp->firmware = msg;
        } else if (!strncmp(msg, "MAJOR=", 6)) {
            msg += 6;
            luther_gliethttp->major = atoi(msg);
        } else if (!strncmp(msg, "MINOR=", 6)) {
            msg += 6;
            luther_gliethttp->minor = atoi(msg);
        } else if (!strncmp(msg, "CIS_CAN_NAME=", 13)) {
            msg += 13;
            luther_gliethttp->name = msg;
        } else if (!strncmp(msg, "CIS_CAN_VALUE=", 14)) {
            msg += 14;
            luther_gliethttp->value = atoi(msg);
        }
        /* advance to after the next \0 */
        while(*msg++)
            ;
    }
}

