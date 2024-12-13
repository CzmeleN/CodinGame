#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <limits>
#include <iostream>
#include <stack>
#include <algorithm>
#include <math.h>

constexpr int MAX_X = 1920;
constexpr int MAX_Y = 750;
constexpr int GARGOYLES_LIMIT = 3;
constexpr int OBJECTS_LIMIT = 120;
constexpr int MAX_DIST = 150;
constexpr int REACH = 30;
constexpr int INF = std::numeric_limits<int>::max();
constexpr int MAX_TURNS = 20;
constexpr int MID_X = 960;
constexpr int THIRD_X = 640;
constexpr int START_X = 320;
constexpr int MID_Y = 380;
constexpr int CLUSTER_THRESHOLD = 140 * 140;
constexpr int STEPS_W = 100;
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
    Gargoyle my_gargoyle[GARGOYLES_LIMIT];
    Gargoyle foe_gargoyle[GARGOYLES_LIMIT];
    Object objects[OBJECTS_LIMIT];
    std::pair<int, int> weighted_list[OBJECTS_LIMIT];
    Move move[GARGOYLES_LIMIT];
    bool capturable[OBJECTS_LIMIT];
    bool use_fireball[GARGOYLES_LIMIT];
    int fireball_id[GARGOYLES_LIMIT];
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
        
        for (int i = 0; i < GARGOYLES_LIMIT; ++i) {
            std::cin >> my_gargoyle[i].x;
            std::cin >> my_gargoyle[i].y;
            std::cin >> my_gargoyle[i].cooldown;
        }
        
        std::cin >> foe_score;

        for (int i = 0; i < GARGOYLES_LIMIT; ++i) {
            std::cin >> foe_gargoyle[i].x;
            std::cin >> foe_gargoyle[i].y;
            std::cin >> foe_gargoyle[i].cooldown;
        }

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
            objects[i].y += objects[i].speed;
            if (objects[i].y < 0) capturable[i] = false;
        }
    }

    void demove_objects() {
        for (int i = 0; i < objects_count; ++i) {
            objects[i].y -= objects[i].speed;
            if (objects[i].y < 0) {
                capturable[i] = false;
            }
            else {
                capturable[i] = true;
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
            size = objects[i].val;

            while(!stk.empty()) {
                curr = stk.top();
                stk.pop();
                
                objects[curr].cluster = clusters_count;

                for (int j = 0; j < objects_count; ++j) {
                    if (j == curr || !capturable[j] || objects[j].cluster != -1) continue;
                    if (sq_dist(objects[curr].x, objects[curr].y, objects[j].x, objects[j].y) <= CLUSTER_THRESHOLD) {
                        stk.push(j);
                        objects[j].cluster = clusters_count;
                        size += objects[j].val;
                    }
                }
            }

            cluster_sizes[clusters_count++] = size;
        }
    }

    bool in_zone_obj(const int g, const int o) {
        return objects[o].x >= THIRD_X * g && (objects[o].x < THIRD_X * (g + 1) || (g == 2 && objects[o].x == THIRD_X * (g + 1)));
    }

    int calc_req(bool mine, int g, int o) {
        const int xdiffpow = ((mine ? my_gargoyle[g].x : foe_gargoyle[g].x) - objects[o].x) * ((mine ? my_gargoyle[g].x : foe_gargoyle[g].x) - objects[o].x);
        const int s = objects[o].speed;
        int y = objects[o].y;
        int ydiff = (mine ? my_gargoyle[g].y : foe_gargoyle[g].y) - y;
        int possib = MAX_DIST + REACH;
        int res = 0;
        
        if (y < 0) return -1;

        while (possib * possib < xdiffpow + ydiff * ydiff) {
            possib += MAX_DIST;
            ydiff -= s;
            y += s;
            res++;

            if (res > MAX_TURNS || y < 0) {
                return -1;
            }
        }

        return res;
    }

    void get_weights(int g) {
        int curr_req, diff_req, opp_req;
        weighted_count = 0;

        for (int i = 0; i < objects_count; ++i) {
            if (capturable[i] && in_zone_obj(g, i) && (curr_req = calc_req(true, g, i)) != -1) {
                if (objects[i].y < 0) std::cerr << objects[i].y;

                weighted_list[weighted_count++] = {curr_req * STEPS_W + cluster_sizes[objects[i].cluster] * CLUSTER_W + objects[i].y * HEIGHT_W / 100 + objects[i].val * VALUE_W, i};
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
        for (int g = 0; g < gargoyles_count; ++g) {
            get_weights(g);
            use_fireball[g] = false;

            if (weighted_count == 0) {
                if (my_gargoyle[g].cooldown == 0) {
                    for (int i = 0; i < objects_count; ++i) {
                        if (sq_dist(my_gargoyle[g].x, my_gargoyle[g].y, objects[i].x, objects[i].y - objects[i].speed) <= SQ_FB_REACH && objects[i].val > best_val) {
                            use_fireball[g] = true;
                            fireball_id[g] = objects[i].id;
                            best_val = objects[i].val;
                        }
                    }
                }
                move[g] = {START_X + g * THIRD_X, MAX_Y - 50};
            }
            else {
                best = weighted_list[0].second;
                turns_req = calc_req(true, g, best);
                move[g].x = objects[best].x;
                move[g].y = objects[best].y + turns_req * objects[best].speed;
                if (move[g].y < 0) {
                    std::cerr << move[g].y << ' ' << turns_req << ' ' << objects[best].y << std::endl;
                }

                if (turns_req == 0) {
                    for (int i = 0; i < objects_count; ++i) {
                        if (in_zone_obj(g, i)) {
                            curr_dist = sq_dist(objects[i].x, objects[i].y, objects[best].x, objects[best].y);
                            if (i != best && curr_dist < best_dist) {
                                second = i;
                                best_dist = curr_dist;
                            }
                        }
                    }

                    if (second != -1) {
                        turns_req = calc_req(true, g, second);

                        if (turns_req == 0) {
                            dx = objects[second].x - move[g].x;
                            dy = objects[second].y - move[g].y;
                            std::cerr << "both: " << dx << ' ' << dy << std::endl;
                        }
                        else {
                            capturable[best] = false;
                            move_objects();
                            clusterize();
                            get_weights(g);
                            my_gargoyle[g].x = move[g].x;
                            my_gargoyle[g].y = move[g].y;


                            if (weighted_count == 0) {
                                dx = START_X + g * THIRD_X - move[g].x;
                                dy = MAX_Y - 50 - move[g].y;
                                std::cerr << "centr" << std::endl;
                            }
                            else {
                                second = weighted_list[0].second;
                                dx = objects[second].x - move[g].x;
                                if (move[g].y < 0) std::cerr << move[g].y << std::endl;
                                dy = objects[second].y - move[g].y + calc_req(true, g, second) * objects[second].speed;
                            }
                            demove_objects();
                        }

                        dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));

                        if (dist != 0) {
                            move[g].x += static_cast<double>(dx * (REACH - 2)) / dist;
                            move[g].y += static_cast<double>(dy * (REACH - 2)) / dist;
                        }
                    }
                }
                else {
                    if (my_gargoyle[g].cooldown == 0) {
                        for (int i = 0; i < objects_count; ++i) {
                            if (in_zone_obj(g, i) && (calc_req(false, 0, i) == 0 || calc_req(false, 1, i) == 0 || calc_req(false, 2, i) == 0) && sq_dist(my_gargoyle[g].x, my_gargoyle[g].y, objects[i].x, objects[i].y - objects[i].speed) <= SQ_FB_REACH && objects[i].val > best_val) {
                                use_fireball[g] = true;
                                fireball_id[g] = objects[i].id;
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
    }

    void get_move() {
        calc_move();
        for (int i = 0; i < gargoyles_count; ++i) {
            if (use_fireball[i]) {
                std::cout << "FIREBALL " << fireball_id[i] << std::endl;
            }
            else {
                std::cout << "FLY " << move[i].x << ' ' << (move[i].y > MAX_Y ? MAX_Y : move[i].y) << std::endl;
            }
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
