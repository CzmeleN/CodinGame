#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <limits>
#include <iostream>
#include <stack>
#include <algorithm>
#include <math.h>

constexpr int MAX_X = 1920;
constexpr int MAX_Y = 750;
constexpr int GARGOYLES_LIMIT = 10;
constexpr int OBJECTS_LIMIT = 120;
constexpr int MAX_DIST = 150;
constexpr int REACH = 30;
constexpr int INF = std::numeric_limits<int>::max();
constexpr int MAX_TURNS = 20;
constexpr int MID_X = 960;
constexpr int MID_Y = 380;
constexpr int CLUSTER_THRESHOLD = 140 * 140;
constexpr int STEPS_W = 100;
constexpr int OPPONENT_W = 0;
constexpr int CLUSTER_W = -25;
constexpr int HEIGHT_W = -1;
constexpr int VALUE_W = -100;
constexpr int SQ_FB_REACH = 300 * 300;

struct Gargoyle {
    int x;
    int y;
    int cooldown;
};

struct Object {
    int id;
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
    std::pair<int, int> weighted_list[OBJECTS_LIMIT];
    Move move;
    bool capturable[OBJECTS_LIMIT];
    bool use_fireball;
    int fireball_id;
    int clusters[OBJECTS_LIMIT][OBJECTS_LIMIT];
    int cluster_sizes[OBJECTS_LIMIT];
    int gargoyles_count;
    int objects_count;
    int missed_till_end;
    int my_score;
    int foe_score;
    int clusters_count;
    int weighted_count;

    void init_input() {
        std::cin >> gargoyles_count;
    }

    void turn_input() {
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
            std::cin >> objects[i].id;
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
        int curr, size;
        
        clusters_count = 0;

        for (int i = 0; i < objects_count; ++i) {
            if (!capturable[i] || objects[i].cluster != -1) continue;
            std::stack<int> stk;
            stk.push(i);
            objects[i].cluster = clusters_count;
            size = 1;

            while(!stk.empty()) {
                curr = stk.top();
                stk.pop();
                
                objects[curr].cluster = clusters_count;

                for (int j = 0; j < objects_count; ++j) {
                    if (j == curr || !capturable[j] || objects[j].cluster != -1) continue;
                    if (sq_dist(objects[curr].x, objects[curr].y, objects[j].x, objects[j].y) <= CLUSTER_THRESHOLD) {
                        stk.push(j);
                        objects[j].cluster = clusters_count;
                        size++;
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

    void get_weights() {
        int curr_req, diff_req;
        weighted_count = 0;

        for (int i = 0; i < objects_count; ++i) {
            if (capturable[i] && (curr_req = calc_req(true, i)) != -1) {
                diff_req = curr_req - calc_req(false, i);

                if (diff_req < 0) {
                    diff_req = 0;
                }

                weighted_list[weighted_count++] = {curr_req * STEPS_W + diff_req * OPPONENT_W + cluster_sizes[objects[i].cluster] * CLUSTER_W + objects[i].y * HEIGHT_W / 100 + objects[i].val * VALUE_W, i};
            }
        }
        
        std::sort(weighted_list, weighted_list + weighted_count, [] (const std::pair<int, int>&a, const std::pair<int, int>&b) -> bool {
            return a.first < b.first;
        });
    }

    void calc_move() {
        int best, second = -1, best_dist = INF, curr_dist, best_val = 0;
        int turns_req;
        int dx, dy;
        double dist;

        clusterize();
        get_weights();
        use_fireball = false;

        if (weighted_count == 0) {
            if (my_gargoyle.cooldown == 0) {
                for (int i = 0; i < objects_count; ++i) {
                    if (sq_dist(my_gargoyle.x, my_gargoyle.y, objects[i].x, objects[i].y - objects[i].speed) <= SQ_FB_REACH && objects[i].val > best_val) {
                        use_fireball = true;
                        fireball_id = objects[i].id;
                        best_val = objects[i].val;
                    }
                }
            }
            move = {MID_X, MAX_Y};
        }
        else {
            best = weighted_list[0].second;
            turns_req = calc_req(true, best);
            move.x = objects[best].x;
            move.y = objects[best].y + turns_req * objects[best].speed;

            if (turns_req == 0) {
                for (int i = 0; i < objects_count; ++i) {
                    curr_dist = sq_dist(objects[i].x, objects[i].y, objects[best].x, objects[best].y);
                    if (i != best && curr_dist < best_dist) {
                        second = i;
                        best_dist = curr_dist;
                    }
                }

                if (second != -1) {
                    turns_req = calc_req(true, second);

                    if (turns_req == 0) {
                        dx = objects[second].x - move.x;
                        dy = objects[second].y - move.y;
                        std::cerr << "both: " << dx << ' ' << dy << std::endl;
                    }
                    else {
                        capturable[best] = false;
                        move_objects();
                        clusterize();
                        get_weights();
                        my_gargoyle.x = move.x;
                        my_gargoyle.y = move.y;


                        if (weighted_count == 0) {
                            dx = MID_X - move.x;
                            dy = MAX_Y - move.y;
                            std::cerr << "centr" << std::endl;
                        }
                        else {
                            second = weighted_list[0].second;
                            std::cerr << "next" << std::endl;
                            dx = objects[second].x - move.x;
                            dy = objects[second].y - move.y + turns_req * objects[second].speed;
                        }
                    }

                    dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));

                    if (dist != 0) {
                        move.x += static_cast<double>(dx * (REACH - 2)) / dist;
                        move.y += static_cast<double>(dy * (REACH - 2)) / dist;
                    }
                }
            }
            else {
                if (my_gargoyle.cooldown == 0) {
                    for (int i = 0; i < objects_count; ++i) {
                        if (calc_req(false, i) == 0 && sq_dist(my_gargoyle.x, my_gargoyle.y, objects[i].x, objects[i].y - objects[i].speed) <= SQ_FB_REACH && objects[i].val > best_val) {
                            use_fireball = true;
                            fireball_id = objects[i].id;
                        }
                    }

                    // if (!use_fireball && cluster_sizes[objects[best].cluster] == 1 && sq_dist(my_gargoyle.x, my_gargoyle.y, objects[best].x, objects[best].y - objects[best].speed) <= SQ_FB_REACH) {
                    //     use_fireball = true;
                    //     fireball_id = objects[best].id;
                    // }
                }
            }
        }
    }

    void get_move() {
        calc_move();
        if (use_fireball) {
            std::cout << "FIREBALL " << fireball_id << std::endl;
        }
        else {
            std::cout << "FLY " << move.x << ' ' << (move.y > MAX_Y ? MAX_Y : move.y) << std::endl;
        }
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
