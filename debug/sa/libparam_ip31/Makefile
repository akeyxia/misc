all:arm x86
	@echo build success
	@echo run ./test.sh with sudo 

arm:./src/libparam_ip31.c ./src/ip31-write-param.c ./include/libparam_ip31.h
	 @arm-linux-gnueabihf-gcc -c ./src/ip31-write-param.c -o ./ip31-write-param.o -I./include
	 @arm-linux-gnueabihf-gcc -c ./src/libparam_ip31.c -o ./libparam_ip31.o -I./include
	 @arm-linux-gnueabihf-gcc ./ip31-write-param.o  ./libparam_ip31.o -o ip31-write-param

x86:./src/libparam_ip31.c ./src/ip31-write-param.c ./include/libparam_ip31.h
	 @gcc -c ./src/ip31-write-param.c -o ./ip31-write-param.o -I./include
	 @gcc -c ./src/libparam_ip31.c -o ./libparam_ip31.o -I./include
	 @gcc ./ip31-write-param.o  ./libparam_ip31.o -o ip31-write-param_x86

test_x86:x86
	sudo bash ./test.sh

	
clean:
	rm *.o
	rm ip31-write-param_x86 ip31-write-param


