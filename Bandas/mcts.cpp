#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <cmath>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include <sys/time.h>

constexpr int SIDE = 8;
constexpr int MAX = 64;
constexpr int MOVES_COUNT = 4;
constexpr int TURN_LIMIT = 200;

constexpr short HOLE = -1;
constexpr short EMPTY = 0;
constexpr short ME = 1;
constexpr short OPP = 2;

constexpr uint32_t UP = 0;
constexpr uint32_t RIGHT = 1;
constexpr uint32_t DOWN = 2;
constexpr uint32_t LEFT = 3;

constexpr int SECOND = 1000000;
constexpr int LIMIT = 100000;
constexpr int INTERVAL = 64;

constexpr float UCB_C = 1.4142f;

constexpr int BUFFER_SIZE = 1000000;

uint64_t seed = 1234567;

uint32_t randU32() {
    seed = (0x5DEECE66DUL * seed + 0xBUL) & ((1UL << 48) - 1);
    return seed >> 16;
}

class Node {
private:
    Node* parent;
    Node* children[MOVES_COUNT] = {nullptr};
    short board[MAX];
    int my_id;
    int opp_id;
    int turn;
    int my_count;
    int opp_count;
    int visits = 0;
    int wins = 0;
    int id;
    bool my_turn;
    bool solved = false;
    bool is_win = false;

    void copy(Node* src) {
        std::memcpy(board, src->board, sizeof(board));
        my_id = src->my_id;
        opp_id = src->opp_id;
        turn = src->turn;
        my_count = src->my_count;
        opp_count = src->opp_count;
        my_turn = src->my_turn;
        parent = src;
        solved = src->solved;
        is_win = src->is_win;
    }

    void move(const uint32_t dir) {
        int movable_id = my_turn ? ME : OPP;
        int non_movable_id = my_turn ? OPP : ME;
        int id;
        short last;
        bool move, clear_rows, clear_cols;

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

        clear_rows = true;
        clear_cols = true;

        for (int i = 0; i < SIDE; ++i) {
            if (clear_rows) {
                id = i * SIDE;
                for (int j = 0; j < SIDE; ++j, ++id) {
                    if (board[id] > EMPTY) {
                        clear_rows = false;
                        break;
                    }
                }

                if (clear_rows) {
                    id = i * SIDE;

                    for (int j = 0; j < SIDE; ++j, ++id) {
                        board[id] = HOLE;
                    }
                }
            }

            if (clear_cols) {
                id = i;

                for (int j = 0; j < SIDE; ++j, id += SIDE) {
                    if (board[id] > EMPTY) {
                        clear_cols = false;
                        break;
                    }
                }

                if (clear_cols) {
                    id = i;

                    for (int j = 0; j < SIDE; ++j, id += SIDE) {
                        board[id] = HOLE;
                    }
                }
            }
        }

        clear_rows = true;
        clear_cols = true;

        for (int i = SIDE - 1; i >= 0; --i) {
            if (clear_rows) {
                id = i * SIDE;
                for (int j = 0; j < SIDE; ++j, ++id) {
                    if (board[id] > EMPTY) {
                        clear_rows = false;
                        break;
                    }
                }

                if (clear_rows) {
                    id = i * SIDE;

                    for (int j = 0; j < SIDE; ++j, ++id) {
                        board[id] = HOLE;
                    }
                }
            }

            if (clear_cols) {
                id = i;

                for (int j = 0; j < SIDE; ++j, id += SIDE) {
                    if (board[id] > EMPTY) {
                        clear_cols = false;
                        break;
                    }
                }

                if (clear_cols) {
                    id = i;

                    for (int j = 0; j < SIDE; ++j, id += SIDE) {
                        board[id] = HOLE;
                    }
                }
            }
        }

        turn++;

        if (my_count == 0) {
            solved = true;
            is_win = false;
        } else if (opp_count == 0) {
            solved = true;
            is_win = true;
        } else if (turn >= TURN_LIMIT) {
            solved = true;
            is_win = (my_count > opp_count);
        }
    }

    void change_whose() {
        my_turn = !my_turn;
    }

    int random_game() {
        Node sim;
        int best_move, best_score;
        sim.copy(this);
        
        while (true) {
            if (sim.solved) return sim.is_win ? 1 : 0;

            best_move = -1;
            best_score = -1;

            for (int i = 0; i < MOVES_COUNT; ++i) {
                Node child;

                child.copy(&sim);
                child.move(i);
                child.change_whose();

                if (child.my_count > child.opp_count) {
                    best_move = i;
                    break;
                }
            }

            if (best_move == -1) best_move = randU32() % MOVES_COUNT;

            sim.move(best_move);
            sim.change_whose();
        }
    }

    float ucb_val() {
        if (solved) {
            return is_win ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
        }

        if (visits == 0) {
            return std::numeric_limits<float>::infinity();
        }

        return (static_cast<float>(wins) / visits) + UCB_C * std::sqrt(std::log(static_cast<float>(parent->visits)) / visits);
    }

    Node* select_node() {
        Node *curr = this, *selected, *child;
        float best, ucb;
        bool has_unexpanded;

        while (true) {
            if (curr->solved) {
                return curr;
            }

            has_unexpanded = false;

            for (int i = 0; i < MOVES_COUNT; ++i) {
                if (curr->children[i] == nullptr) {
                    has_unexpanded = true;
                    break;
                }
            }

            if (has_unexpanded) return curr;

            selected = nullptr;
            best = -std::numeric_limits<float>::infinity();

            for (int i = 0; i < MOVES_COUNT; ++i) {
                child = curr->children[i];

                ucb = child->ucb_val();

                if (ucb > best) {
                    best = ucb;
                    selected = child;
                }
            }

            curr = selected;
        }

        return curr;
    }

    Node* expand() {
        Node* child;

        for (int i = 0; i < MOVES_COUNT; ++i) {
            if (children[i] == nullptr) {
                child = new Node();

                child->copy(this);
                child->move(i);
                child->change_whose();
                children[i] = child;

                if (child->solved) {
                    child->visits = 1;
                    child->wins = child->is_win ? 1 : 0;
                }

                return child;
            }
        }

        return this;
    }

    void backprop(int res) {
        Node* curr = this, *child;

        while (curr != nullptr) {
            curr->visits += 1;
            curr->wins += res;

            if (!curr->solved) {
                bool all_solved = true;
                bool any_win = false;
                bool all_lose = true;

                for (int i = 0; i < MOVES_COUNT; ++i) {
                    child = curr->children[i];

                    if (child == nullptr) {
                        all_solved = false;
                        continue;
                    }

                    if (!child->solved) {
                        all_solved = false;
                    }
                    if (child->is_win) {
                        any_win = true;
                        all_lose = false;
                    }
                }

                if (all_solved) {
                    curr->solved = true;
                    curr->is_win = curr->my_turn ? any_win : all_lose;
                }
            }

            curr = curr->parent;
        }
    }

    void mcts_search() {
        Node* selected, *child;
        timeval start, end;
        int elapsed = 0, played = 0, res;

        gettimeofday(&start, nullptr);
        
        while (elapsed < LIMIT) {
            selected = select_node();

            if (selected->solved) {
                res = selected->is_win ? 1 : 0;
            } else {
                 child = selected->expand();
                res = child->random_game();
            }
            selected->backprop(res);

            if (++played % INTERVAL == 0) {
                gettimeofday(&end, nullptr);
                elapsed = (end.tv_sec - start.tv_sec) * SECOND + end.tv_usec - start.tv_usec;
            }
        }
    }

    void free() {
        for (int i = 0; i < MOVES_COUNT; ++i) {
            if (children[i] != nullptr) {
                children[i]->free();
                delete children[i];
            }
        }
    }

public:
    void* operator new(std::size_t);
    void operator delete(void*);

    static Node buffer[BUFFER_SIZE];
    static int free_list[BUFFER_SIZE];
    static int free_count;

    static void init_buffor() {
        free_count = BUFFER_SIZE;
        
        for(int i = 0; i < BUFFER_SIZE; ++i) {
            free_list[i] = i;
        }
    }

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
        my_turn = true;

        for (int i = 0; i < SIDE; ++i) {
            std::string line;
            std::getline(std::cin, line);

            for (int j = 0; j < SIDE * 2; j += 2, ++id) {
                c = line[j];

                if (c == '-') {
                    board[id] = EMPTY; 
                } else if (c == 'x') {
                    board[id] = HOLE;
                } else if (c == '0') {
                    if (my_id == 0) {
                        my_count++;
                    } else {
                        opp_count++;
                    }
                    board[id] = my_id == 0 ? ME : OPP;
                } else  {
                    if (my_id == 1) {
                        my_count++;
                    } else {
                        opp_count++;
                    }
                    board[id] = my_id == 1 ? ME : OPP;
                }
            }
        }

        turn++;
    }

    Node* move_to(int id) {
        Node* res = children[id];

        res->parent = nullptr;
        children[id] = nullptr;
        free();

        return res;
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

    int find_root(Node& solver) {
        for (int i = 0; i < MOVES_COUNT; ++i) {
            if (children[i] == nullptr) {
                continue;
            }

            if (std::memcmp(solver.board, children[i]->board, sizeof(short) * MAX) == 0) {
                return i;
            }
        }
        
        std::cerr << "nie znaleziono dziecka :o" << std::endl;
        return -1;
    }

    int find_best() {
        mcts_search();
        Node *child;
        int res = UP;
        float best = -1.0, score;

        for (int i = 0; i < MOVES_COUNT; ++i) {
            if (children[i] == nullptr) {
                continue;
            }

            child = children[i];

            if (child->visits > 0) {
                score = static_cast<float>(child->wins) / child->visits;
            } else {
                score = child->is_win ? 1 : 0;
            }

            if (score > best) {
                best = score;
                res = i;
            }

            std::cerr << (i == UP ? "UP" : i == RIGHT ? "RIGHT" : i == DOWN ? "DOWN" : "LEFT") << ' ' << score << ' ' << child->visits << std::endl;
        }

        std::cout << (res == UP ? "UP" : res == RIGHT ? "RIGHT" : res == DOWN ? "DOWN" : "LEFT") << std::endl;
        return res;
    }

    void init_copy(Node* cp) {
        copy(cp);
        parent = nullptr;
    }
};

Node Node::buffer[BUFFER_SIZE];
int Node::free_list[BUFFER_SIZE];
int Node::free_count = 0;

void* Node::operator new(std::size_t) {
    if (free_count == 0) {
        std::cerr << "buffer is full" << std::endl;
        throw std::bad_alloc();
    }

    int id = free_list[--free_count];
    buffer[id].id = id;
    return static_cast<void*>(&buffer[id]);
}

void Node::operator delete(void* ptr) {
    if (ptr == nullptr) {
        std::cerr << "dereferencing a null pointer" << std::endl;
        return;
    }

    Node* np = static_cast<Node*>(ptr);
    int id = np - &buffer[0];
    free_list[free_count++] = id;
}

int main() {
    Node::init_buffor();
    Node* root = new Node(), *next;
    Node solver;
    int res;

    solver.init_input();
    solver.turn_input();
    root->init_copy(&solver);

    res = root->find_best();
    next = root->move_to(res);
    delete root;
    root = next;

    while(true) {
        solver.turn_input();
        res = root->find_root(solver);
        next = root->move_to(res);
        delete root;
        root = next;

        res = root->find_best();
        next = root->move_to(res);
        delete root;
        root = next;
    }
}
