CC=msp430-gcc
MCU=msp430g2231
CFLAGS=-Wall -Werror -Os -mmcu=$(MCU)

all:	install

rgb:	init
	$(CC) $(CFLAGS) -o build/rgb.elf rgb.c uart.c

init:
	mkdir -p build

dco:	init
	$(CC) $(CFLAGS) -o build/dco.elf dcocal.c

install: rgb
	sudo mspdebug rf2500 -q 'prog build/rgb.elf'

read:
	sudo stty -F /dev/ttyACM0 9600 raw
	cat /dev/ttyACM0

clean:
	rm -rf build
