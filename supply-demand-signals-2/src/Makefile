cc = gcc
CFLAGS = -Wall -Wextra -Werror -std=c++17

all: my_program

my_program: main.cpp Person.cpp Firm.cpp Machine.cpp Product.cpp Producer.cpp Distributor.cpp Society.cpp Sim.cpp
	$(cc) $(CFLAGS) -o my_program main.cpp Person.cpp Firm.cpp Machine.cpp Product.cpp Producer.cpp Distributor.cpp Society.cpp Sim.cpp

clean:
	rm -f my_program

run: my_program
	./my_program

.PHONY: all clean run
