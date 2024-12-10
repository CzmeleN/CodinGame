#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>

constexpr int MAX_X = 1920;
constexpr int MAX_Y = 750;
constexpr int GARGOYLES_LIMIT = 10;
constexpr int OBJECTS_LIMIT = 120;
constexpr int MAX_DIST = 150;
constexpr int REACH = 30;
constexpr int INF = 99;
constexpr int MAX_TURNS = 20;
constexpr int MID_X = 960;
constexpr int MID_Y = 380;

struct Gargoyle {
    int x;
    int y;
    int cooldown;
};

struct Object {
    int val;
    int x;
    int y;
    int speed;
};

struct Move {
    int turns;
    int x;
    int y;
};

class Solver {
    private:
    Gargoyle my_gargoyles[GARGOYLES_LIMIT];
    Gargoyle foe_gargoyles[GARGOYLES_LIMIT];
    Object objects[OBJECTS_LIMIT];
    bool capturable[OBJECTS_LIMIT];
    bool captured[OBJECTS_LIMIT];
    int turns_req[GARGOYLES_LIMIT][OBJECTS_LIMIT];
    int moves[GARGOYLES_LIMIT];
    int gargoyles_count;
    int objects_count;
    int missed_till_end;
    int my_score;
    int foe_score;

    void init_input() {
        std::cin >> gargoyles_count;
    }

    void turn_input() {
        int id;

        std::cin >> missed_till_end;
        std::cin >> my_score;
        
        for (int i = 0; i < gargoyles_count; ++i) {
            std::cin >> my_gargoyles[i].x;
            std::cin >> my_gargoyles[i].y;
            std::cin >> my_gargoyles[i].cooldown;
        }

        std::cin >> foe_score;

        for (int i = 0; i < gargoyles_count; ++i) {
            std::cin >> foe_gargoyles[i].x;
            std::cin >> foe_gargoyles[i].y;
            std::cin >> foe_gargoyles[i].cooldown;
            moves[i] = -1;
        }

        std::cin >> objects_count;

        for (int i = 0; i < objects_count; ++i) {
            std::cin >> id;
            std::cin >> objects[i].val;
            std::cin >> objects[i].x;
            std::cin >> objects[i].y;
            std::cin >> objects[i].speed;
            objects[i].speed = -objects[i].speed;
            capturable[i] = true;
        }
    }

    void move_objects() {
        for (int i = 0; i < objects_count; ++i) {
            if (objects[i].y <= objects[i].speed) {
                capturable[i] = false;
            }
            else {
                objects[i].y += objects[i].speed;
            }
        }
    }

    int calc_req(int g, int o) {
        const int xdiffpow = (my_gargoyles[g].x - objects[o].x) * (my_gargoyles[g].x - objects[o].x);
        const int s = objects[o].speed;
        int y = objects[o].y;
        int ydiff = my_gargoyles[g].y - y;
        int possib = MAX_DIST + REACH;
        int res = 0;
        
        while (possib * possib < xdiffpow + ydiff * ydiff) {
            possib += MAX_DIST;
            ydiff -= s;
            y += s;
            res++;

            if (res > MAX_TURNS || y <= 0) {
                return -1;
            }
        }

        return res;
    }

    void calc_moves() {
        int curr_req;
        int my_best = INF, my_best_id;

        for (int i = 0; i < gargoyles_count; ++i) {
            for (int j = 0; j < objects_count; ++j) {
                if (capturable[j]) {
                    turns_req[i][j] = calc_req(i, j);
                }
                else {
                    turns_req[i][j] = -1;
                }
            }
        }

        for (int t = 0; t < MAX_TURNS; ++t) {
            for (int i = 0; i < gargoyles_count; ++i) {
                if (moves[i] == -1) {
                    for (int j = 0; j < objects_count; ++j) {
                        if (turns_req[i][j] == t && !captured[j]) {
                            moves[i] = j;
                            break;
                        }
                    }
                }
            }
        }
    }

    void get_moves() {
        int id, y;

        for (int i = 0; i < gargoyles_count; ++i) {
            id = moves[i];
            y = objects[id].y + turns_req[i][id] * objects[id].speed;

            if (y > MAX_Y) {
                y = MAX_Y;
            }

            if (id != -1) {
                std::cout << "FLY " << objects[id].x << ' ' << y << " might collect" << std::endl;
            }
            else {
                std::cout << "FLY " << MID_X << ' ' << MID_Y << " i'm chilling" << std::endl;
            }
        }
    }

    public:
    void solve() {
        init_input();

        while(true) {
            turn_input();
            move_objects();
            calc_moves();
            get_moves();
        }
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Solver solver;
    solver.solve();

    return 0;
}
