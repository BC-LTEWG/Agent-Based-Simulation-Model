BIN_DIR = bin
HDR_DIR = header
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test
TEST_SRCS = $(wildcard ${TEST_DIR}/*.cpp)
TEST_EXECS = $(patsubst %.cpp, %.test, ${TEST_SRCS})
APP = ${BIN_DIR}/sim
FLAGS = -Wall -Iheader -std=c++17

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

${BUILD_DIR}/Product.o : ${SRC_DIR}/Product.cpp ${HDR_DIR}/Product.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Machine.o : ${SRC_DIR}/Machine.cpp ${HDR_DIR}/Machine.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Person.o : ${SRC_DIR}/Person.cpp ${HDR_DIR}/Person.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Firm.o : ${SRC_DIR}/Firm.cpp ${HDR_DIR}/Firm.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Producer.o : ${SRC_DIR}/Producer.cpp ${HDR_DIR}/Producer.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Distributor.o : ${SRC_DIR}/Distributor.cpp ${HDR_DIR}/Distributor.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/PriceController.o : ${SRC_DIR}/PriceController.cpp ${HDR_DIR}/PriceController.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Society.o : ${SRC_DIR}/Society.cpp ${HDR_DIR}/Society.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Sim.o : ${SRC_DIR}/Sim.cpp ${HDR_DIR}/Sim.h
	g++ ${FLAGS} -c $< -o $@

TEST_DEPS_RAW = $(wildcard ${SRC_DIR}/*.cpp)
TEST_DEPS = $(filter-out ${SRC_DIR}/main.cpp, ${TEST_DEPS_RAW})

tests: ${TEST_EXECS}

test/%.test: test/%.cpp ${TEST_DEPS}
	g++ ${FLAGS} $^ -o $@


clean:
	rm -f $(wildcard ${BIN_DIR}/*) $(wildcard ${BUILD_DIR}/*) $(wildcard ${TEST_DIR}/*.test)

