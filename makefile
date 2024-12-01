# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# Define directories
SRCDIR = src
BUILDDIR = build

# Find all .cpp files recursively in src/ and its subdirectories
SRC = $(shell find $(SRCDIR) -name "*.cpp")

# Object files (convert .cpp to .o and place in the build directory, preserving subdirectories)
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Define the target executable
TARGET = main

# Default target
all: $(TARGET)

# Rule to link object files to create the final executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Rule to compile each .cpp file into .o object file in the build directory
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)  # Ensure the target directory exists
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BUILDDIR) $(TARGET)
