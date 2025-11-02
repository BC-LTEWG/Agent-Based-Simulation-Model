CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -I../header
BUILD_DIR = ../header
SRC_DIR = .

TARGET = lte_program

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/Person.o $(BUILD_DIR)/Firm.o $(BUILD_DIR)/Machine.o $(BUILD_DIR)/Distributor.o $(BUILD_DIR)/Society.o main.cpp
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp Person.o Firm.o Machine.o Distributor.o Society.o -o $(SRC_DIR)/$(TARGET) ; popd

$(BUILD_DIR)/Person.o: Person.cpp $(BUILD_DIR)/Person.h
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Person.cpp -o Person.o ; popd

$(BUILD_DIR)/Firm.o: Firm.cpp $(BUILD_DIR)/Firm.h
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Firm.cpp -o Firm.o ; popd

$(BUILD_DIR)/Machine.o: Machine.cpp $(BUILD_DIR)/Machine.h
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Machine.cpp -o Machine.o ; popd

$(BUILD_DIR)/Distributor.o: Distributor.cpp $(BUILD_DIR)/Distributor.h
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Distributor.cpp -o Distributor.o ; popd

$(BUILD_DIR)/Society.o: Society.cpp $(BUILD_DIR)/Society.h
	pushd $(BUILD_DIR) ; $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Society.cpp -o Society.o ; popd

clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
