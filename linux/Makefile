# Marcel Timm, RhinoDevel, 2019aug23
#
# - Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc

#CFLAGS=-c -O3 -std=gnu11 -Wall -DNDEBUG -DMT_LINUX
CFLAGS=-c -g -std=gnu11 -Wall -DMT_LINUX

LDFLAGS=-lm
SOURCES=\
	main.c \
	../lib/mem/mem.c \
	../lib/alloc/alloc.c \
	../lib/alloc/alloc_mem.c \
	../lib/alloc/nodemem.c \
	../lib/str/str.c \
	../lib/calc/calc.c \
	../lib/console/console.c \
	../hardware/gpio/gpio.c \
	../app/tape/tape_extract_buf.c \
	../app/tape/tape_fill_buf.c \
	../app/tape/tape_init.c \
	../app/tape/tape_receive_buf.c \
	../app/tape/tape_receive.c \
	../app/tape/tape_send_buf.c \
	../app/tape/tape_send.c \

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cbmtapepi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
