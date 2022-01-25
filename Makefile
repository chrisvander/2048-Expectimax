all: puzzle

puzzle:
	g++ -fopenmp -std=c++11 *.cpp -o game

