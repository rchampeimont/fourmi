CFLAGS=-Wall -pedantic -ansi
CC=gcc
PROGRAMS=fourmi lire sfourmi

all: fourmi sfourmi

fourmi : fourmi.c fourmi.h
	${CC} ${CFLAGS} fourmi.c -o fourmi

lire : lire.c
	${CC} ${CFLAGS} `sdl-config --cflags` lire.c `sdl-config --libs` -lSDL_image -o lire

sfourmi : fourmi.c fourmi.h sfourmi.c sfourmi.h
	${CC} ${CFLAGS} `sdl-config --cflags` -DVERSION_SDL fourmi.c sfourmi.c `sdl-config --libs` -o sfourmi


clean:
	rm -f ${PROGRAMS} *.o

