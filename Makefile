NAME=							otterstatus
SRC=							otterstatus.c

CC=								cc
CC_COMMAND =			$(CC) -std=c99 -Wall -O3 $(SRC) -o $(NAME) -I/usr/local/include -L/usr/local/lib -lX11

build:
	$(CC_COMMAND)

clean:
	rm otterstatus
