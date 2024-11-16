#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <limits>
#include <set>
#include <iostream>
#include <string>
#include <sys/time.h>

constexpr int EDGE = 400;
constexpr int MAX_SIZE = EDGE * EDGE;
constexpr int EMPTY = 0;
constexpr int OBSTACLE = 1;
constexpr int HOME = 2;
constexpr int UNKNOWN = 3;
constexpr int DIRS = 4;
constexpr int x_dirs[DIRS] = {0, 1, 0, -1};
constexpr int y_dirs[DIRS] = {-1, 0, 1, 0}; 
constexpr char char_dirs[DIRS] = {'W', 'S', 'E', 'N'};
constexpr int start_x = EDGE / 2 - 1, start_y = EDGE / 2 - 1;
constexpr float FLOAT_INF = std::numeric_limits<float>::infinity();

class Solver {
private:
    int map[MAX_SIZE];
    int x = start_x, y = start_y;
    int last_x = x, last_y = y;
    int vision_range, vision_radius;
    int home_dist_hor; // Distance in W/E direction towards home (negative means W side, positive means E side)
    int home_dist_ver; // Distance in N/S direction towards home (negative means N side, positive means S side)
    int home_x, home_y;
    float km = 0.0;

    struct Node {
        int x, y;
        float g;
        float rhs;
        std::pair<float, float> key;
        bool is_open;
    };

    Node nodes[MAX_SIZE];

    std::set<std::pair<std::pair<float, float>, int>> open_list;

    void init_nodes() {
        for (int i = 0; i < MAX_SIZE; ++i) {
            nodes[i].x = i / EDGE;
            nodes[i].y = i % EDGE;
            nodes[i].g = FLOAT_INF;
            nodes[i].rhs = FLOAT_INF;
            nodes[i].key = {FLOAT_INF, FLOAT_INF};
            nodes[i].is_open = false;
        }
    }

    inline void mark_line(std::string &line, int id) {
        for (int i = 0; i < vision_range; ++i) {
            switch (line[i]) {
                case '#':
                    if (map[id] != OBSTACLE) {
                        map[id] = OBSTACLE;
                        update_vertex(id);
                        int ux = id / EDGE;
                        int uy = id % EDGE;
                        for (int d = 0; d < DIRS; ++d) {
                            int nx = ux + x_dirs[d];
                            int ny = uy + y_dirs[d];
                            if (nx >= 0 && nx < EDGE && ny >= 0 && ny < EDGE) {
                                update_vertex(nx * EDGE + ny);
                            }
                        }
                    }
                    break;
                case '?':
                    break;
                case 'H':
                    if (map[id] != HOME) {
                        map[id] = HOME;
                        update_vertex(id);
                    }
                    break;
                default:
                    if (map[id] != EMPTY) {
                        map[id] = EMPTY;
                        update_vertex(id);
                    }
                    break;
            }
            id++;
        }
    }

    void print_map() {
        for (int i = -5; i <= 5; i++) {
            for (int j = -5; j <= 5; j++) {
                int curr_id = (x + i) * EDGE + y + j;
                if (i == 0 && j == 0) {
                    std::cerr << "L ";
                }
                else std::cerr << map[curr_id] << ' ';

            }
            std::cerr << std::endl;
        }
    }

    void turn_input() {
        int base_id = y - vision_radius;
        std::string line;

        for (int i = -vision_radius; i <= vision_radius; ++i) {
            std::getline(std::cin, line);
            // std::cerr << line << '\n';
            mark_line(line, (x + i) * EDGE + base_id);
        }

        print_map();
    }

    float heur(int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }

    void init_dstar() {
        init_nodes();

        Node& s_goal = get_node(home_x, home_y);
        s_goal.rhs = 0.0;
        s_goal.key = calc_key(home_x, home_y);
        s_goal.is_open = true;

        open_list.insert({s_goal.key, home_x * EDGE + home_y});
    }

    std::pair<float, float> calc_key(int nx, int ny) {
        Node& s = get_node(nx, ny);
        float min_g_rhs = std::min(s.g, s.rhs);
        return {min_g_rhs + heur(x, y, nx, ny) + km, min_g_rhs};
    }

    void update_vertex(int id) {
        Node& u = nodes[id];
        int ux = u.x, uy = u.y;
        float new_cost;
        int nx, ny;

        if (u.is_open) {
            open_list.erase({u.key, id});
            u.is_open = false;
        }

        if (ux != home_x || uy != home_y) {
            u.rhs = FLOAT_INF;

            for (int i = 0; i < DIRS; ++i) {
                nx = ux + x_dirs[i];
                ny = uy + y_dirs[i];

                if (valid(nx, ny)) {
                    Node &s = get_node(nx, ny);

                    new_cost = s.g + 1.0;

                    if (new_cost < u.rhs) {
                        u.rhs = new_cost;
                    }
                }
            }
        }

        if (u.g != u.rhs) {
            u.key = calc_key(ux, uy);
            open_list.insert({u.key, id});
            u.is_open = true;
        }


        // if (ux == home_x && uy == home_y) {
        //     if (u.rhs != 0.0) {
        //         u.rhs = 0.0;
        //         if (u.g != u.rhs) {
        //             if (!u.is_open) {
        //                 u.is_open = true;
        //                 open_list.push({u.key, ux * EDGE + uy});
        //             }
        //         }
        //     }
        //     return;
        // }
        //
        // if (u.g > u.rhs) u.rhs = FLOAT_INF;
        //
        // for (int i = 0; i < DIRS; ++i) {
        //     px = ux + x_dirs[i];
        //     py = uy + y_dirs[i];
        //
        //     if (valid(px, py)) {
        //         Node& prev_s = get_node(px, py);
        //         tent_rhs = prev_s.g + 1.0;
        //
        //         if (tent_rhs < u.rhs) {
        //             u.rhs = tent_rhs;
        //         }
        //     }
        // }
        //
        // if (u.g != u.rhs) {
        //     if (!u.is_open) {
        //         u.is_open = true;
        //         open_list.push({calc_key(ux, uy), ux * EDGE + uy});
        //     }
        //     else if (u.is_open){
        //         u.is_open = false;
        //     }
        // }
    }

    void compute_path() {
        Node &s_start = get_node(x, y);
        std::pair<float, float> k_start = calc_key(x, y);
        int nx, ny, u_id;

        while (!open_list.empty() && (open_list.begin()->first < k_start || s_start.rhs != s_start.g)) {
            auto top = *open_list.begin();
            open_list.erase(open_list.begin());
            u_id = top.second;
            Node &u = nodes[u_id];

            if (!u.is_open) {
                continue;
            }

            u.is_open = false;

            if (u.g > u.rhs) {
                u.g = u.rhs;

                for (int i = 0; i < DIRS; ++i) {
                    nx = u.x + x_dirs[i];
                    ny = u.y + y_dirs[i];

                    if (valid(nx, ny)) {
                        update_vertex(nx * EDGE + ny);
                    }
                }
            }
            else {
                u.g = FLOAT_INF;
                update_vertex(u_id);

                for (int i = 0; i < DIRS; ++i) {
                    nx = u.x + x_dirs[i];
                    ny = u.y + y_dirs[i];

                    if (valid(nx, ny)) {
                        update_vertex(nx * EDGE + ny);
                    }
                }
            }

            k_start = calc_key(x, y);
        }
    }

    char get_move() {
        float min_cost = FLOAT_INF, new_cost;
        int nx, ny;
        char res = '*';

        for (int i = 0; i < DIRS; ++i) {
            nx = x + x_dirs[i];
            ny = y + y_dirs[i];

            if (valid(nx, ny)) {
                Node& s_next = get_node(nx, ny);

                std::cerr << s_next.g << ' ';
                if (s_next.g < FLOAT_INF) {
                    new_cost = 1.0 + s_next.g;

                    if (new_cost < min_cost) {
                        min_cost = new_cost;
                        res = char_dirs[i];
                    }
                }
            }
            else {
                std::cerr << "X ";
            }
        }

        return res;
    }

    Node &get_node(int x, int y) {
        return nodes[x * EDGE + y];
    }

    bool valid(int x, int y) {
        if (x >= 0 && x < EDGE && y >= 0 && y < EDGE) {
            if (map[x * EDGE + y] != OBSTACLE) {
                return true;
            }
        }
        return false;
    }

    void update_km() {
        km += heur(last_x, last_y, x, y);
        last_x = x;
        last_y = y;
    }

public:
    void init_input() {
        std::fill(map, map + MAX_SIZE, UNKNOWN);
        std::cin >> vision_range >> home_dist_hor >> home_dist_ver; std::cin.ignore();
        vision_radius = vision_range / 2;
        // std::cerr << vision_range << ' ' << home_dist_hor << ' ' << home_dist_ver << '\n';

        home_x = x + home_dist_ver;
        home_y = y + home_dist_hor;
    }

    void solve() {
        char dir;
        int dir_id = 0;
        init_dstar();

        while (map[x * EDGE + y] != HOME) {
            // gettimeofday(&start, nullptr);
            turn_input();
            update_km();
            compute_path();
            dir = get_move();

            if (dir == '*') {
                std::cout << "Lazzie got lost :c" << std::endl;
                return;
            }

            for (int i = 0; i < DIRS; ++i) {
                if (dir == char_dirs[i]) {
                    dir_id = i;
                    break;
                }
            }

            x += x_dirs[dir_id];
            y += y_dirs[dir_id];

            // gettimeofday(&end, nullptr);
            // std::cerr << (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
            std::cout << dir << std::endl;
        }
    }
};

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    Solver solver;
    solver.init_input();
    solver.solve();

    return 0;
}
