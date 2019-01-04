srcs=$(wildcard *.c)
objs=$(patsubst %.c,$(OBJ)/%.o,$(srcs))

$(OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -I. -c -o $@ $<

all: $(objs)

