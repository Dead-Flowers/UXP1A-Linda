#!/bin/bash
HOST_PATH="$1"
CLIENT_PATH="$2"
shift 2
CLIENT_PIDS=""
HOST_PID=""

bash -c "$HOST_PATH -r" >/dev/null 2>&1 &
HOST_PID="$!"

while (( "$#" >= 2 )); do
    sleep "$1"
    truncate "$2.out" --size 0
    bash -c '
    cat "$1" | while read line;
    do
      echo "$line";
      bash -c "$0 $line" >> "$1.out";
      echo "Exit code: $?" >> "$1.out";
    done' "$CLIENT_PATH" "$2" >/dev/null 2>&1 &
    CLIENT_PIDS="$CLIENT_PIDS $!"
    shift 2
done

wait $CLIENT_PIDS
kill "$HOST_PID"
echo "Done"