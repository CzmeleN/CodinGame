#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <stack>
#include <math.h>

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
constexpr int CLUSTER_THRESHOLD = 10000;

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
    int cluster;
};

struct Move {
    int x;
    int y;
};

class Solver {
    private:
    Gargoyle my_gargoyle;
    Gargoyle foe_gargoyle;
    Object objects[OBJECTS_LIMIT];
    Move move;
    bool capturable[OBJECTS_LIMIT];
    int turns_req[OBJECTS_LIMIT];
    int clusters[OBJECTS_LIMIT][OBJECTS_LIMIT];
    int cluster_sizes[OBJECTS_LIMIT];
    int gargoyles_count;
    int objects_count;
    int missed_till_end;
    int my_score;
    int foe_score;
    int clusters_count;

    void init_input() {
        std::cin >> gargoyles_count;
    }

    void turn_input() {
        int id;

        std::cin >> missed_till_end;
        std::cin >> my_score;
        
        std::cin >> my_gargoyle.x;
        std::cin >> my_gargoyle.y;
        std::cin >> my_gargoyle.cooldown;

        std::cin >> foe_score;

        std::cin >> foe_gargoyle.x;
        std::cin >> foe_gargoyle.y;
        std::cin >> foe_gargoyle.cooldown;

        std::cin >> objects_count;

        for (int i = 0; i < objects_count; ++i) {
            std::cin >> id;
            std::cin >> objects[i].val;
            std::cin >> objects[i].x;
            std::cin >> objects[i].y;
            std::cin >> objects[i].speed;
            objects[i].speed = -objects[i].speed;
            capturable[i] = true;
            objects[i].cluster = -1;
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

    int sq_dist(int x0, int y0, int x1, int y1) {
        return (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
    }

    void clusterize() {
        int curr, size = 0;
        
        clusters_count = 0;

        for (int i = 0; i < objects_count; ++i) {
            if (!capturable[i] || objects[i].cluster != -1) continue;

            std::stack<int> stk;
            stk.push(i);
            objects[i].cluster = clusters_count;

            while(!stk.empty()) {
                curr = stk.top();
                stk.pop();
                
                objects[curr].cluster = clusters_count;

                for (int j = 0; j < objects_count; ++j) {
                    if (j == curr || !capturable[j] || objects[i].cluster != -1) continue;
                    if (sq_dist(objects[curr].x, objects[curr].y, objects[j].x, objects[j].y) <= CLUSTER_THRESHOLD) {
                        stk.push(j);
                        objects[i].cluster = clusters_count;
                    }
                }
            }

            cluster_sizes[clusters_count++] = size;
        }
    }

    int calc_req(bool mine, int o) {
        const int xdiffpow = ((mine ? my_gargoyle.x : foe_gargoyle.x) - objects[o].x) * ((mine ? my_gargoyle.x : foe_gargoyle.x) - objects[o].x);
        const int s = objects[o].speed;
        int y = objects[o].y;
        int ydiff = (mine ? my_gargoyle.y : foe_gargoyle.y) - y;
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

    void calc_move() {
        // TODO: Implement weighted list
        int best_list[OBJECTS_LIMIT];
        int best = -1, best_t, second = -1;
        int curr_req;
        int my_best = INF, my_best_id;
        int dx, dy;
        double dist;

        for (int i = 0; i < objects_count; ++i) {
            if (capturable[i]) {
                turns_req[i] = calc_req(true, i);
            }
            else {
                turns_req[i] = -1;
            }
        }

        for (int t = 0; t < MAX_TURNS; ++t) {
            for (int i = 0; i < objects_count; ++i) {
                if (turns_req[i] == t) {
                    if (best == -1) {
                        best = i;
                        best_t = t;
                        break;
                    }
                }
            }
        }

        if (best == -1) {
            move = {MID_X, MAX_Y};
        }
        else {
            move.x = objects[best].x;
            move.y = objects[best].y + turns_req[best] * objects[best].speed;

            if (turns_req[best] == 0) {
                for (int i = 0; i < objects_count; ++i) {
                    if (i != best && turns_req[i] <= 1) {
                        second = i;
                        break;
                    }
                }

                if (second != -1) {
                    dx = objects[second].x - move.x;
                    dy = objects[second].y - move.y + objects[second].speed;

                    dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                    
                    move.x += static_cast<double>(dx * (REACH - 5)) / dist;
                    move.y += static_cast<double>(dy * (REACH - 5)) / dist;
                }
            }
        }
    }

    void get_move() {
        calc_move();
        std::cout << "FLY " << move.x << ' ' << move.y << " hello there" << std::endl;
    }

    public:
    void solve() {
        init_input();

        while(true) {
            turn_input();
            move_objects();
            get_move();
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
