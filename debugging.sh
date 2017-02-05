#!/bin/bash
INC='-Iinc -Ieint_t/inc -Iintlen/inc -Igetdigit/inc -Ito_string/inc'
LIB='-Llib -Lintlen/lib -Lgetdigit/lib -Lto_string/lib'
LL='-ltagged_memory -lto_string -lgetdigit -lintlen'
make clean && make && g++ -std=c++11 $INC $LIB -Wall -DARC64 -o bin/debugging.exec debugging.cpp $LL && clear && ./bin/debugging.exec
