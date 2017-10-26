#!/usr/bin/env bash

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $CWD/.. 
ROOT=$PWD
source $ROOT/sh/configure_env.sh

cd $CWD

CARGS="$(mkn -G nixish_cargs -C ..)"
CARGS=$(echo "$CARGS" | sed -e "s/ -ffast-math//g")
CARGS=$(echo "$CARGS" | sed -e "s/ -O2//g")
CARGS=$(echo "$CARGS" | sed -e "s/ -O3//g" )
CARGS=$(echo "$CARGS" | sed -e "s/ -g//g" )
[ ! -z "$CXXFLAGS" ] && CARGS="$CXXFLAGS $CARGS"

COMPILERS=(
  "g++6.3.0"
  "g++7.2.0"
  "clang5"
  "icc-2018"
)

FLAGSETS=(
  "-O3 -ffast-math"
  "-O3 -funroll-loops -march=native -ffast-math"
)

cd $ROOT/benchmark

MKN_REPO="$(mkn -G MKN_REPO)"

MALLOCS=(
   ""
   "${MKN_REPO}/google/perf/master/inst/lib/libtcmalloc.${LIB_POSTEXT}"
   "${MKN_REPO}/mem/jemalloc/master/inst/lib/libjemalloc.${LIB_POSTEXT}"
)

DIR=$ROOT/benchmark/results/$(date '+%d-%m-%Y_%H-%M-%S')
mkdir -p "$DIR"

for MALLOC in "${MALLOCS[@]}"; do
  [ -n "$MALLOC" ] && [ ! -f "$MALLOC" ] \
    && echo "Malloc ${MALLOC} does not exist on this system, please install it" \
    && echo "For *nix systems this can be done by running the command 'mkn' in the directory ${CWD}" \
    && echo "Exit with error: 1" && exit 1
done

cd $ROOT
export PYTHONPATH=$PWD

for COMPILER in "${COMPILERS[@]}"; do
  export MKN_X_FILE="$COMPILER"
  KLOG=3 RESWIG=1 ./mkn.sh  array
done

for COMPILER in "${COMPILERS[@]}"; do

  export MKN_X_FILE="$COMPILER"

  for FLAG in "${FLAGSETS[@]}"; do
    echo "COMPILER FLAGS: $CARGS $FLAG"
    rm -rf $ROOT/bin

    CXXFLAGS="$CARGS $FLAG" ./mkn.sh

    for FIL in $(find $ROOT/benchmark -maxdepth 1 -type f -name "*.py"); do
      echo FOUND FILE $FIL
      for MALLOC in "${MALLOCS[@]}"; do
        echo runnin malloc $MALLOC
        [ ! -z "$MALLOC" ] && export LD_PRELOAD="$MALLOC"           
        OUT="${COMPILER}_$(basename ${FIL})"
        [ ! -z "$FLAG" ]   && OUT="${OUT}_${FLAG}"_
        [ ! -z "$MALLOC" ] && OUT="${OUT}_$(basename ${MALLOC})"
        OUT=$(echo $OUT |\
          sed -e "s/ /_/g" | sed -e "s/__/_/g")
        NOW=$(date +%s%3N)
        OUT=$(echo $OUT | sed -e "s/.so$//g")
        OUT=$(echo $OUT | sed -e "s/_$//g")
        for i in $(seq 1 5); do
          $PY $FIL > /dev/null
        done
        echo "$(($(date +%s%3N) - NOW))" > "${DIR}/${OUT}"
        export LD_PRELOAD=""
      done
    done
  done
done

export MKN_X_FILE=""