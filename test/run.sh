#!/bin/bash

run() {
	src/imager $1 test/faustctf.ppm /tmp/a.ppm
	diff -qs /tmp/a.ppm test/faustctf_$2.ppm
}

echo "Running test files"
run "1 1" 1
run "1 2" 2
run "1 3" 3
run "1 4" 4
run "1 5" 5
run "1 6" 6
run "3 1 1 1" 111
run "3 3 3 3" 333
run "2 1 5" 15

echo "Running exploits"
make -s run-exploit-water
#make -s run-exploit-twobyte
make -s run-exploit-blur
