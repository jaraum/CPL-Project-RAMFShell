#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
sample_dir="$project_dir/sample"
selected=${1:-all}

case "$selected" in
  all) samples="1 2 3 4 5"; run_unit=true ;;
  unit) samples=""; run_unit=true ;;
  1|2|3|4|5) samples="$selected"; run_unit=false ;;
  *) echo "Usage: $0 [1|2|3|4|5|unit|all]" >&2; exit 2 ;;
esac

run_test() {
  source_file=$1
  test_name=$2
  binary=$(mktemp "${TMPDIR:-/tmp}/ramfs-shell-${test_name}.XXXXXX")
  trap 'rm -f "$binary"' EXIT HUP INT TERM
  echo "== Running ${test_name} =="
  gcc -g -std=c17 -O2 -I"$project_dir/include" -I"$sample_dir/unit" \
    "$source_file" "$project_dir/fs/ramfs.c" "$project_dir/sh/shell.c" \
    -o "$binary"
  "$binary"
  rm -f "$binary"
  trap - EXIT HUP INT TERM
}

for number in $samples; do
  run_test "$sample_dir/sample${number}.c" "sample-${number}"
done

if [ "$run_unit" = true ]; then
  for source_file in "$sample_dir"/unit/test_*.c; do
    test_name=$(basename "$source_file" .c)
    run_test "$source_file" "$test_name"
  done
fi
