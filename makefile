BIN =../bin
OBJ =../obj
CFLAGS =-ggdb
CFLAGS += $(shell pkg-config --cflags libconfig)
LDFLAGS := -lm
LDFLAGS += $(shell pkg-config --libs libconfig)
VALGRIND := valgrind --leak-check=full --error-exitcode=1 -q

export BIN OBJ CFLAGS

.PHONY: clean src test

all: test bin/max-chunks bin/run-tpj

test: bin/test-task bin/test-taskset bin/test-ordl bin/max-chunks
	$(VALGRIND) bin/test-task
	$(VALGRIND) bin/test-taskset
	$(VALGRIND) bin/test-ordl
	$(VALGRIND) bin/max-chunks -t ex/1task.ts

bin/test-task: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-task obj/test-task.o obj/task.o

bin/test-taskset: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-taskset obj/taskset.o obj/task.o \
	    obj/test-taskset.o obj/ordl.o

bin/test-ordl: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-ordl obj/test-ordl.o obj/ordl.o

bin/max-chunks: src
	echo $(LDFLAGS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/max-chunks obj/max-chunks.o \
	    obj/taskset.o obj/task.o obj/ordl.o obj/maxchunks.o obj/taskset-config.o

bin/run-tpj: src
	echo $(LDFLAGS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/run-tpj obj/run-tpj.o \
	    obj/taskset.o obj/task.o obj/ordl.o obj/tpj.o obj/taskset-config.o

src: | obj bin
src: 
	$(MAKE) -C src $(TGT)

obj:
	mkdir obj
bin:
	mkdir bin

clean:
	rm -rf obj bin
