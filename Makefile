all: src/tagged_memory.o
	ar rcs lib/libtagged_memory.a src/tagged_memory.o
	cp src/tagged_memory.hpp inc/tagged_memory.hpp

src/tagged_memory.o: src/tagged_memory.cpp
	g++ -c -std=c++11 -Wall -DARC64 -o src/tagged_memory.o src/tagged_memory.cpp -lintlen

clean:
	rm -f bin/*
	rm -f src/*.o
	rm -f lib/*.a
	rm -f inc/*.hpp
