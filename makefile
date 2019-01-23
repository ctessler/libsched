CFLAGS =-ggdb -Isrc
CFLAGS += $(shell pkg-config --cflags libconfig)
LDFLAGS := -L./lib -lsched -lm -lgsl -lgslcblas
LDFLAGS += $(shell pkg-config --libs libconfig)
VALGRIND := valgrind --leak-check=full --error-exitcode=1 -q
export BIN OBJ CFLAGS

BIN = bin
OBJ = obj
SRC = src
BINS = maxchunks tpj uunifast \
	ts-gen ts-deadline-bb ts-gf ts-print ts-divide ts-merge
dirs := bin obj lib

.PHONY: clean src test $(BINS) vgcheck

all: $(BINS) unittest vgcheck

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
# ts-gf
#
tsgf_srcs = ex_ts-gf.c
tsgf_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsgf_srcs))
ts-gf: bin/ts-gf
bin/ts-gf: $(tsgf_objs) lib/libsched.a
	$(CC) -o $@ $(tsgf_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-print
#
tsp_srcs = ex_ts-print.c
tsp_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsp_srcs))
ts-print: bin/ts-print
bin/ts-print: $(tsp_objs) lib/libsched.a
	$(CC) -o $@ $(tsp_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-divide
#
tsv_srcs = ex_ts-divide.c
tsv_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsv_srcs))
ts-divide: bin/ts-divide
bin/ts-divide: $(tsv_objs) lib/libsched.a
	$(CC) -o $@ $(tsv_objs) $(LDFLAGS) $(CFLAGS)

#
# ts-merge
#
tsm_srcs = ex_ts-merge.c
tsm_objs = $(patsubst %.c,$(OBJ)/%.o,$(tsm_srcs))
ts-merge: bin/ts-merge
bin/ts-merge: $(tsm_objs) lib/libsched.a
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
	$(VALGRIND) bin/ts-print ex/baruah-2005.ts >/dev/null

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
