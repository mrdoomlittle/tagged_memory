CURR_PATH=${CURDIR}
INC=-I$(CURR_PATH)/termio/inc -I$(CURR_PATH)/echar_t/inc -I$(CURR_PATH)/mdlint/inc -I$(CURR_PATH)/intlen/inc -I$(CURR_PATH)/getdigit/inc -I$(CURR_PATH)/to_string/inc
all: libs src/tagged_memory.o
	ar rcs lib/libtagged_memory.a src/tagged_memory.o
	cp src/tagged_memory.hpp inc/tagged_memory.hpp

src/tagged_memory.o: src/tagged_memory.cpp
	g++ -c -std=c++11 -Wall $(INC) -DARC64 -o src/tagged_memory.o src/tagged_memory.cpp

libs:
	cd termio; make; cd ../;
	cd intlen; make ARC64 MDLINT_INC=$(CURR_PATH)/mdlint/inc; cd ../;
	cd getdigit; make ARC64 MDLINT_INC=$(CURR_PATH)/mdlint/inc INTLEN_INC=$(CURR_PATH)/intlen/inc INTLEN_LIB=$(CURR_PATH)/intlen/lib; cd ../;
	cd to_string; make ECHAR_T_INC=$(CURR_PATH)/echar_t/inc MDLINT_INC=$(CURR_PATH)/mdlint/inc GETDIGIT_INC=$(CURR_PATH)/getdigit/inc INTLEN_INC=$(CURR_PATH)/intlen/inc GETDIGIT_LIB=$(CURR_PATH)/getdigit/lib INTLEN_LIB=$(CURR_PATH)/intlen/lib;

install:
	cp inc/tagged_memory.hpp /usr/local/include
	cp lib/libtagged_memory.a /usr/local/lib

lib_clean:
	cd termio; make clean; cd ../;
	cd intlen; make clean; cd ../;
	cd getdigit; make clean; cd ../;
	cd to_string; make clean; cd ../;

clean:
	rm -f bin/*
	rm -f src/*.o
	rm -f lib/*.a
	rm -f inc/*.hpp
