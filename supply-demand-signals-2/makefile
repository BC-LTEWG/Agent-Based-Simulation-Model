APP = persontest

${APP}: header/*.h src/*.cpp
	g++ -std=c++17  src/Person.cpp  src/Firm.cpp src/Society.cpp src/Machine.cpp src/Sim.cpp src/Product.cpp -o $@

clean: 
	rm -f ${APP}
