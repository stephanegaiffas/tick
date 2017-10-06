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

FLAGSETS=(
  "-O2 -funroll-loops -ftree-loop-distribution -ftree-vectorize -ftree-vectorizer-verbose=5 -ffast-math -march=native"
  "-O2 -ffast-math"
  "-O3 -ffast-math"
  "-O2 -march=native"
  "-O2 -mtune=native"
  "-O2 -march=native -mtune=native"
  "-O2 -ffast-math -march=native"
)

MKN_REPO="$(mkn -G MKN_REPO)"
MALLOCS=(
   ""
   "${MKN_REPO}/google/perf/master/inst/lib/libtcmalloc.${LIB_POSTEXT}"
   "${MKN_REPO}/mem/jemalloc/master/inst/lib/libjemalloc.${LIB_POSTEXT}"
)

cd $CWD/..
export PYTHONPATH=$PWD

DIR=$CWD/results/$(date '+%d-%m-%Y_%H-%M-%S')
mkdir -p "$DIR"

for MALLOC in "${MALLOCS[@]}"; do
    [ -n "$MALLOC" ] && [ ! -f "$MALLOC" ] \
        && echo "Malloc ${MALLOC} does not exist on this system, please install it" \
        && echo "For *nix systems this can be done by running the command 'mkn' in the directory ${CWD}" \
        && echo "Exit with error: 1" && exit 1
done

for FLAG in "${FLAGSETS[@]}"; do
    echo "COMPILER FLAGS: $CARGS $FLAG"
    rm -rf $CWD/../bin
    
    for P in "${PROFILES[@]}"; do
        LIBS=""
        IFS=$'\n'
        for j in $(mkn tree -p $P); do
            set +e
            echo "$j" | grep "+" | grep "tick" 2>&1 > /dev/null
            WIN=$?
            set -e
            if [ "$WIN" == "0" ]; then    
                D=$(echo $j | cut -d "[" -f2 | cut -d "]" -f1)
                EX=$(hash_index $D)
                ADD_LIB=${LIBRARIES[$EX]}
                LIBS="$LIBS ${ADD_LIB}.${LIB_POSTEXT}"
            fi
        done

        mkn build -p $P -tSa "${CARGS} ${FLAG}" -l "${LDARGS} ${LIBS}" -b "$PINC:$PNIC" -P lib_name="$LIB_POSTFIX",nixish_cargs=""
        EX=$(hash_index $P)
        PUSHD=${LIBRARIES[$EX]}
        pushd $(dirname ${PUSHD}) 2>&1 > /dev/null
            for f in $(find . -maxdepth 1 -type f ! -name "*.py" ! -name "__*" ); do
                SUB=${f:2:3}
                [ "$SUB" == "lib" ] && mv "$f" "_${f:5}"
            done
        popd 2>&1 > /dev/null
    done
 
    for FIL in $(find $CWD -maxdepth 1 -type f -name "*.py"); do
       echo FOUND FILE $FIL
       for MALLOC in "${MALLOCS[@]}"; do
           [ ! -z "$MALLOC" ] && export LD_PRELOAD="$MALLOC"           
           OUT="$(basename ${FIL})"
           [ ! -z "$FLAG" ]   && OUT="${OUT}_${FLAG}"_
           [ ! -z "$MALLOC" ] && OUT="${OUT}_$(basename ${MALLOC})"
           OUT=$(echo $OUT |\
               sed -e "s/ /_/g" | sed -e "s/__/_/g")
           NOW=$(date +%s%3N)
           $PY $FIL > /dev/null
           echo "$(($(date +%s%3N) - NOW))" > "${DIR}/${OUT}"
           export LD_PRELOAD=""
       done
   done
done

