CFLAGS =-ggdb -Isrc
CFLAGS += $(shell pkg-config --cflags libconfig)
LDFLAGS := -L./lib -lsched -lm -lgsl -lgslcblas
LDFLAGS += $(shell pkg-config --libs libconfig)
VALGRIND := valgrind --leak-check=full --error-exitcode=1 -q
export BIN OBJ CFLAGS

BIN = bin
OBJ = obj
SRC = src
BINS = maxchunks tpj uunifast ts-gen ts-deadline-bb ts-1tom
dirs := bin obj lib

.PHONY: clean src test $(BINS) vgcheck

all: $(BINS) unittest vgcheck

test: $(BINS) bin/test-task bin/test-taskset bin/test-ordl
	$(VALGRIND) bin/test-task
	$(VALGRIND) bin/test-taskset
	$(VALGRIND) bin/test-ordl

bin/test-task: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-task obj/test-task.o obj/task.o

bin/test-taskset: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-taskset obj/taskset.o obj/task.o \
	    obj/test-taskset.o obj/ordl.o

bin/test-ordl: src
	$(CC) $(LDFLAGS) $(CFLAGS) -o bin/test-ordl obj/test-ordl.o obj/ordl.o

$(OBJ)/%.o: $(SRC)/%.c | $(dirs)
	$(CC) $(CFLAGS) -c -o $@ $<

#
# Maximum Non Preemptive Chunks implementation
#
mc_srcs = ex_maxchunks.c
mc_objs = $(patsubst %.c,$(OBJ)/%.o,$(mc_srcs))
maxchunks: bin/maxchunks
bin/maxchunks: $(mc_objs) lib/libsched.a
	$(CC) -o $@ $(mc_objs) $(LDFLAGS) $(CFLAGS)
#
# Threads Per Job implementation
#
tpj_srcs = ex_tpj.c
tpj_objs = $(patsubst %.c,$(OBJ)/%.o,$(tpj_srcs))
tpj: bin/tpj
bin/tpj: $(tpj_objs) lib/libsched.a
	$(CC) -o $@ $(tpj_objs) $(LDFLAGS) $(CFLAGS)

#
# UUNiFast
#
uuf_srcs = ex_uunifast.c
uuf_objs = $(patsubst %.c,$(OBJ)/%.o,$(uuf_srcs))
uunifast: bin/uunifast
bin/uunifast: $(uuf_objs) lib/libsched.a
	$(CC) -o $@ $(uuf_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-gen
#
tsg_srcs = ex_ts-gen.c
tsg_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsg_srcs))
ts-gen: bin/ts-gen
bin/ts-gen: $(tsg_objs) lib/libsched.a
	$(CC) -o $@ $(tsg_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-deadline-bb
#
tsd_srcs = ex_ts-deadline-bb.c
tsd_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsd_srcs))
ts-deadline-bb: bin/ts-deadline-bb
bin/ts-deadline-bb: $(tsd_objs) lib/libsched.a
	$(CC) -o $@ $(tsd_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-1tom
#
tsm_srcs = ex_ts-1tom.c
tsm_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsm_srcs))
ts-1tom: bin/ts-1tom
bin/ts-1tom: $(tsm_objs) lib/libsched.a
	$(CC) -o $@ $(tsm_objs) $(LDFLAGS) $(CFLAGS)


#
# Unit Tests
#
ut_srcs = unittest.c ut_suites.c ut_cunit.c ut_task.c ut_tpj.c
ut_objs = $(patsubst %.c,$(OBJ)/%.o,$(ut_srcs))
unittest: bin/unittest vgcheck
	$(VALGRIND) bin/unittest
bin/unittest: LDFLAGS += -lcunit
bin/unittest: $(ut_objs) lib/libsched.a
	$(CC) -o $@ $(ut_objs) $(LDFLAGS) $(CFLAGS)

#
# valgrind check the executables
#
vgcheck: $(BINS)
	$(VALGRIND) bin/maxchunks -s ex/1task.ts > /dev/null
	$(VALGRIND) bin/uunifast -s ex/uunifast.ts -u .9 > /dev/null
	$(VALGRIND) bin/ts-gen -n 5 --minp 5 --maxp 20 > /dev/null

#
# libsched library
#
lib_srcs = \
	maxchunks.c \
	ordl.c \
	task.c \
	taskset.c \
	taskset-config.c \
	taskset-create.c \
	tpj.c \
	uunifast.c
lib_objs = $(patsubst %.c,obj/%.o,$(lib_srcs))
lib/libsched.a: $(lib_objs)
	ar rcs $@ $^

$(dirs):
	mkdir $@

clean:
	rm -rf $(dirs)
