#!/bin/bash
INC='-Iinc -Iechar_t/inc -Ieint_t/inc -Iintlen/inc -Igetdigit/inc -Ito_string/inc'
LIB='-Llib -Lintlen/lib -Lgetdigit/lib -Lto_string/lib'
LL='-ltagged_memory -lto_string -lgetdigit -lintlen'
g++ -std=c++11 $INC $LIB -Wall -DARC64 -o $2 $1 $LL
