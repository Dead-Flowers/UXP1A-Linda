#!/bin/bash
HOST_PATH="$1"
CLIENT_PATH="$2"
shift 2
CLIENT_PIDS=""
HOST_PID=""

nohup sh -c "$HOST_PATH" & HOST_PID="$!" > /dev/null

while (( "$#" >= 2 )); do
    truncate "$2.out" --size 0
    nohup bash -c '
    while read line; do
      echo "$line" | xargs bash -c "$0" >> "$1"
      echo "Exit code: $?" >> "$1"
    done' "$CLIENT_PATH" "$2.out" < "$2" & CLIENT_PIDS="CLIENT_PIDS $!"

    sleep "$1"
    shift 2
done

wait $CLIENT_PIDS
kill -9 $HOST_PID
