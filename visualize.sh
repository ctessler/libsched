#!/bin/bash

GSL_RNG_SEED=`date +%s`

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-gen-nodes -n 6 -e 0.2 -o vis-01.dot
./dtask2pdf.sh vis-01.dot

(( ++GSL_RND_SEED ))
GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-demand -t vis-01.dot -w 50 -j 3 -f 0.7 -o vis-02.dot
./dtask2pdf.sh vis-02.dot

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-period -t vis-02.dot -o vis-03.dot -u 0.6
./dtask2pdf.sh vis-03.dot

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-deadline -t vis-03.dot -o vis-04.dot -b
./dtask2pdf.sh vis-04.dot

# Arbitrary order of candidates for collapse
bin/dts-cand-order -t vis-04.dot -a -o cand.list

# Collapse all the beneficial nodes
bin/dts-collapse-list -L cand.list vis-04.dot -o vis-05.dot
./dtask2pdf.sh vis-05.dot
