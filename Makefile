all: puzzle

puzzle:
	g++-7 -fopenmp -std=c++11 *.cpp -o out

