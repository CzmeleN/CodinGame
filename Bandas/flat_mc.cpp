#include <cstdlib>
#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <cstring>
#include <stdint.h>

constexpr int SIDE = 8;
constexpr int MAX = 64;

constexpr short HOLE = -1;
constexpr short EMPTY = 0;
constexpr short ME = 1;
constexpr short OPP = 2;

constexpr uint32_t UP = 0;
constexpr uint32_t RIGHT = 1;
constexpr uint32_t DOWN = 2;
constexpr uint32_t LEFT = 3;

constexpr int LIMIT = 100;

uint64_t seed = 1234567;

uint32_t randU32() {
    seed = (0x5DEECE66DUL * seed + 0xBUL) & ((1UL << 48) - 1);
    return seed >> 16;
}

class Solver {
private:
    short board[MAX];
    int my_id;
    int opp_id;
    int turn;
    int my_count;
    int opp_count;

    void copy(const Solver* src) {
        std::memcpy(board, src->board, sizeof(board));
        my_id = src->my_id;
        opp_id = src->opp_id;
        turn = src->turn;
        my_count = src->my_count;
        opp_count = src->opp_count;
    }

    void move(const uint32_t dir, const bool mine) {
        int movable_id = mine ? ME : OPP;
        int non_movable_id = mine ? OPP : ME;
        int id, emptied_count = 0;
        short last;
        bool move;

        for (int i = 0; i < SIDE; ++i) {
            if (dir == UP) {
                id = (SIDE - 1) * SIDE + i;
            } else if (dir == RIGHT) {
                id = SIDE * i;
            } else if (dir == DOWN) {
                id = i;
            } else { // LEFT
                id = SIDE * i + SIDE - 1;
            }

            move = board[id] == movable_id;

            if (move) {
                board[id] = EMPTY;
                last = movable_id;
                emptied_count++;
            }

            for (int j = 1; j < SIDE; ++j) {
                if (dir == UP) {
                    id -= SIDE;
                } else if (dir == RIGHT) {
                    id++;
                } else if (dir == DOWN) {
                    id += SIDE;
                } else { // LEFT
                    id--;
                }

                if (board[id] == movable_id) {
                    if (move) {
                        board[id] = last;
                    }
                    else {
                        board[id] = EMPTY;
                    }
                    last = movable_id;
                    move = true;
                } else if (board[id] == non_movable_id) {
                    if (move) {
                        board[id] = last;
                        last = non_movable_id;
                    }
                } else if (board[id] == EMPTY) {
                    if (move) {
                        board[id] = last;
                    }
                    move = false;
                } else if (board[id] == HOLE) {
                    if (move) {
                        if (last == ME) {
                            my_count--;
                        } else {
                            opp_count--;
                        }
                    }
                    move = false;
                }
            }

            if (move) {
                if (last == ME) {
                    my_count--;
                } else {
                    opp_count--;
                }
            }
        }

        if (emptied_count == SIDE) {
            if (dir == UP) {
                id = (SIDE - 1) * SIDE;
            } else if (dir == RIGHT) {
                id = 0; 
            } else if (dir == DOWN) {
                id = 0;
            } else { // LEFT
                id = SIDE - 1;
            }

            for (int i = 0; i < SIDE; ++i) {
                board[id] = HOLE;

                if (dir == UP) {
                    id++;
                } else if (dir == RIGHT) {
                    id += SIDE;
                } else if (dir == DOWN) {
                    id++;
                } else { // LEFT
                    id += SIDE;
                }
            }
        }

        turn++;
    }


    bool random_game() {
        bool my_turn = false;

        while(true) {
            uint32_t choice = rand() % 4;
            move(choice, my_turn);
            
            if (opp_count == 0) {
                return true;
            } else if (my_count == 0) {
                return false;
            } else if (turn == 200) {
                return my_count >= opp_count;
            }

            my_turn = !my_turn;
        }
    }

public:
    void init_input() {
        int h, w;

        std::cin >> my_id;
        std::cin >> h >> w;
        std::cin.ignore();

        opp_id = my_id == 0 ? 1 : 0;
        turn = 0;
    }

    void turn_input() {
        char c;
        int id = 0;
        my_count = 0;
        opp_count = 0;

        for (int i = 0; i < SIDE; ++i) {
            std::string line;
            std::getline(std::cin, line);
            // std::cerr << line << std::endl;

            for (int j = 0; j < SIDE * 2; j += 2, ++id) {
                c = line[j];

                if (c == '-') {
                    board[id] = EMPTY; 
                } else if (c == 'x') {
                    board[id] = HOLE;
                } else if (c == '0') {
                    if (my_id == 0) {
                        my_count++;
                    }
                    board[id] = my_id == 0 ? ME : OPP;
                } else  {
                    if (my_id == 1) {
                        my_count++;
                    }
                    board[id] = my_id == 1 ? ME : OPP;
                }
            }
        }

        turn++;
    }

    void print_board() {
        int id = 0;

        for (int i = 0; i < SIDE; ++i) {
            for (int j = 0; j < SIDE; ++j, ++id) {
                std::cerr << board[id] << ' ';
            }
            std::cerr << std::endl;
        }
    }

    void find_best() {
        Solver sim;
        int won, max = 0, res = UP;
        
        for (int i = 0; i < 4; ++i) {
            won = 0;

            for (int j = 0; j < LIMIT; ++j) {
                sim.copy(this);
                sim.move(i, true);
                
                if (sim.random_game()) {
                    won++;
                }
            }

            if (won > max) {
                max = won;
                res = i;
            }
        }

        std::cout << (res == UP ? "UP" : res == RIGHT ? "RIGHT" : res == DOWN ? "DOWN" : "LEFT") << std::endl;
    }
};

int main() {
    Solver solver;
    solver.init_input();

    while(true) {
        solver.turn_input();
        solver.find_best();
    }
}
