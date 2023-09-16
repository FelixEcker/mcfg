#!/bin/bash
# Build script to be used in the absence of any mariebuild version
# utilising the mcfg format.

export SRC_DIR="src/"
export OBJ_DIR="obj/"
export BINNAME="libmcfg.a"

function compile_file() {
  obj_path="${0#$SRC_DIR}"
  mkdir -p out/$(dirname $obj_path)
  echo \>\> Compiling $0
  gcc -Wall -O3 -Isrc/ -c -o out/$obj_path.o $0
}

export -f compile_file

find $SRC_DIR -name "*.c" -type f -exec bash -c "compile_file $0" {} $SRC_DIR \;

OBJS="$(find out/ -name "*.o" | tr '\n' ' ')"

echo Creating static library archive...
ar -rc $BINNAME $OBJS
ranlib $BINNAME

echo Finished! Find the library at: $BINNAME
