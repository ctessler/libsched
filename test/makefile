# Unit Test Makefile

OBJ?=../obj
BIN?=../bin
OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))

$(OBJ)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BIN)/unittest: CFLAGS += -I../src
$(BIN)/unittest: LDFLAGS += -lcunit -L../lib -lsched -lm -lconfig
$(BIN)/unittest: $(OBJS)
	$(CC)  $(OBJS) -o $@ $(LDFLAGS)	

clean:
	rm -rf $(BIN)/unittest $(OBJS)
