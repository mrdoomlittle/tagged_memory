#!/bin/sh

if [ "$1" = "false" ]; then
	sh compile.sh benchmark.cpp bin/benchmark.exec && ./bin/benchmark.exec
else
	sh compile.sh benchmark.cpp bin/benchmark.exec && clear && ./bin/benchmark.exec
fi

