[![Build Status](https://travis-ci.org/chrisvander/2048-Expectimax.svg?branch=master)](https://travis-ci.org/chrisvander/2048-Expectimax)

# 2048 AI
An in-console game of 2048. Play as single player and see what the heuristics do, or run with an AI at multiple search tree depths and see the highest score it can get.

## To download and install
For a machine that has g++ installed, getting this running is as easy as 
```
git clone https://github.com/chrisvander/2048-Expectimax
cd 2048-Expectimax
make
./out
```
You don't have to use `make`, any OpenMP-compatible C++ compiler should work.

## Modes
### AI
Runs with an AI. Specify a number for the search tree depth. For example, 4 is a moderate speed, decent accuracy search to start at.
### Random Maxing
Plays the game several hundred times for each possible moves and picks the move that results in the highest average score.
### Random
Just plays it randomly once. No idea why I added this.
### Debug
View the heuristic score of any possible board state.
### Singleplayer
Just play 2048! Provides heuristic scores and before/after compacting of columns and rows for debug purposes.
