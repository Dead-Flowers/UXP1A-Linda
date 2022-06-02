#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

BASE_PATH="$SCRIPTPATH/../cmake-build-debug/src"
CLIENT_PATH="$BASE_PATH/linda-client"
HOST_PATH="$BASE_PATH/linda-server"

bash runner.sh "$HOST_PATH" "$CLIENT_PATH" 0 "./1/1.in"