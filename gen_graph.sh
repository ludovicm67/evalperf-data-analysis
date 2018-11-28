#!/bin/sh

if [ $# -ne 1 ]; then
  echo "please specify a file in argument" >&2
  exit 1
fi

if [ ! -f "$1" ]; then
  echo "file '$1' not found" >&2
  exit 1
fi

f_in="$1"
f_out="$(echo "$1" | sed 's/\.txt$//').png"
g_title="$(echo "$1" | sed 's/^graph_//' | sed 's/\.txt$//' | sed 's/_/ /')"

gnuplot << EOF
set grid
set xlabel "Time (in seconds)"
plot \
    "${f_in}" using 1:2 with linespoints lt 1 title "${g_title}"
set terminal png
set output "${f_out}"
replot
EOF

exit 0
