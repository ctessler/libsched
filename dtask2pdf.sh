#!/bin/bash
tex=${1%.*}.tex
pdf=${1%.*}.pdf
dot2tex -tmath -p --autosize $1 > $tex
latexmk -pdf $tex
latexmk -c $tex
