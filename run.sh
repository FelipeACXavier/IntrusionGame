# Run simulations with several parameters

ROOT=".."
LEVEL="$ROOT/levels/verified/level_12_q.json"
ITERATIONS=20
BATCHES=100
FPS=1000
CYCLES=5000
CONFIDENCE=0.95
ENTITY="guards" # or employees or attacker

# Set the parameter to be modified
CONFIG="number_of_guards"
OUT_DIR="$ROOT/data/path/$CONFIG/"

# Example, run 6 times with these parameters
for i in 5 6 7 8 9 10; do
  (
    cd build;
    ./intrusion_game \
      -c $LEVEL \
      -i $ITERATIONS \
      -b $BATCHES \
      --cycles $CYCLES \
      --fps $FPS \
      --confidence $CONFIDENCE \
      --chg-entity $ENTITY \
      --chg-param $CONFIG \
      --chg-value $i \
      --out-dir $OUT_DIR \
      --hidden
  )
done
