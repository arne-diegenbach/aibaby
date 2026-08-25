#!/bin/bash
SP=/tmp/claude-1000/-home-arne-development-aibaby/dc7627fe-bcfb-4faa-aa2c-4319cb1eaf38/scratchpad
cd /home/arne/development/aibaby
for ARM in A B C; do
  case $ARM in
    A) D="random tract, no competition (control)";;
    B) D="exuberant + born weak, absolute pruning only (v30's refuted arm)";;
    C) D="exuberant + born weak + COMPETITIVE pruning";;
  esac
  echo "===== arm $ARM: $D ====="
  ./build/aibaby --dna $SP/ex-$ARM.toml --experiment m3probe --ticks 1600000 2>&1 \
    | sed -n '/a cube or a ball/,/^$/p' | grep -E "module|central|vision"
done
