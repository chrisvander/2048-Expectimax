//
//  Board.h
//  
//
//  Created by Chris Vanderloo on 4/1/18.
//

#ifndef Board_h
#define Board_h

#include <stdio.h>
#include <unordered_map>

class Board {
public:
    // CONSTRUCTORS
    Board();
    Board(Board &b);
    Board(uint64_t _state, long _gScore=0) : state(_state), gameScore(_gScore) {}
    
    // PRINTING
    void printBoard();
    void printNumHelper(int tile);
    
    // MOVES
    void setFirstMove();
    bool move(int i, bool addTile=true, bool debug=true);
    bool createRandomTile();
    uint16_t smushInDirection(uint16_t val, bool pos, bool score=true);
    
    // GETTERS
    uint8_t getTile(int i);
    uint16_t getCol(int i);
    uint16_t getRow(int i);
    
    // SETTERS
    void setTile(int index, int value);
    void setCol(int i, uint16_t v);
    void setRow(int i, uint16_t v);
    
    // SCORE
    double calculateScore(bool debug=true);
    
    
    /** VARIABLES **/
    uint64_t state; // board state
    long gameScore; // score
};

class TranspositionTable {
public:
    TranspositionTable() {}
    void insert(uint64_t state, double score) { table[state] = score; }
    
    // GET
    // Score returns in score variable
    // RETURN: Success of retrieval from table
    bool get(uint64_t state, double& score){
        std::unordered_map<uint64_t, double>::iterator i = table.find(state);
        if (i == table.end()) return false;
        score = i->second;
        return true;
    }
    
    double operator[](uint64_t state) {
        return table[state];
    }
    
    std::unordered_map<uint64_t, double> table;
};

bool canMove(Board& b);
float blankTiles(Board& b);

#endif
