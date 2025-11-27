APP = sim
FLAGS = -Wall -Iheader -std=c++17
HDR_DIR = header
SRC_DIR = src
BUILD_DIR = build

${APP} : ${BUILD_DIR}/Product.o \
	${BUILD_DIR}/Machine.o \
	${BUILD_DIR}/Person.o \
	${BUILD_DIR}/Firm.o \
	${BUILD_DIR}/Producer.o \
	${BUILD_DIR}/Distributor.o \
	${BUILD_DIR}/PriceController.o \
	${BUILD_DIR}/Society.o \
	${BUILD_DIR}/Sim.o \
	${SRC_DIR}/main.cpp
	g++ ${FLAGS} $^ -o $@

${BUILD_DIR}/Product.o : ${SRC_DIR}/Product.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Machine.o : ${SRC_DIR}/Machine.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Person.o : ${SRC_DIR}/Person.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Firm.o : ${SRC_DIR}/Firm.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Producer.o : ${SRC_DIR}/Producer.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Distributor.o : ${SRC_DIR}/Distributor.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/PriceController.o : ${SRC_DIR}/PriceController.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Society.o : ${SRC_DIR}/Society.cpp
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Sim.o : ${SRC_DIR}/Sim.cpp
	g++ ${FLAGS} -c $< -o $@

clean:
	rm -f ${BUILD_DIR}/*


