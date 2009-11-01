

CC     = gcc
STD    = _GNU_SOURCE
CFLAGS = -DDEBUG
BIN    = cas_test lock_free_queue_test

.c.o:
	$(CC) -c -Wall -I. $(CFLAGS) -D$(STD) $<

all: $(BIN)

cas_test: cas_test.o
	$(CC) cas_test.o -o cas_test

lock_free_queue_test: lock_free_queue.o lock_free_queue_test.o
	$(CC) lock_free_queue.o lock_free_queue_test.o -o lock_free_queue_test

clean:
	rm -f *~ *.o core $(BIN)

