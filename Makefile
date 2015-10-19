CC=gcc
CFLAGS=-I.
DEPS = headerinfo.h# header file 
OBJ = server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:clean

clean:
	rm server
	rm server.o