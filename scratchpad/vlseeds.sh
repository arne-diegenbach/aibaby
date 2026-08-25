#!/bin/bash
SP=/tmp/claude-1000/-home-arne-development-aibaby/dc7627fe-bcfb-4faa-aa2c-4319cb1eaf38/scratchpad
cd /home/arne/development/aibaby
for SEED in 20360812 20451117; do
  cp dna/default.toml $SP/vl.toml; sed -i "s/^seed = .*/seed = $SEED/" $SP/vl.toml
  echo "===== seed $SEED ====="
  ./build/aibaby --dna $SP/vl.toml --experiment vocallearn --ticks 3400000 2>&1 \
    | grep -E "taught - yoked|fixed  - yoked|NOT MET|VOCAL LEARNING|UNDERPOWERED"
done
