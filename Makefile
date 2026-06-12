# Build for the RAD-2D library example.
#
# Targets:
#   make        build the example (default)
#   make run    build (if needed) and launch it
#   make clean  remove build artifacts
#
# Note: this is a simple single-platform (Linux) Makefile for building the
# example. To consume the library as a package (add_subdirectory / FetchContent /
# install), use the CMakeLists.txt instead.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# -I. lets sources find the umbrella header (rad2d.hpp); raylib flags come from pkg-config
INCLUDES := -I. -Ianimation -Idraw -Iassets -Itiles
LIBS     := $(shell pkg-config --libs raylib)

TARGET   := rad2d_example
SOURCES  := main.cpp animation/animation.cpp draw/draw.cpp assets/assets.cpp tiles/tiles.cpp
OBJECTS  := $(SOURCES:.cpp=.o)
HEADERS  := rad2d.hpp animation/animation.h draw/draw.h assets/assets.h tiles/tiles.h

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
