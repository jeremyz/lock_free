

CC     = gcc
STD    = _GNU_SOURCE
CFLAGS = -DDEBUG
BIN    = cas_test lock_free_queue_test lf_fifo_test

.c.o:
	$(CC) -c -Wall -I. $(CFLAGS) -D$(STD) $<

all: $(BIN)

cas_test: cas_test.o
	$(CC) cas_test.o -o cas_test

lock_free_queue_test: lock_free_queue.o lock_free_queue_test.o
	$(CC) lock_free_queue.o lock_free_queue_test.o -o lock_free_queue_test

lf_fifo.o: lf_fifo.h lf_fifo_cas.h

lf_fifo_test: lf_fifo.o lf_fifo_test.o
	$(CC) lf_fifo.o lf_fifo_test.o -o lf_fifo_test

as:
	$(CC) -S lf_fifo.c

clean:
	rm -f *~ *.o *.s core $(BIN)
