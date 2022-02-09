.PHONY: all cls build

all: cls build

cls:
	clear

build:
	$(CXX) test.cpp -fdiagnostics-color -o a.exe

run:
	./a.exe