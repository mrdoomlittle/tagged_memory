make clean && make && g++ -std=c++11 -Llib -Iinc -o bin/debugging.exec debugging.cpp -ltagged_memory -lboost_system -lboost_filesystem  && clear && ./bin/debugging.exec
