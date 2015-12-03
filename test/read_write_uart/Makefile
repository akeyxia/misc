all: read_uart write_uart

read_uart: read_uart.c
	arm-linux-gnueabihf-gcc read_uart.c -o read_uart

write_uart: write_uart.c
	arm-linux-gnueabihf-gcc write_uart.c -o write_uart

clean:
	rm -rf read_uart write_uart

