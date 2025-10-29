cc = gcc
CFLAGS = -Wall -Wextra -Werror -std=c++17
BUILD_DIR = ../build

# all: my_program

# lte_simulation: main.cpp Person.cpp Firm.cpp Machine.cpp Product.cpp Producer.cpp Distributor.cpp Society.cpp Sim.cpp
#	$(cc) $(CFLAGS) -o my_program main.cpp Person.cpp Firm.cpp Machine.cpp Product.cpp Producer.cpp Distributor.cpp Society.cpp Sim.cpp

all: copy lte_simulation

copy: copy_header copy_src

copy_header:
	cp ../header/*.h ${BUILD_DIR}

copy_src:
	cp ./*.cpp ${BUILD_DIR}

lte_simulation: ${BUILD_DIR}/Person.o main.cpp
	pushd ${BUILD_DIR} ; g++ ${FLAGS} $^ -o $@ ; popd

Person.o : Person.cpp
	pushd ${BUILD_DIR} ; g++ ${FLAGS} -c $^ -o $@ ; popd 

clean:
	rm -f my_program *.o

run: my_program
	./my_program

.PHONY: all clean run
