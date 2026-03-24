HDR_DIR = header
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test
DATA_DIR = data
BIN_DIR = bin
DOCS_DIR = docs
GRAPH_DIR = ${DOCS_DIR}/graphs
TEST_SRCS = $(wildcard ${TEST_DIR}/*.cpp)
TEST_EXECS = $(patsubst %.cpp, %.test, ${TEST_SRCS})
APP = ${BIN_DIR}/sim
FLAGS = -Wall -Wsign-compare -Iheader -std=c++17 -g
PLOT_SRC = ${GRAPH_DIR}/plot_producer_distributor_transactions.cpp
PLOT_APP = ${BIN_DIR}/plot_producer_distributor_transactions
MATPLOT_INCLUDE ?= ${HOME}/.local/include
MATPLOT_LIB ?= ${HOME}/.local/lib
PLOT_FLAGS = -std=c++17 -I${MATPLOT_INCLUDE} -L${MATPLOT_LIB} -Wl,-rpath,${MATPLOT_LIB} -lmatplot

${APP} : ${BUILD_DIR}/Product.o \
	${BUILD_DIR}/Machine.o \
	${BUILD_DIR}/ConsumerGood.o \
	${BUILD_DIR}/Person.o \
	${BUILD_DIR}/Firm.o \
	${BUILD_DIR}/Producer.o \
	${BUILD_DIR}/Distributor.o \
	${BUILD_DIR}/PriceController.o \
	${BUILD_DIR}/Society.o \
	${BUILD_DIR}/Logger.o \
	${BUILD_DIR}/Sim.o \
	${BUILD_DIR}/sqlite3.o \
	${SRC_DIR}/main.cpp
	g++ ${FLAGS} $^ -o $@

${BUILD_DIR}/Product.o : ${SRC_DIR}/Product.cpp ${HDR_DIR}/Product.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Machine.o : ${SRC_DIR}/Machine.cpp ${HDR_DIR}/Machine.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/ConsumerGood.o : ${SRC_DIR}/ConsumerGood.cpp ${HDR_DIR}/ConsumerGood.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Person.o : ${SRC_DIR}/Person.cpp ${HDR_DIR}/Person.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Firm.o : ${SRC_DIR}/Firm.cpp ${HDR_DIR}/Firm.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Producer.o : ${SRC_DIR}/Producer.cpp ${HDR_DIR}/Producer.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Distributor.o : ${SRC_DIR}/Distributor.cpp ${HDR_DIR}/Distributor.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/PriceController.o : ${SRC_DIR}/PriceController.cpp ${HDR_DIR}/PriceController.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Society.o : ${SRC_DIR}/Society.cpp ${HDR_DIR}/Society.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Logger.o : ${SRC_DIR}/Logger.cpp ${HDR_DIR}/Logger.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/Sim.o : ${SRC_DIR}/Sim.cpp ${HDR_DIR}/Sim.h ${HDR_DIR}/Constants.h
	g++ ${FLAGS} -c $< -o $@

${BUILD_DIR}/sqlite3.o : ${SRC_DIR}/sqlite3.c ${HDR_DIR}/sqlite3.h
	gcc -Wall -std=c99 -I${HDR_DIR} -c $< -o $@

TEST_DEPS_RAW = $(wildcard ${SRC_DIR}/*.cpp)
TEST_DEPS = $(filter-out ${SRC_DIR}/main.cpp, ${TEST_DEPS_RAW})

tests: ${TEST_EXECS}

test/%.test: test/%.cpp ${TEST_DEPS}
	g++ ${FLAGS} $^ -o $@

.PHONY: trace plot-tool graphs

trace: ${APP}
	./${APP} > trace.txt

plot-tool: ${PLOT_SRC}
	mkdir -p ${BIN_DIR}
	g++ ${PLOT_SRC} ${PLOT_FLAGS} -o ${PLOT_APP}

graphs: trace plot-tool
	mkdir -p ${GRAPH_DIR}
	./${PLOT_APP} trace.txt ${GRAPH_DIR}


clean:
	rm -rf $(wildcard ${BIN_DIR}/*) $(wildcard ${BUILD_DIR}/*) $(wildcard ${TEST_DIR}/*.test) \
		$(wildcard ${DATA_DIR}/*)

