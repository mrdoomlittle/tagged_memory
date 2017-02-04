make clean && make && g++ -std=c++11 -Iinc -Llib -Wall -DARC64 -o bin/debugging.exec debugging.cpp -ltagged_memory -lintlen && clear && ./bin/debugging.exec
