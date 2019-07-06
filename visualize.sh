#!/bin/bash

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` bin/dts-gen-nodes -n 6 -e 0.2 -o vis-01.dot
dot2tex -tmath -p --autosize vis-01.dot > vis-01.tex
pdflatex vis-01.tex

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
    bin/dts-demand -t vis-01.dot -w 50 -j 3 -f 0.7 -o vis-02.dot
dot2tex -tmath -p --autosize vis-02.dot > vis-02.tex
pdflatex vis-02.tex

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
	    bin/dts-period -t vis-02.dot -o vis-03.dot -u 0.6
dot2tex -tmath -p --autosize vis-03.dot > vis-03.tex
pdflatex vis-03.tex

GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%s` \
	    bin/dts-deadline -t vis-03.dot -o vis-04.dot -b
dot2tex -tmath -p --autosize vis-04.dot > vis-04.tex
pdflatex vis-04.tex
