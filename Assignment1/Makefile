CC=gcc
CFLAGS=-Wall

chan: channel.o semaphores.o share.o
	$(CC) $(CFLAGS) -o chan channel.o semaphores.o share.o -lssl -lcrypto
	
enc: enc.o semaphores.o share.o
	$(CC) $(CFLAGS) -o enc enc.o semaphores.o share.o -lssl -lcrypto
	
main: semaphores.o main.o share.o
	$(CC) $(CFLAGS) -o main semaphores.o main.o share.o

share.o: share.c
	$(CC) $(CFLAGS) -c share.c

channel.o: channel.c
	$(CC) $(CFLAGS) -c channel.c -lssl -lcrypto
	
enc.o: enc.c 
	$(CC) $(CFLAGS) -c enc.c -lssl -lcrypto
	
semaphores.o: semaphores.c
	$(CC) $(CFLAGS) -c semaphores.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean

clean:
	rm -f *.o chan main enc
