#!/bin/bash
SP=/tmp/claude-1000/-home-arne-development-aibaby/dc7627fe-bcfb-4faa-aa2c-4319cb1eaf38/scratchpad
cd /home/arne/development/aibaby
for SEED in 20360812 20451117; do
  cp dna/default.toml $SP/ts.toml; sed -i "s/^seed = .*/seed = $SEED/" $SP/ts.toml
  echo "===== seed $SEED ====="
  ./build/aibaby --dna $SP/ts.toml --experiment teachsound --ticks 3400000 --wav $SP/teach-$SEED 2>&1 \
    | grep -E "^    taught|^    yoked|^    F2|error moved|audible change|taught d'|yoked  d'|TAUGHT|NOT "
done
