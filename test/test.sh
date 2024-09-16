#!/bin/sh

set -e

here=$(dirname $0)
fail=0

tool="cical"
tool_opts="-j"

do_test() {
	tool_bin="$1"
	vec="$2"
	expected="$3"
	tmp=$(mktemp)
	status=0
	$tool_bin $tool_opts < $vec > $tmp 2> /dev/null || status=$?
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
	tool_bin=$here/../$tool
	if ! [ -f "$tool_bin" ]; then
		tool_bin=$here/$tool
	fi
	do_test "$tool_bin" "$vec" "$expected"
done

exit $fail
