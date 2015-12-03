#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

/*uart_open_function*/
int open_port(int fd,int comport)
{
	char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
	long vdisable;
	
	if(comport==1)//uart1
        {
                fd=open("/dev/ttyS0",O_RDWR|O_NOCTTY|O_NDELAY);
                if(-1==fd){
                        perror("Can't Open Serial Port1!");
                        return(-1);
                }
        }
	else if(comport==2)//uart2
	{
		fd=open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NDELAY);
		if(-1==fd){
			perror("Can't Open Serial Port2!");
			return(-1);
		}
	}
	else if(comport==3)//uart1
        {
                fd=open("/dev/ttyS2",O_RDWR|O_NOCTTY);
                if(-1==fd){
                        perror("Can't Open Serial Port3!");
                        return(-1);
                }
        }
	/*return uart status is block*/
	if(fcntl(fd,F_SETFL,0)<0)
		printf("fcnl failed!\n");
	else
		printf("fcnl=%d\n",fcntl(fd,F_SETFL,0));
	/*test terminal device*/
	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");
		printf("fd-open=%d\n",fd);
	return fd;

}

/*uart_set_function*/
int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
	struct termios newtio,oldtio;
	/*save local informion of uart,if uart port error,then about it informion be appeard*/
	if(tcgetattr(fd,&oldtio)!=0){
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio,sizeof(newtio));
	/*step 1,set fontsize*/
	newtio.c_cflag |=CLOCAL | CREAD;
	newtio.c_cflag &=~CSIZE;
	/*set bit_stop*/
	switch(nBits)
	{
	case 7:
		newtio.c_cflag |=CS7;
		break;
	case 8:
		newtio.c_cflag |=CS8;
		break;
 	}
	/*set jo_check_bit*/
	switch(nEvent)
	{
	case 'O'://j
		newtio.c_cflag |=PARENB;
		newtio.c_cflag |=PARODD;
		newtio.c_iflag |=(INPCK | ISTRIP);
		break;
	case 'E'://O
		newtio.c_iflag |=(INPCK | ISTRIP);
		newtio.c_cflag |=PARENB;
		newtio.c_cflag &=~PARODD;
		break;
	case 'N'://no
		newtio.c_cflag &=~PARENB;
		break;
	}
	/*set bps*/
	switch(nSpeed)
	{
		case 2400:
			cfsetispeed(&newtio,B2400);
			cfsetospeed(&newtio,B2400);
			break;
		case 4800:
			cfsetispeed(&newtio,B4800);
            cfsetospeed(&newtio,B4800);
			break;
		case 9600:
			cfsetispeed(&newtio,B9600);
            cfsetospeed(&newtio,B9600);
			break;
		case 115200:
			cfsetispeed(&newtio,B115200);
            cfsetospeed(&newtio,B115200);
			break;
		case 460800:
			cfsetispeed(&newtio,B460800);
            cfsetospeed(&newtio,B460800);
			break;
		default:
			cfsetispeed(&newtio,B115200);
            cfsetospeed(&newtio,B115200);
			break;
	}
	/*set stop bit*/
		if(nStop==1)
			newtio.c_cflag &=~CSTOPB;
		else if(nStop==2)
			newtio.c_cflag |=CSTOPB;
	/*set waittime and mincharsize*/
		newtio.c_cc[VTIME]=0;
		newtio.c_cc[VMIN]=0;
	/*process char unrece*/
		tcflush(fd,TCIFLUSH);
	/*action newsetinfo*/
		if((tcsetattr(fd,TCSANOW,&newtio))!=0)
		{
			perror("com set error");
			return -1;
		}
		printf("set done!\n");
		return 0;
}

int main(void)
{
	int fd;
	int nwrite,i;
	char buff[]="hello\n";
	
	if((fd=open_port(fd,1))<0){
		perror("open_port error");
		return;
	}
	
	if((i=set_opt(fd,115200,8,'N',1))<0){
		perror("set_opt error");
		return;
	}
	printf("fd=%d\n",fd);

	nwrite=write(fd,buff,8);
	printf("nwrite=%d\n",nwrite);
	close(fd);
	return;
}
