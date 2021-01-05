CC=gcc
CFLAGS=-Wall

	
assign2: second_chance.o lru.o hashtable.o simulator.o
	$(CC) $(CFLAGS) -o assign2 second_chance.o lru.o hashtable.o simulator.o -lm

second_chance.o: second_chance.c
	$(CC) $(CFLAGS) -c second_chance.c
	
lru.o: lru.c 
	$(CC) $(CFLAGS) -c lru.c
	
hashtable.o: hashtable.c
	$(CC) $(CFLAGS) -c hashtable.c

simulator.o: simulator.c
	$(CC) $(CFLAGS) -c simulator.c -lm

.PHONY: clean

clean:
	rm -f *.o assign2
