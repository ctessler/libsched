BIN=../bin
OBJ=../obj
CFLAGS=-ggdb
export BIN OBJ CFLAGS

.PHONY: clean src test

all: bin/test-task bin/test-task-set

bin/test-task: src
	$(CC) $(CFLAGS) -o bin/test-task obj/test-task.o obj/task.o

bin/test-task-set: src
	$(CC) $(CFLAGS) -o bin/test-taskset obj/taskset.o obj/task.o \
	    obj/test-taskset.o

src: | obj bin
src: 
	$(MAKE) -C src $(TGT)

obj:
	mkdir obj
bin:
	mkdir bin

clean:
	rm -rf obj bin
