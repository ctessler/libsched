CFLAGS =-ggdb -Isrc
CFLAGS += $(shell pkg-config --cflags libconfig)
CFLAGS += $(shell pkg-config libgvc --cflags)
CFLAGS += -D_GNU_SOURCE -fPIC
LDFLAGS := -L./lib -lsched -lm -lgsl -lgslcblas
LDFLAGS += $(shell pkg-config --libs libconfig)
LDFLAGS += $(shell pkg-config --libs libgen)
LDFLAGS += $(shell pkg-config --libs libgvc)
VALGRIND := valgrind --leak-check=full --error-exitcode=1 -q
export BIN OBJ CFLAGS

BIN = bin
OBJ = obj
SRC = src
dirs := bin obj lib
subs := src progs test

.PHONY: clean src progs test vgcheck

all: $(subs) #vgcheck

$(OBJ)/%.o: $(SRC)/%.c | $(dirs)
	$(CC) $(CFLAGS) -c -o $@ $<

release: CFLAGS += -O3 -g0
release: $(BINS)
#
# valgrind check the executables
#
vgcheck: $(BINS) 
	$(VALGRIND) bin/maxchunks -s ex/1task.ts > /dev/null
	$(VALGRIND) bin/uunifast -s ex/uunifast.ts -u .9 > /dev/null
	$(VALGRIND) bin/ts-gen -n 5 --minp 5 --maxp 20 > /dev/null
	$(VALGRIND) bin/ts-print ex/baruah-2005.ts >/dev/null
	$(VALGRIND) bin/ts-gentp -p ex/mthreads.tp >/dev/null

$(dirs):
	mkdir $@

progs test: src
$(subs): | $(dirs)
	make -C $@ $(TGT)

clean: TGT=clean
clean: $(subs)
	rm -rf $(dirs)
	rm -rf *.aux *.dot *.pdf *.tex *.log
