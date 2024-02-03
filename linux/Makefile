# Marcel Timm, RhinoDevel, 2019aug23
#
# - Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc

#CFLAGS=-c -O3 -std=gnu11 -Wall -DNDEBUG -DMT_LINUX
CFLAGS=-c -g -std=gnu11 -Wall -DMT_LINUX

LDFLAGS=-lbcm_host

SOURCES=\
	main.c \
	\
	file/file.c \
	dma/dma.c \
	dma/clk/clk.c \
	dma/dma_gpio/dma_gpio.c \
	dma/inf/inf.c \
	dma/mbo/mbo.c \
	dma/mem/mem.c \
	dma/pwm/pwm.c \
	\
	../hardware/gpio/gpio.c \
	\
	../lib/calc/calc.c \
	../lib/console/console.c \
	../lib/alloc/alloc.c \
    ../lib/alloc/alloc_mem.c \
    ../lib/alloc/nodemem.c \
	../lib/str/str.c \
	../lib/petasc/petasc.c \
	\
	../app/tape/tape_fill_buf.c \
	../app/tape/tape_input.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cbmtapepi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
