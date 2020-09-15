CC=gcc
CFLAGS= -O3 -Wall -pedantic

all: main

main: image.o game.o player.o strprint.o main.o util.o map.o sound.o minigame.o
	$(CC) $(CFLAGS) -o main *.o -pthread

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

game.o: game.c game.h
	$(CC) $(CFLAGS) -c game.c -pthread

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

player.o: player.c player.h
	$(CC) $(CFLAGS) -c player.c

strprint.o: strprint.c strprint.h
	$(CC) $(CFLAGS) -c strprint.c -pthread

minigame.o: minigame.c minigame.h
	$(CC) $(CFLAGS) -c minigame.c -pthread

map.o: map.c map.h
	$(CC) $(CFLAGS) -c map.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

sound.o: sound.c sound.h
	$(CC) $(CFLAGS) -c sound.c

clean:
	rm -rf *.o *.dSYM
	rm -rf main
