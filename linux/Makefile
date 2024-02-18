# Marcel Timm, RhinoDevel, 2019aug23
#
# - Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc

#CFLAGS=-c -O3 -std=gnu11 -Wall -DNDEBUG -DMT_LINUX
CFLAGS=-c -g -std=gnu11 -Wall -DMT_LINUX
#
# Add for "older" Raspberry OS versions:
#
# -I/opt/vc/include

LDFLAGS=-lbcm_host
#
# Add for "older" Raspberry OS versions:
#
# -L/opt/vc/lib

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
        ProgressBar/ProgressBar.c \
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
        ../lib/mem/mem.c \
        ../lib/filesys/filesys.c \
        ../lib/dir/dir.c \
        ../lib/basic/basic.c \
	\
	../app/tape/tape_fill_buf.c \
	../app/tape/tape_input.c \
	../app/petload/petload.c \
        ../app/cmd/cmd.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cbmtapepi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
