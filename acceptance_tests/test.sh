#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

BASE_PATH="$SCRIPTPATH/../cmake-build-debug/src"
CLIENT_PATH="$BASE_PATH/linda-client"
HOST_PATH="$BASE_PATH/linda-server"

runTest () {
  echo "Running test $1"
  shift
  bash runner.sh "$HOST_PATH" "$CLIENT_PATH" "$@"
  while (( "$#" >= 2 )); do
    if [[ -f "$2.expected" ]]
    then
      diff -s "$2.expected" "$2.out" || exit 1
    else
      echo "Creating expected file"
      cp "$2.out" "$2.expected"
    fi
    shift 2
  done
  echo ""
}

runTest "Input integer" 0 "./1/test_1"
runTest "Read float" 0 "./2/test_2"
runTest "Read multiple tuples" 0 "./3/test_3"
runTest "Read after input" 0 "./4/test_4"
runTest "Consume with string pattern" 0 "./5/test_5"
runTest "Too big tuples" 0 "./6/test_6"
runTest "Multiple clients test" 0 "./7/test_7a" 1 "./7/test_7b" 1 "./7/test_7c" 1 "./7/test_7d" 1 "./7/test_7e" 1 "./7/test_7f"
