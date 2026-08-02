#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
sample_dir="$project_dir/sample"
selected=${1:-all}

case "$selected" in
  all) samples="1 2 3 4 5" ;;
  1|2|3|4|5) samples="$selected" ;;
  *) echo "Usage: $0 [1|2|3|4|5|all]" >&2; exit 2 ;;
esac

for number in $samples; do
  binary=$(mktemp "${TMPDIR:-/tmp}/ramfs-shell-sample-${number}.XXXXXX")
  trap 'rm -f "$binary"' EXIT HUP INT TERM
  echo "== Running sample ${number} =="
  gcc -g -std=c17 -O2 -I"$project_dir/include" \
    "$sample_dir/sample${number}.c" "$project_dir/fs/ramfs.c" "$project_dir/sh/shell.c" \
    -o "$binary"
  "$binary"
  rm -f "$binary"
  trap - EXIT HUP INT TERM
done
