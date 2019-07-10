#!/bin/bash

NODES=${nodes:-10}
EDGEP=${edgep:-0.2}
WCET1=${wcet1:-50}
OBJS=${objs:-3}
GROWF=${growf:-0.7}
UTIL=${util:-0.6}

GSL_RNG_TYPE=ranlxs2
GSL_RNG_SEED=`date +%s`

name=${1:-dtask-${GSL_RNG_SEED}}
name=${name%.*}

(( ++GSL_RNG_SEED ))
bin/dts-gen-nodes -n ${NODES} -e ${EDGEP} -o ${name}-01.dot

(( ++GSL_RND_SEED ))
bin/dts-demand -t ${name}-01.dot -w ${WCET1} -j ${OBJS} -f ${GROWF}\
	       -o ${name}-02.dot

(( ++GSL_RND_SEED ))
bin/dts-period -t ${name}-02.dot -u ${UTIL} -o ${name}-03.dot

(( ++GSL_RND_SEED ))
bin/dts-deadline -t ${name}-03.dot -b -o ${name}.dot

rm ${name}-01.dot ${name}-02.dot ${name}-03.dot 
