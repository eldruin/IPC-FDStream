CC = g++
CXXFLAGS = -Wall -pedantic -ansi

all: parent program

parent: fdstream.hpp
	$(CC) $(CXXFLAGS) -o $@ $@.cpp
