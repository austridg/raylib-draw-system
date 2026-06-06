# Build for the animation library example.
#
# Targets:
#   make        build the example (default)
#   make run    build (if needed) and launch it
#   make clean  remove build artifacts
#
# Note: this is a simple single-platform (Linux) Makefile. If the library
# ever needs to build on Windows/macOS or be consumed by others as a package,
# migrate to CMake at that point.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# -I/usr/local/include is where rang.hpp lives; raylib flags come from pkg-config
INCLUDES := -I/usr/local/include -Ianimation -Idraw -Iassets
LIBS     := $(shell pkg-config --libs raylib)

TARGET   := anim_example
SOURCES  := main.cpp animation/animation.cpp draw/draw.cpp assets/assets.cpp
OBJECTS  := $(SOURCES:.cpp=.o)
HEADERS  := animation/animation.h draw/draw.h assets/assets.h

.PHONY: all run clean

all: $(TARGET)

# link the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LIBS)

# compile each .cpp to a .o; coarse but correct: any header change rebuilds all
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# run from the project root so test_anim.png resolves by relative path
run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)
