# Unit Test Makefile

OBJ=../obj
BIN=../bin
OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))

.PHONY: run-test
all: run-test

$(OBJ)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BIN)/unittest: CFLAGS += -I../src
$(BIN)/unittest: LDFLAGS += -lcunit -L../lib -lsched -lm -lconfig
$(BIN)/unittest: $(OBJS) ../lib/libsched.so
	$(CC)  $(OBJS) -o $@ $(LDFLAGS)	

run-test: $(BIN)/unittest
	$(BIN)/unittest

clean:
	rm -rf $(BIN)/unittest $(OBJS)
