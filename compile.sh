#!/bin/bash
INC='-Iinc -Itermio/inc -Iechar_t/inc -Ieint_t/inc -Iintlen/inc -Igetdigit/inc -Ito_string/inc'
LIB='-Llib -Ltermio/lib -Lintlen/lib -Lgetdigit/lib -Lto_string/lib'
LL='-ltagged_memory -lto_string -lgetdigit -lintlen -ltermio'
g++ -std=c++11 $INC $LIB -Wall -DARC64 -o $2 $1 $LL
