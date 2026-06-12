# Build for the RAD-2D static library.
#
# Targets:
#   make        build the static library -> librad2d.a (default)
#   make clean  remove build artifacts
#
# Note: this is a simple single-platform (Linux) Makefile. To consume the
# library as a package (add_subdirectory / FetchContent / install), or to build
# on other platforms, use the CMakeLists.txt instead.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
AR       := ar

# -I. lets sources find the umbrella header (rad2d.hpp); raylib headers come from pkg-config
INCLUDES := -I. -Ianimation -Idraw -Iassets -Itiles $(shell pkg-config --cflags raylib)

TARGET   := librad2d.a
SOURCES  := animation/animation.cpp draw/draw.cpp assets/assets.cpp tiles/tiles.cpp
OBJECTS  := $(SOURCES:.cpp=.o)
HEADERS  := rad2d.hpp animation/animation.h draw/draw.h assets/assets.h tiles/tiles.h

.PHONY: all clean

all: $(TARGET)

# archive the compiled objects into a static library
$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

# compile each .cpp to a .o; coarse but correct: any header change rebuilds all
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
