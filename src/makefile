# Library makefile
OBJ=../obj
LIB=../lib

CFLAGS += -I. -fPIC -D_GNU_SOURCE
LDFLAGS += -lm -ldl -lgsl -lgslcblas

all: $(LIB)/libsched.so

OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))
$(OBJ)/%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(LIB)/libsched.a: $(OBJS)
	ar rcs $@ $^

$(LIB)/libsched.so: $(OBJS)
	gcc -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -rf $(OBJS)

clean:
	rm -rf $(LIB)/libsched.a $(OBJS)


