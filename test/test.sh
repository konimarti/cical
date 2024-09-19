#!/bin/sh

set -e

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
