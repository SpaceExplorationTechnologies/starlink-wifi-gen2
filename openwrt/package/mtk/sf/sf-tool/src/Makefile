CC ?= gcc
CFLAGS ?= -Wall
OBJS = sf.o

all: sf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

sf: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f sf *.o
