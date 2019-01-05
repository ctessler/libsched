BIN=../bin
OBJ=../obj
CFLAGS=-ggdb
LDFLAGS=-lm
export BIN OBJ CFLAGS

.PHONY: clean src test

all: test

test: bin/test-task bin/test-taskset bin/test-ordl
	bin/test-task
	bin/test-taskset
	bin/test-ordl

bin/test-task: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-task obj/test-task.o obj/task.o

bin/test-taskset: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-taskset obj/taskset.o obj/task.o \
	    obj/test-taskset.o

bin/test-ordl: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-ordl obj/test-ordl.o obj/ordl.o

src: | obj bin
src: 
	$(MAKE) -C src $(TGT)

obj:
	mkdir obj
bin:
	mkdir bin

clean:
	rm -rf obj bin
