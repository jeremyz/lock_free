

CC     = gcc
STD    = _GNU_SOURCE
CFLAGS = -DDEBUG
BIN    = cas container_of lock_free_queue_test lf_fifo_test lf_ring_buffer_test

.c.o:
	$(CC) -march=i686 -O2 -c -Wall -I. $(CFLAGS) -D$(STD) $<

all: $(BIN)

cas.o: cas.c
cas: cas.o
	$(CC) -S cas.c
	$(CC) cas.o -o cas

container_of.o: container_of.c
container_of: container_of.o
	$(CC) -S container_of.c
	$(CC) container_of.o -o container_of

lock_free_queue.o: lock_free_queue.h lock_free_queue.c
lock_free_queue_test.o: lock_free_queue_test.c
lock_free_queue_test: lock_free_queue.o lock_free_queue_test.o
	$(CC) lock_free_queue.o lock_free_queue_test.o -o lock_free_queue_test

lf_fifo.o: lf_fifo.h lf_fifo.c lf_cas.h
lf_fifo_test.o: lf_fifo_test.c
lf_fifo_test: lf_fifo.o lf_fifo_test.o
	$(CC) lf_fifo.o lf_fifo_test.o -o lf_fifo_test

lf_ring_buffer.o: lf_ring_buffer.h lf_ring_buffer.c lf_portable_cas.h
lf_ring_buffer_test.o: lf_ring_buffer_test.c
lf_ring_buffer_test: lf_ring_buffer.o lf_ring_buffer_test.o
	$(CC) -lrt lf_ring_buffer.o lf_ring_buffer_test.o -o lf_ring_buffer_test

clean:
	rm -f *~ *.o *.s core $(BIN)
