#!/bin/sh

set -e

usage() { echo "Usage: $0 [-g]" 1>&2; exit 1; }

generate=0
while getopts "g" o; do
    case "${o}" in
        g)
            generate=1
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

here=$(dirname $0)
fail=0

tool="cical"

do_test() {
	prefix="$1"
	tool_bin="$2"
	tool_opts="$3"
	vec="$4"
	expected="$5"
	tmp=$(mktemp)
	status=0
	if [ "$generate" -eq 1 ]; then
		$prefix $tool_bin $tool_opts < $vec > $expected
		return
	fi
	$prefix $tool_bin $tool_opts < $vec > $tmp || status=$?
	if [ $status -eq 0 ] && diff -u "$expected" "$tmp"; then
		echo "ok      $tool < $vec > $tmp"
	else
		echo "error   $tool < $vec > $tmp [status=$status]"
		fail=1
	fi
	rm -f -- "$tmp"
}

for vec in $here/vectors/*.in; do
	expected=${vec%%.in}.expected
	tool_bin=$here/../debug/$tool
	tool_opts=-$(basename $vec | cut -d '.' -f 2)
	prefix="$TEST_PREFIX"
	if ! [ -f "$tool_bin" ]; then
		tool_bin=$here/$tool
	fi
	do_test "$prefix" "$tool_bin" "$tool_opts" "$vec" "$expected"
done

exit $fail
