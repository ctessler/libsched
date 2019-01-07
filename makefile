BIN =../bin
OBJ =../obj
CFLAGS =-ggdb
CFLAGS += `pkg-config --cflags libconfig`
LDFLAGS := -lm
LDFLAGS += `pkg-config --libs libconfig`

export BIN OBJ CFLAGS

.PHONY: clean src test

all: test bin/max-chunks

test: bin/test-task bin/test-taskset bin/test-ordl bin/max-chunks
	bin/test-task
	bin/test-taskset
	bin/test-ordl
	valgrind --leak-check=full --error-exitcode=1 bin/max-chunks -f ex/1task.ts

bin/test-task: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-task obj/test-task.o obj/task.o

bin/test-taskset: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-taskset obj/taskset.o obj/task.o \
	    obj/test-taskset.o obj/ordl.o

bin/test-ordl: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-ordl obj/test-ordl.o obj/ordl.o

bin/max-chunks: src
	echo $(LDFLAGS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/max-chunks obj/max-chunks.o

src: | obj bin
src: 
	$(MAKE) -C src $(TGT)

obj:
	mkdir obj
bin:
	mkdir bin

clean:
	rm -rf obj bin
