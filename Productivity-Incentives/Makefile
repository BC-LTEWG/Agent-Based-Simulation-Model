CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCDIR = src
INCDIR = include
TARGET = simulation

# Source files
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/Firm.cpp $(SRCDIR)/Worker.cpp $(SRCDIR)/PriceController.cpp $(SRCDIR)/Simulation.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Include path
INCLUDES = -I$(INCDIR)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the simulation
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean run debug 