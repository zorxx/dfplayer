APP = dfplayer

DFPLAYER_SRCDIR := ../../src

SRC = $(DFPLAYER_SRCDIR)/dfplayer.c main.c 

LINKFILE=
CC = gcc
CXX = g++
LD = ld

CDEFS = DEBUG_PRINT
CFLAGS = -O3 -Wall -pedantic
CFLAGS += $(foreach def,$(CDEFS),-D${def})
CFLAGS += -I$(DFPLAYER_SRCDIR)
LFLAGS = -lm -lrt -lc

all: $(APP)

$(APP): $(patsubst %.c,%.o,$(SRC))
	@echo "LD $@"
	@$(CXX) $(LFLAGS) $^ -o $@

%.o: %.cpp
	@echo "CC $^ -> $@"
	@$(CXX) -c -o $@ $(CFLAGS) $^

%.o: %.c
	@echo "CC $^ -> $@"
	@$(CC) -c -o $@ $(CFLAGS) $^

clean:
	@echo "Cleaning ${APP}"
	@rm -f *.o $(APP)

