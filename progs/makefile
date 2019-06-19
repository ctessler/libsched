# Executable programs makefile

OBJ=../obj
BIN=../bin
CC=gcc
OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))
BINS=$(patsubst %.c,$(BIN)/%,$(wildcard *.c))

CFLAGS += -I../src
LDFLAGS += -L../lib -lsched -lgsl -lconfig -lc

all: $(BINS) 

$(OBJ):
	mkdir $@

$(OBJ)/%.o : %.c | $(OBJ)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BINS): $(OBJS)
	$(CC) $(OBJ)/$(@F).o -o $@ $(LDFLAGS)

clean:
	rm -rf $(BINS) $(OBJS)
