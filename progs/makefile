# Executable programs makefile

OBJ=../obj
BIN=../bin
OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))
BINS=$(patsubst %.c,$(BIN)/%,$(wildcard *.c))

CFLAGS += -I../src -D_GNU_SOURCE
LDFLAGS += -L../lib -lgsl -lgslcblas -lsched -lm -lc -lconfig -lgslcblas -lgsl
LDFLAGS += -lc --entry main
LDFLAGS += $(shell pkg-config --libs libconfig)

all: $(BINS)

$(OBJ)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BINS): $(OBJS)
	$(CC) $(OBJ)/$(@F).o -o $@ $(LDFLAGS) 

clean:
	rm -rf $(BINS) $(OBJS)
