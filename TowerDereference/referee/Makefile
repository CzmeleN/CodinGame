CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wfloat-equal -Wundef -Wcast-qual -Wcast-align -Wnon-virtual-dtor
DEBUG_FLAGS := -g -O1 -fsanitize=address -fno-omit-frame-pointer

.PHONY: all debug clean

all: flat_mc referee

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

flat_mc: flat_mc.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

referee: referee.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f flat_mc referee

