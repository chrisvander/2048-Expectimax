//  2048 AI
//  main.cpp
//  
//
//  Created by Chris Vanderloo on 3/29/18.
//

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <fstream>
#include <sstream>
#include <limits>
#include <time.h>
#include <cstdlib>
#include <vector>
#include "Board.h"
#include "AI.h"
#include "bpnet.h"

#define LOOKAHEAD 4
#define NUM_RAND_GAMES 100

std::vector<int> valid_moves(Board b) {
    std::vector<int> moves;
    for (int i = 0; i < 4; ++i)
    {
        Board dup(b);
        dup.move(i,false,false);
        if (dup.state != b.state) moves.push_back(i);
    }
    return moves;
}

void ansi_init() {
    //-- SET UP INTERFACE
    struct winsize w;
    int i;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
    
    for (i=0; i<w.ws_row; i++) {
        for (int j=0; j<w.ws_col; j++) {
            printf(" ");
        }
        printf("\n");
    }
    printf("\e[%dA", w.ws_row);
    
    for (i=0; i<w.ws_col; i++) {
        printf("_");
    }
    for (i=0; i<w.ws_row-3; i++) {
        printf(" \n");
    }
    for (i=0; i<w.ws_col; i++) {
        printf("_");
    }
    printf("\n");
    printf("\e[%dA", w.ws_row-3);
    //-- END SET UP INTERFACE
}

void loadTable(TranspositionTable& t) {
    std::ifstream in("transposition_table.txt");
    std::string b_state;
    std::string b_score;
    printf("\nLoading in tables...");
    long c = 0;
    while (in >> b_state) {
        in >> b_score;
        std::istringstream iss(b_state);
        uint64_t val;
        iss >> val;
        t.insert(val, atof(b_score.c_str()));
        //printf("\e[A\e[K%ld: Init: %llu at %s\n",c,val, b_score.c_str());
        c++;
        if (c % 13452 == 0) printf("\e[A\e[K%ld\n",c);
    }
    ansi_init();
    printf("Initialized %ld tables\n", c);
    in.close();
}

void exportTable(TranspositionTable& t) {
    std::ofstream ex;
    ex.open("transposition_table.txt", std::ofstream::out | std::ofstream::trunc);
    printf("Beginning transposition\n");
    long long c = 0;
    for (auto itr = t.table.begin(); itr != t.table.end(); ++itr) {
        ex << itr->first << " " << itr->second << "\n";
        c++;
    }
    printf("\e[A\e[KWrote %lld tables", c);
    ex.close();
}

double max(double a, double b) {
    return (a<b)?b:a;
}

double min(double a, double b) {
    return (a>b)?b:a;
}

double expectimax(Board b, int steps, int agent, TranspositionTable& t) {
    Board dup(b);
    double score = 0;
    if (!canMove(b)) return -DBL_MAX;
    if (steps == 0) return b.calculateScore(false);
    else if (agent == 1){
        int c = 0;
        #pragma omp parallel for
        for (int i=0; i<16; i++) {
            if (b.getTile(i) == 0 && c < steps+2) {
                dup.state = b.state;
                dup.setTile(i,1);
                score += 0.9 * expectimax(dup,steps-1,0,t);
                dup.state = b.state;
                dup.setTile(i,2);
                score += 0.1 * expectimax(dup,steps-1,0,t);
                c++;
            }
        }
        if (c == 0) return -DBL_MAX;
        return score/c;
    }
    else if (agent == 0) {
        for (int i : valid_moves(b)) {
            dup.state = b.state;
            dup.move(i,false,false);
            score = max(score, expectimax(dup,steps-1,1,t));
        }
        return score;
    }
    return 0;
}

double calculateLookahead(Board b, int steps, TranspositionTable& t) {
    return expectimax(b,steps,1,t);
}

double rand_selection(Board b, int times) {
    double score = 0;
    for (int i=0; i<times; i++) {
        Board dup(b);
        while (canMove(dup)) {
            dup.move(rand()%4,true,false);
        }
        score += dup.gameScore;
    }
    return score/times;
}

int programMain(char n, bool used) {
    srand(time(NULL));
    
    ansi_init();
    
    char ans = 'y';
    bpnet network;
    printf("Run: \n[a]i \nrandom [m]axing\n[r]andom\n[d]ebug\n[s]ingleplayer\nSelection : ");
    if (!used) std::cin >> ans;
    else ans = n;
    printf("\e[K\e[3;0H");
    
    TranspositionTable t;
    loadTable(t);
    printf("Loading heuristics...");
    
    ansi_init();

    bool by_one = true;
    int lookahead = 1;
    if (ans == 'a') {
        printf("Lookahead: ");
        scanf("%d", &lookahead);
        printf("\e[A\e[K");
        if (lookahead < 4) by_one = false;
    }
    
    Board b;
    printf("First Move rand val: \n");
    b.setFirstMove();
    b.printBoard();
    printf("\e[A");

    
    char move;
    bool cont = true;
    int d = 0;
    double avg_score = rand_selection(b,10);
    double overall_avg = 0;


    while (cont) {
        if (ans == 'a') {
            Board dup;
            double look = 0;

            std::vector<int> valid = valid_moves(b);
            int index = 0;
            if (valid.size() > 0) index = valid_moves(b)[0];
            else break;

            int c = 0;
            for (int j : valid) {
                dup = Board(b);
                dup.move(j,false,false);
                double calc = calculateLookahead(dup,lookahead,t);
                // printf("\n\e[K%d Lookahead: %f", j, calc);
                if (look < calc || c == 0) {
                    index = j;
                    look = calc;
                }
                c++;
            }
            double temp = rand_selection(b,50);
            overall_avg += (temp-avg_score);
            if (d%10==0||by_one) printf("\e[34;0H\n\n\e[KSelected: %d, Move: %d, Avg Score: %.1f, Change in Avg: %.1f, Slope: %.1f", index, d, temp, temp-avg_score, overall_avg);
            avg_score = temp;
            
            if (canMove(b)) b.move(index,true,false);
            else {
                cont = false;
                break;
            }
            printf("\e[6;0H");
            if (d%10==0||by_one) {
                b.printBoard();
                b.calculateScore();
            }
        }
        else if (ans == 'm') {
            Board dup;
            double look = -1*pow(10,17);
            int index = 0;
            int j=0;
            for (; j<4; j++) {
                dup = Board(b);
                dup.move(j,false,false);
                if (dup.state != b.state) {
                    double calc = rand_selection(dup,NUM_RAND_GAMES);
                    printf("\e[4%d;0H\nScore Max %d: %f", j, j, calc);
                    if (j==0) look = calc;
                    else if (look < calc) {
                        index = j;
                        look = calc;
                    }
                }
                else printf("\e[4%d;0H\n\e[K", j);
            }
            printf("\n\nSelected: %d, Move: %d, Score: %ld", index, d, b.gameScore);
            
            if (canMove(b)) b.move(index,true,false);
            else {
                cont = false;
                break;
            }
            printf("\e[6;0H");
            b.printBoard();
            b.calculateScore();
        }
        else if (ans == 'r') {
            std::vector<int> pos_moves = valid_moves(b);
            Board dup;
            if (pos_moves.size() == 0) {
                cont = false;
                break;
            }
            int m = pos_moves[rand() % pos_moves.size()];
            b.move(m,true,false);
            printf("\e[6;0H");
            b.printBoard();
        }
        else if (ans == 'd') {
            uint64_t board_s=0;
            printf("\n\n\nEnter BoardState: ");
            scanf("%llx",&board_s);
            b.state = board_s;
            printf("\e[6;0H");
            b.printBoard();
            b.calculateScore();
        }
        else {
            printf("\e[K");
            printf("Make move (wasd): ");
            std::cin >> move;
            int select=-1;
            if (move == 'w') select = 0;
            else if (move == 'a') select = 1;
            else if (move == 's') select = 2;
            else if (move == 'd') select = 3;
            else if (move == 'r') return 0; // restart
            else cont = false;
            unsigned long st = b.state;
            if (canMove(b)) b.move(select);
            else {
                cont = false;
                break;
            }
            printf("\e[6;0H");
            b.printBoard();
            b.calculateScore();
            if (st == b.state) printf("\e[KNo Change");
            printf("\e[25;0H");
        }
        d++;
    }
    printf("\e[2J");
    printf("\e[0;0H");
    printf("\e[2J\e[0;0HYou have lost.\nFinal board state: ");
    printf("\n AVG SCORE: %2f",overall_avg);
    b.printBoard();
    b.calculateScore();
    exportTable(t);
    return 1; // end program
}

int main (int argc, char **argv) {
    char n = ' ';
    bool used = false;
    if (argc>1) {
        n = argv[1][0];
        used = true;
    }
    while (true) if (programMain(n,used) == 1) break;
}
