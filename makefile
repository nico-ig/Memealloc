TARGET ?= memalloc

CC = gcc
CFLAGS = -g -Wall -Wextra --no-pie

exemplo: exemplo.o memalloc.o
	$(CC) $(CFLAGS) $^ -o $@

exemplo.o: exemplo.c
	$(CC) $(CFLAGS) -c $^ -o $@

memalloc.o: memalloc.s
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
		@rm -f *.o vgcore*

purge: clean
		@rm -f $(TARGET) exemplo


