# Executable programs makefile

OBJ=../obj
BIN=../bin
CC=gcc
OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))
BINS=$(patsubst %.c,$(BIN)/%,$(wildcard *.c))

# CFLAGS += -ggdb -I../src -D_GNU_SOURCE
CFLAGS += -I../src
LDFLAGS += -L../lib -lsched -lgsl -lconfig -lc
# LDFLAGS += -L../lib -lgsl -lgslcblas -lsched -lm -lc -lconfig -lgslcblas -lgsl
# LDFLAGS += $(shell pkg-config --libs libconfig)

all: $(BINS)

$(OBJ)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BINS): $(OBJS)
	$(CC) $(OBJ)/$(@F).o -o $@ $(LDFLAGS) 

clean:
	rm -rf $(BINS) $(OBJS)
