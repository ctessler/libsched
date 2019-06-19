# Library makefile
OBJ=../objlib
LIB=../lib

CFLAGS += -I. -fPIC -D_GNU_SOURCE
CFLAGS += $(shell pkg-config libgvc --cflags)
LDFLAGS += -lm -ldl -lgsl -lgslcblas
LDFLAGS += $(shell pkg-config libgvc --libs)

all: $(LIB)/libsched.so

$(OBJ):
	mkdir $@

OBJS=$(patsubst %.c,$(OBJ)/%.o,$(wildcard *.c))
$(OBJ)/%.o : %.c  | $(OBJ)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(LIB)/libsched.a: $(OBJS) 
	ar rcs $@ $^

$(LIB)/libsched.so: $(OBJS)
	gcc -shared -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(LIB)/libsched.a $(OBJS) $(OBJ)


