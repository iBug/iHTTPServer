#!/bin/sh

if [ $# -eq 0 ]; then
  echo "Arguments?" >&2
  exit 1
fi

DATA_ROOT=data
EXIT=0

while [ $# -ne 0 ]; do
  file="$DATA_ROOT/$1"
  if ! [ -r "$file" ]; then
    echo "Bad file $1" >&2
    EXIT=1
    continue
  fi

  xxd -i "$file" > "$file".c
  shift
done

exit $EXIT
