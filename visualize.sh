#!/bin/bash

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` bin/dts-gen-nodes -n 6 -e 0.2 -o vis.dot
dot2tex -tmath -p --autosize vis.dot > vis.tex
pdflatex vis.tex

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-demand -t vis.dot -w 50 -j 3 -f 0.7 -o vis-demand.dot
dot2tex -tmath -p --autosize vis-demand.dot > vis-demand.tex
pdflatex vis-demand.tex

