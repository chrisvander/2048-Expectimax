//
//  Board.cpp
//
//
//  Created by Chris Vanderloo on 4/1/18.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "Board.h"

Board::Board() {
    state = 0x0;
    gameScore = 0;
}

Board::Board(Board &b) {
    state = b.state;
    gameScore = b.gameScore;
}

int getIndex(uint16_t val, int index) {
    return (val & (0xF << 4*index)) >> 4*index;
}

float getRand(int max, int min=0) {
    return min+(rand()%(max-min));
}

uint16_t reverseTiles(uint16_t in) {
    uint16_t val = 0;
    for (int i=0; i<4; i++) {
        val = val << 4;
        val += getIndex(in, i);
    }
    return val;
}

void printArray(std::vector<int> in) {
    for (int i : in) {
        printf("%d ", i);
    }
}


void Board::setTile(int index, int value) {
    long val = (long)value;
    index = 4*index;
    long mask = 0xf;
    val = val << index;
    mask = mask << index;
    mask = ~mask;
    state = state & mask;
    state = state | val;
}

void Board::setFirstMove() {
    int i=2;
    int prev=0;
    int r=0;
    while (i!=0) {
        if (prev==0) r = getRand(16);
        else {
            r = getRand(16,1);
            if (r<=prev) r--;
        }
        float p = getRand(100)/100;
        int v;
        printf("%.3f\n",p);
        if (p<0.9) v=1;
        else v=2;
        setTile(r, v);
        prev = r;
        i--;
    }
}

uint8_t Board::getTile(int i) {
    long state_dup = state;
    state_dup = state_dup >> (4*i);
    long mask = 0xf;
    state_dup = state_dup & mask;
    return (uint8_t)state_dup;
}

uint16_t Board::getCol(int i) {
    uint16_t sec = 0;
    for (int j=0; j<4; j++) {
        sec = sec << 4;
        sec += getTile(j*4+i);
    }
    return sec;
}

void Board::setRow(int i, uint16_t v) {
    for (int j=0; j<4; j++) {
        setTile(i*4+j, getIndex(v,0));
        v = v >> 4;
    }
}

void Board::setCol(int i, uint16_t v) {
    for (int j=0; j<4; j++) {
        setTile(j*4+i, getIndex(v,0));
        v = v >> 4;
    }
}

uint16_t Board::getRow(int i) {
    uint16_t sec = 0;
    for (int j=0; j<4; j++) {
        sec = sec << 4;
        sec += getTile(i*4+j);
    }
    return sec;
}

uint16_t Board::smushInDirection(uint16_t val, bool pos, bool score) {
    std::vector<int> sections;
    if (pos) val = reverseTiles(val);
    //printf("\n\e[30C 0x%04x - ", val);
    for (int i=3; i>=0; i--) {
        int k = getIndex(val,i);
        if (k != 0) sections.push_back(k);
    }
    int current = 0;
    //printArray(sections);
    //printf(" - ");
    for (int n=sections.size()-1; n>=0; n--) {
        if (sections[n]==current) {
            //printf("%d, %d", sections[n+1], sections[n]);
            sections[n]++;
            sections.erase(sections.begin() + n + 1);
            if (score) gameScore = gameScore += pow(2,sections[n]);
            current = 0;
        }
        else current = sections[n];
    }
    //printf(" - ");
    //printArray(sections);
    uint16_t ret = 0;
    for (int n : sections) {
        ret = ret << 4;
        ret += n;
    }
    if (!pos) ret = reverseTiles(ret);
    return ret;
}

bool Board::createRandomTile() {
    std::vector<int> indexes;
    for (int i=0; i<16; i++)
        if (getTile(i) == 0) indexes.push_back(i);
    if (indexes.size() == 0) return false;
    float p = getRand(100)/100;
    int v;
    if (p<0.9) v=1;
    else v=2;
    setTile(indexes[(int)getRand(indexes.size())],v);
    return true;
}

bool Board::move(int i, bool addTile, bool debug) {

    //***** DEBUG INFO *****//
    if (debug && addTile) {
        printf("\e[10;4H\e[34C -- Debug Info --\n");
        printf("\n\e[27CLocation | ");
        if (i>1) printf("\e[42m\e[1m");
        printf("value");
        printf("\e[0m   | ");
        if (i<2) printf("\e[42m\e[1m");
        printf("reverse\e[0m | post-move");
        if (i%2==0) for (int j=0; j<4; j++) {
            printf("\n\e[30CCol %01d | 0x%04x  | 0x%04x  | 0x%04x", j, getCol(j), reverseTiles(getCol(j)), reverseTiles(smushInDirection(getCol(j), i<2, false)));
        }
        else for (int j=0; j<4; j++) {
            printf("\n\e[30CRow %01d | 0x%04x  | 0x%04x  | 0x%04x", j, getRow(j), reverseTiles(getRow(j)), reverseTiles(smushInDirection(getRow(j), i<2, false)));
        }
        printf("\n\e[30CDirection: %s\n", i>1 ? "RIGHT" : "LEFT ");
    }
    //***** END DEBUG INFO *****//

    uint64_t st2 = state;

    for (int j=0; j<4; j++) {
        if (i%2==0) setCol(j, smushInDirection(getCol(j), i<2));
        else setRow(j, smushInDirection(getRow(j), i<2));
    }
    if (state == st2) {
        state = st2;
    }
    else if (addTile) return createRandomTile();
    return false;
}

float blankTiles(Board& b) {
    float c = 0;
    for (int i=0; i<16; i++) {
        if (getIndex(b.getCol(i/4), i%4) == 0)
            c++;
    }
    return c;
}

std::vector<int> neighbors(int index) {
    std::vector<int> result;
    if (index   % 4 != 0) result.push_back(index-1);
    if (index+1 % 4 != 0) result.push_back(index+1);
    if (index <12) result.push_back(index+4);
    if (index > 3) result.push_back(index-4);
    return result;
}

bool canMove(Board& b) {
    bool ret = false;
    for (int i=0; i<4; i++) {
        Board tmp(b);
        tmp.move(i,false,false);
        if (b.state != tmp.state){
            ret = true;
            break;
        }
    }
    return ret;
}

int maxTile(Board& b) {
    int max = b.getTile(0);
    for (int i=1; i<16; i++) {
        int tile = b.getTile(i);
        if (tile>max) max = tile;
    }
    return max;
}

double Board::calculateScore(bool debug) {
    double score = 10; 
    //***** HEURISTICS *****//
    // These determine the performance of the AI based on what
    // makes a board "good" or "bad."
    if (debug)printf("\n\n");

    // DIFFERENT WEIGHTS
    // int weights[16] = {
    //    6, 5, 4, 3,
    //    5, 4, 3, 2,
    //    4, 3, 2, 1,
    //    3, 2, 1, 0
    // };
    int weights[16] = { // this table works the best usually
        15, 14, 13, 12,
        8,  9,  10, 11,
        7,  6,   5,  4,
        0,  1,   2,  3
    };
    #pragma omp parallel for
    for (int i=0; i<16; i++) {
        if (getTile(i)!=0) {
            double tile = pow(2,getTile(i));
            double weight = pow(2,weights[i]);
            double fin = tile * weight;
            score += fin;

            for (int j : neighbors(i)) {
                if (getTile(j)!=0)
                    score -= pow(2,abs(getTile(i)-getTile(j)));
            }
        }
    }
    score=150+score;
    if (debug) printf("Weight Score: %f", score);

    int count = 0;
    for (int i=0;i<4;i++) for (int j=0; j<3; j++) {
        uint16_t row = reverseTiles(getRow(i));
        if (getIndex(row,j) > getIndex(row,j+1) && getIndex(row,j+1) != 0)
            count+=pow(2,getIndex(row,j));
        else break;
    }
    if (debug) printf("\n\e[KRow Score Count: %d",count);
    score+=count;

    for (int i=0;i<4;i++) for (int j=0; j<3; j++) {
        uint16_t col = reverseTiles(getCol(i));
        if (getIndex(col,j) > getIndex(col,j+1) && getIndex(col,j+1) != 0)
            count+=pow(2,getIndex(col,j));
        else break;
    }
    if (debug) printf("\n\e[KCol Score Count: %d",count);
    score+=count;

    if (debug) printf("\n\nHeuristics Score: %f", score);
    //***** END HEURISTICS *****//

    return score;
}

void Board::printNumHelper(int tile) {
    int n = getTile(tile);
    n = pow(2,n);
    if (n==1) printf("     ");
    else printf("%5d", n);
}

void Board::printBoard() {
    int k = 0;
    printf("\n\e7%ld  0x%016llx\n", gameScore, state);
    for (int j=0; j<17; j++) {
        if (j%4==0)
            for (int i=0; i<25; i++)
                printf("-");
        else
            for (int i=0; i<25; i++) {
                if (i%6==0) printf("|");
                else if ((j+2)%4 == 0) {printNumHelper(k);k++;i+=4;}
                else printf(" ");
            }
        printf("\n");
    }
}
