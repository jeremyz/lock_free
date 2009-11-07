

CC     = gcc
STD    = _GNU_SOURCE
CFLAGS = -DDEBUG
BIN    = cas container_of lock_free_queue_test lf_fifo

.c.o:
	$(CC) -c -Wall -I. $(CFLAGS) -D$(STD) $<

all: $(BIN)

cas: cas.o
	$(CC) -S cas.c
	$(CC) cas.o -o cas

container_of: container_of.o
	$(CC) -S container_of.c
	$(CC) container_of.o -o container_of

lock_free_queue_test: lock_free_queue.o lock_free_queue_test.o
	$(CC) lock_free_queue.o lock_free_queue_test.o -o lock_free_queue_test



lf_fifo.o: lf_fifo.h lf_cas.h

lf_fifo_test: lf_fifo.o lf_fifo_test.o
	$(CC) lf_fifo.o lf_fifo_test.o -o lf_fifo_test

clean:
	rm -f *~ *.o *.s core $(BIN)
