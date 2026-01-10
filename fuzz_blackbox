#!/bin/sh
tmp=test_fuzz.out
i=0
while [ $i -lt 1000 ]; do

	head -c 200 /dev/urandom | tr -dc '()\n\t".;#+-a-z0-9A-Z ' > $tmp;
	./scheme >/dev/null 2>/dev/null < $tmp
	rc=$?

	if [ $rc -ne 0 ] && [ $rc -ne 1 ]; then
		exit $rc
	fi

	i=$((i+1))
done
