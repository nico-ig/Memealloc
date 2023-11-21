
ifeq ($(MAKECMDGOALS),)
	TARGET = avaliacao
else
	TARGET = $(firstword $(MAKECMDGOALS))
endif

CC = gcc
CFLAGS = -g -Wall -Wextra

ifeq ($(filter clean purge,$(MAKECMDGOALS)),)

$(TARGET): $(TARGET).o memalloc.o
	$(CC) $(CFLAGS) $^ -o $@ -fPIE

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c $^ -o $@ -fPIC

memalloc.o: memalloc.s
	$(CC) $(CFLAGS) -c $^ -o $@ -fPIC

endif

clean:
		@rm -f *.o vgcore*

purge: clean
		@rm -f $(TARGET) exemplo test avaliacao


