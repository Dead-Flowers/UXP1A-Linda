#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

BASE_PATH="$SCRIPTPATH/../cmake-build-debug/src"
CLIENT_PATH="$BASE_PATH/client"
HOST_PATH="$BASE_PATH/server"

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

runTest "Something" 0 "./1/test_1"