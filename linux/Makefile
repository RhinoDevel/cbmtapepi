# RhinoDevel, Marcel Timm, 2019aug23
#
# - Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc

#CFLAGS=-c -O3 -std=gnu11 -Wall -DNDEBUG -DMT_LINUX
CFLAGS=-c -g -std=gnu11 -Wall -DMT_LINUX

LDFLAGS=-lm
SOURCES=main.c ../lib/mem/mem.c ../hardware/baregpio/baregpio.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cbmtapepi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
