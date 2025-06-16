// LINK: https://www.codingame.com/multiplayer/bot-programming/tower-dereference
#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <filesystem>
#include <fstream>
#include <signal.h>
#include <cstring>

using namespace std;
using namespace filesystem;

using fint = int_fast16_t; // however will be enlarged to 64 bits
using cint = uint64_t; // TOWER CODE INT
using pff = pair<fint, fint>;
using sv = string_view;

// MAP
constexpr sv MAPS_FOLDER = "maps/";
constexpr fint HEIGHT = 17;
constexpr fint WIDTH = 19;
constexpr fint MAX = HEIGHT * WIDTH;
constexpr fint MIN_LEN = 30;
constexpr fint MAX_LEN = 40;
constexpr fint MAX_PATHS_COUNT = 30;
constexpr fint MAX_PATHS_BUFFOR = MAX_LEN * MAX_PATHS_COUNT;
constexpr fint TILE_SIZE = 10;
constexpr fint POSSIBS_COUNT = 3;

constexpr fint DIRS = 4;
constexpr fint DX[4] = { -1, 1, 0, 0 };
constexpr fint DY[4] = { 0, 0, -1, 1 };
enum Dir { UP, DOWN, LEFT, RIGHT };

// STARTING STATS
constexpr fint START_CASH = 350;
constexpr fint START_LIVES = 20;

constexpr fint PLACE_COSTS[4] = { 100, 100, 100, 70 }; // H/F/Gu/Gl
constexpr fint UPGRADE_COSTS[3] = { 50, 100, 150 };

/* 
 * TOWER SORTED LIST CODING:
 *  1 1  | 1 0 0 0 0 0 | 1 0 0 1 0 0 0 0 0 | 1 0 0 1 0 0 0 0 0 |   1    |  1 1  |  1 1  |  1 1  | 1 0 0 0
 *  type |     dist    |      real id      |     board_id      | player | l dmg | l ran | l rel | cooldown
 * 37-36 |    35-30    |       29-21       |       20-12       |   11   | 10-9  |  8-7  |  6-5  | 4-1 
*/
constexpr fint TYPE_SHIFT = 35;
constexpr fint DIST_SHIFT = 29;
constexpr fint REAL_ID_SHIFT = 20;
constexpr fint BOARD_ID_SHIFT = 11;
constexpr fint PLAYER_SHIFT = 10;
constexpr fint L_DAMAGE_SHIFT = 8;
constexpr fint L_RANGE_SHIFT = 6;
constexpr fint L_RELOAD_SHIFT = 4;
constexpr cint DIST_MASK = 0b11111;
constexpr cint ID_MASK = 0b111111111;
constexpr cint PLAYER_MASK = 0b1;
constexpr cint LEVEL_MASK = 0b11;
constexpr cint COOLDOWN_MASK = 0b1111;

// TOWER STATS
constexpr fint TYPES_COUNT = 4;
constexpr fint LEVELS_COUNT = 4;

constexpr fint COOLDOWNS[16] = { 5, 4, 3, 2, // HEAL
                                 8, 7, 6, 5, // FIRE
                                 5, 4, 3, 2, // GUN
                                 4, 3, 2, 1 }; // GLUE

constexpr fint DAMAGES[16] = { 5, 8, 15, 30, // HEAL
                               2, 3, 5, 7, // FIRE
                               5, 8, 15, 30, // GUN
                               8, 15, 25, 40 }; // GLUE

constexpr fint COD_RANGES[16] { 30, 40, 50, 60, // HEAL
                                15, 20, 23, 25, // FIRE
                                30, 40, 50, 60, // GUN
                                30, 40, 50, 60 }; // GLUE

constexpr fint RANGES[16] = { 90'000, 160'000, 250'000, 360'000, // HEAL
                              22'500, 40'000, 52'900, 62'500, // FIRE
                              90'000, 160'000, 250'000, 360'000, // GUN
                              90'000, 160'000, 250'000, 360'000}; // GLUE

constexpr fint SLOW_MULTIPLIER = 4;

// WAVE STATS
constexpr fint WAVE_TIME = 3;
constexpr fint WAVES = 9;
constexpr fint FIRST_WAVE = 1;
constexpr fint MAX_WAVES = 400;
constexpr fint MAX_WAVE_COUNT = 400;
constexpr fint ATTACKERS_BUFFOR = MAX_WAVE_COUNT * 2;

constexpr fint WAVE_STARTS[WAVES] = { 1, 16, 31, 46, 61, 71, 81, 91, 101 };
constexpr fint WAVE_COUNTS[WAVES] = { 4,  4,  5,  6,  6,  7,  7,  8,   8 };
constexpr fint WAVE_HPS[WAVES] = { 5,  8, 12,  8, 12, 15, 15, 18,  18 };
constexpr fint WAVE_SPEEDS[WAVES] = { 10, 12, 10, 15, 10, 10, 12, 13,  14 };
constexpr fint WAVE_BOUNTIES[WAVES] = { 25, 30, 30, 20, 22, 25, 25, 30,  30 };


// SIMULATION STATS
constexpr fint SEC = 1'000'000;
constexpr fint FIRST_TIME_LIMIT = 1'000'500;
constexpr fint TURN_TIME_LIMIT = 500'500;
constexpr fint RAND_MOVES_COUNT = 4;

// MISC
constexpr sv ANSI_CLEAR = "\033[0m";

constexpr sv ANSI_RED = "\033[31m"; // two turrer
constexpr sv ANSI_YELLOW = "\033[33m"; // canyon
constexpr sv ANSI_BLUE = "\033[34m"; // one turret
constexpr sv ANSI_MAGENTA = "\033[35m"; // two attacker
constexpr sv ANSI_CYAN = "\033[36m"; // one attacker

constexpr sv ANSI_BG_RED = "\033[41m";
constexpr sv ANSI_BG_GREEN = "\033[42m";
constexpr sv ANSI_BG_BLUE = "\033[44m";

constexpr fint X_OF[MAX] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
};

constexpr fint Y_OF[MAX] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
};

enum Player { ONE, TWO };
enum Cell { EMPTY, CANYON, TOWER };
enum TowerType { HEAL, FIRE, GUN, GLUE };
enum OptionType { PASS, PLACE, UPGRADE };
enum UpgradeType { DAMAGE, RANGE, RELOAD };

struct Attacker {
    fint pos;
    fint prev;
    fint next;
    fint progress;
    fint health;
    fint max_hp;
    fint speed;
    fint x;
    fint y;
    fint slowed;
    fint bounty;
    fint real_id;
};

struct Future {
    fint p;
    Attacker a;
};

struct BFS_Elem {
    fint id;
    fint path[MAX_LEN];
    fint len;
};

struct Opt {
    fint option;
    fint id;
    fint type;
};

constexpr Opt OPT_PASS = {PASS, 0, 0};

struct Upgradeable {
    fint id;
    fint lvl;
    fint which;
};

struct Pipe {
    int in[2];
    int out[2];
};

struct BotIO {
    Pipe stdio;
    int err_fd;
};

bool make_pipes(Pipe &p) {
    return (pipe(p.in) == 0 && (pipe(p.out) == 0)); 
}

void close_pipes(Pipe &p) {
    close(p.in[0]);
    close(p.in[1]);

    close(p.out[0]);
    close(p.out[1]);
}

void send(int fd, const string &s) {
    ssize_t n, left = s.size();
    const char *ptr = s.data();

    while (left > 0) {
        n = write(fd, ptr, left);

        if (n <= 0) throw runtime_error("write()");

        left -= n;
        ptr += n;
    }
}

string receive(int fd, fint timeout_ms) {
    string ans;
    char buff[4096];
    fd_set rds;
    struct timeval tv;
    ssize_t n;

    FD_ZERO(&rds);
    FD_SET(fd, &rds);

    tv = {timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

    if (select(fd + 1, &rds, nullptr, nullptr, &tv) <= 0)
        throw runtime_error("bot timeout / select()");

    n = read(fd, buff, sizeof(buff) - 1);

    if (n <= 0) throw runtime_error("read()");

    buff[n] = 0;
    ans = buff;

    return ans;
}

void print_err(int fd, string tag) {
    char buff[4096];
    ssize_t n;
    fd_set set;
    timeval tv {0, 0};

    FD_ZERO(&set);
    FD_SET(fd, &set);

    if (select(fd + 1, &set, nullptr, nullptr, &tv) <= 0) return;

    n = read(fd, buff, sizeof(buff) - 1);

    if (n > 0) {
        buff[n] = '\0';
        cerr << ANSI_RED << tag << buff << ANSI_CLEAR;
    }
}

pid_t spawn_bot(const char *exe, Pipe &p, int &err_parent_fd) {
    int err_pipe[2];

    if (!make_pipes(p) || pipe(err_pipe) != 0) {
        perror("pipe");

        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");

        exit(1);
    }

    if (pid == 0) {
        dup2(p.in[0],  STDIN_FILENO);
        dup2(p.out[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);

        close_pipes(p);
        close(err_pipe[0]); close(err_pipe[1]);

        char* const argv[] = { const_cast<char*>(exe), nullptr };

        execvp(argv[0], argv);
        perror("execvp");

        exit(1);
    }

    close(p.in[0]);
    close(p.out[1]);
    close(err_pipe[1]);

    err_parent_fd = err_pipe[0];

    return pid;
}

static u_int64_t seed = 1234567;

uint32_t rand_u32() {
    seed = (0x5DEECE66DUL * seed + 0xBUL) & ((1UL << 48) - 1);
    return static_cast<uint32_t>(seed >> 16);
}

class Game {
private:
    static fint p1_possibs[MAX * POSSIBS_COUNT], p2_possibs[MAX * POSSIBS_COUNT]; 
    static fint p1_pcounts[MAX], p2_pcounts[MAX];
    static fint p1_dists[MAX], p2_dists[MAX];
    static fint belongs[MAX];
    static fint p1_paths[MAX_PATHS_BUFFOR], p2_paths[MAX_PATHS_BUFFOR];
    static fint p1_real_dists[MAX], p2_real_dists[MAX];
    static fint p1_paths_count, p2_paths_count;
    static fint p1_path_len, p2_path_len;
    static fint my_id;
    static fint bases_count;
    static fint p1_bases[2], p2_bases[2];
    static vector<pair<Opt, Opt>> options;
    static vector<pff> wr;
    static fint good_tiles[MAX];
    static fint good_tiles_count;
    static Upgradeable p1_upgradeables[MAX * POSSIBS_COUNT], p2_upgradeables[MAX * POSSIBS_COUNT];
    static fint p1_upgradeable_count, p2_upgradeable_count;
    static fint start_time, end_time;
    static fint placed_towers;
    static fint max_id_spawned;
    fint board[MAX];
    cint towers[MAX];
    fint towers_count;
    fint cash[2];
    fint lives[2];
    fint curr_player;
    Attacker p1_attackers[ATTACKERS_BUFFOR];
    Attacker p2_attackers[ATTACKERS_BUFFOR];
    fint attackers_count[2];
    fint turn;
    fint wave_id;
    fint wave_start;
    fint wave_count;
    fint wave_hp;
    fint wave_speed;
    fint wave_bounty;
    vector<vector<Future>> future_attackers;

    fint opponent(fint player) {
        return player ^ 1;
    }

    cint code_tower(fint type, fint id, fint x, fint y, fint player, fint l_dmg, fint l_ran, fint l_rel, fint cooldown) {
        cint c = cooldown;

        if (type == FIRE) c++;

        c |= l_rel << L_RELOAD_SHIFT;
        c |= l_ran << L_RANGE_SHIFT;
        c |= l_dmg << L_DAMAGE_SHIFT;
        c |= player << PLAYER_SHIFT;
        c |= (x * WIDTH + y) << BOARD_ID_SHIFT;
        c |= id << REAL_ID_SHIFT;
        c |= (player == ONE ? p1_dists : p2_dists)[id];

        return c | type << TYPE_SHIFT;
    }

    void insert_tower(cint code) {
        fint curr = towers_count, parent;

        towers[towers_count++] = code | static_cast<cint>(curr) << REAL_ID_SHIFT;

        while (curr > 0 && towers[(parent = curr - 1)] > towers[curr]) {
            swap(towers[curr], towers[parent]);
            curr = parent;
        }
    }

    void print_bits(cint x) {
        for (fint i = 21; i > 0; --i) {
            cout << ((x & (1 << i)) >> i);
        }

        cout << '\n';
    }

    pff get_tower_coords(cint tower) {
        fint id = get_board_id(tower);
        fint x = X_OF[id], y = Y_OF[id];

        return {x * 100 + 50, y * 100 + 50};
    }

    static bool p1_attacker_comp(const Attacker &a, const Attacker &b) {
        return p1_real_dists[a.pos] * TILE_SIZE - a.progress < p1_real_dists[b.pos] * TILE_SIZE - b.progress;
    }

    static bool p2_attacker_comp(const Attacker &a, const Attacker &b) {
        return p2_real_dists[a.pos] * TILE_SIZE - a.progress < p2_real_dists[b.pos] * TILE_SIZE - b.progress;
    }

    static bool p1_with_p2_attacker_comp(const Attacker &a, const Attacker &b) {
        return p1_real_dists[a.pos] * TILE_SIZE - a.progress < p2_real_dists[b.pos] * TILE_SIZE - b.progress;
    }

    bool out_of_bounds(fint d, fint id) {
        return (d == UP && X_OF[id] == 0) || (d == DOWN && X_OF[id] == HEIGHT - 1) || (d == LEFT && Y_OF[id] == 0) || (d == RIGHT && Y_OF[id] == WIDTH - 1);
    }

    void fill_next_possibs() {
        fint *list, *counts, *dists;
        fint next;

        for (fint p = ONE; p <= TWO; ++p) {
            if (p == ONE) {
                dists = p1_real_dists;
                list = p1_possibs;
                counts = p1_pcounts;
            } else {
                dists = p2_real_dists;
                list = p2_possibs;
                counts = p2_pcounts;
            }

            for (fint i = 0; i < MAX; ++i) {
                if (board[i] != CANYON) continue;

                for (fint d = 0; d < DIRS; ++d) {
                    next = i + DX[d] * WIDTH + DY[d];
                    
                    if (out_of_bounds(d, i)) continue;
                    if (board[next] == CANYON && dists[next] < dists[i]) {
                        list[i * POSSIBS_COUNT + counts[i]++] = next;
                    }
                }
            }
        }

        for (fint i = 0; i < HEIGHT; ++i) {
            p2_pcounts[i * WIDTH] = 0;
            p1_pcounts[i * WIDTH + WIDTH - 1] = 0;
        }

        options.push_back({OPT_PASS, OPT_PASS});

        // for (fint i = 0; i < SIDE; ++i) {
        //     for (fint j = 0; j < SIDE; ++j) {
        //         cout << p1_pcounts[i * SIDE + j] << ' ';
        //     }
        //
        //     cout << endl;
        // }
    }

    void bfs_real_dists() {
        fint *list, *bases;
        fint id, next;
        pff curr;
        fint curr_bases_count;

        for (fint p = ONE; p <= TWO; ++p) {
            curr_bases_count = 0;

            if (p == ONE) {
                id = 1;
                bases = p1_bases;
            } else {
                id = WIDTH - 2;
                bases = p2_bases;
            }

            
            for (fint i = 0; i < HEIGHT; ++i) {
                if (board[id] == CANYON) {
                    bases[curr_bases_count++] = id + (p == ONE ? -1 : 1);
                }

                id += WIDTH;
            }

            bases_count = curr_bases_count;
        }

        for (fint p = ONE; p <= TWO; ++p) {
            queue<pff> q;

            if (p == ONE) {
                list = p1_real_dists;
                bases = p2_bases;
            } else {
                list = p2_real_dists;
                bases = p1_bases;
            }

            fill(list, list + MAX, MAX);

            for (fint i = 0; i < bases_count; ++i) {
                q.emplace(pff{bases[i], 0});
            }

            while (!q.empty()) {
                curr = q.front();
                q.pop();

                id = curr.first;

                if (list[id] <= curr.second) continue;

                list[id] = curr.second;

                for (fint d = 0; d < DIRS; ++d) {
                    next = curr.first + DX[d] * WIDTH + DY[d];
                    
                    if (out_of_bounds(d, id)) continue;
                    if (board[next] == EMPTY) continue;

                    q.push({next, curr.second + 1});
                }
            }

        }

        p1_path_len = p1_real_dists[p1_bases[0]];
        p2_path_len = p2_real_dists[p2_bases[0]];

        // for (fint i = 0; i < SIDE; ++i) {
        //     for (fint j = 0; j < SIDE; ++j) {
        //         cout << setw(3) << p2_real_dists[i * SIDE + j] << ' ';
        //     }
        //
        //     cout << endl;
        // }
    }

    void place_tower(fint player, fint id, fint type) {
        cint code = type == FIRE ? 1 : 0;

        code |= type << TYPE_SHIFT;
        code |= (player == ONE ? p1_dists[id] : p2_dists[id]) << DIST_SHIFT;
        code |= towers_count << REAL_ID_SHIFT;
        code |= id << BOARD_ID_SHIFT;
        code |= player << PLAYER_SHIFT;
        
        // levels & cooldown = 0;

        board[id] = TOWER;

        insert_tower(code);
    }

    void upgrade_tower(fint id, fint stat) {
        cint one = 1 << (stat == DAMAGE ? L_DAMAGE_SHIFT : stat == RANGE ? L_RANGE_SHIFT : L_RELOAD_SHIFT);

        for (fint i = 0; i < towers_count; ++i) {
            if (get_real_id(towers[i]) == id) {
                towers[i] += one;
                return;
            }
        }
    }

    fint get_type(cint c) {
        return c >> TYPE_SHIFT;
    }

    fint get_dist(cint c) {
        return (c >> DIST_SHIFT) & DIST_MASK;
    }

    fint get_real_id(cint c) {
        return (c >> REAL_ID_SHIFT) & ID_MASK;
    }

    fint get_board_id(cint c) {
        return (c >> BOARD_ID_SHIFT) & ID_MASK;
    }

    fint get_player(cint c) {
        return (c >> PLAYER_SHIFT) & PLAYER_MASK;
    }

    fint get_l_damage(cint c) {
        return (c >> L_DAMAGE_SHIFT) & LEVEL_MASK;
    }

    fint get_l_range(cint c) {
        return (c >> L_RANGE_SHIFT) & LEVEL_MASK;
    }

    fint get_l_reload(cint c) {
        return (c >> L_RELOAD_SHIFT) & LEVEL_MASK;
    }

    fint get_cooldown(cint c) {
        return c & COOLDOWN_MASK;
    }

    void fill_starting() {
        fint x, y;

        for (fint i = 0; i < MAX; ++i) {
            x = X_OF[i];
            y = Y_OF[i];

            p1_dists[i] = y;
            p2_dists[i] = WIDTH - 1 - y;

            belongs[i] = (y < 9 ? ONE : (y > 9 ? TWO : (x < 8 ? ONE : TWO)));
        }

        lives[ONE] = START_LIVES;
        lives[TWO] = START_LIVES;
        cash[ONE] = START_CASH;
        cash[TWO] = START_CASH;
        turn = 0;
        towers_count = 0;
        wave_id = 0;
        wave_start = WAVE_STARTS[0];
        wave_count = WAVE_COUNTS[0];
        wave_hp = WAVE_HPS[0];
        wave_speed = WAVE_SPEEDS[0];
        wave_bounty = WAVE_BOUNTIES[0];
        attackers_count[ONE] = 0;
        attackers_count[TWO] = 0;
        placed_towers = 0;
        future_attackers.assign(250, {});
        max_id_spawned = -1;
    }

    // void print_statics() {
    //     for (fint i = 0; i < MAX; ++i) {
    //         cout << setw(2) << p1_dists[i] << (Y_OF[i] == SIDE - 1 ? '\n' : ' ');
    //     }
    //
    //     cout << '\n';
    //
    //     for (fint i = 0; i < MAX; ++i) {
    //         cout << belongs[i] << (i % SIDE == SIDE - 1 ? '\n' : ' ');
    //     }
    //
    //     cout << '\n';
    // }
    //
    void print_board() {
        for (fint i = 0; i < MAX; ++i) {
            cout << board[i] << (Y_OF[i] == WIDTH - 1 ? '\n' : ' ');
        }
    }

    void print_towers() {
        fint c;

        for (fint i = 0; i < towers_count; ++i) {
            c = towers[i];
            cout << "type: " << get_type(c) << " dist: " << get_dist(c) << " id: " << get_real_id(c) << " pos: " << get_board_id(c) << " player: " << get_player(c) << " cooldown: " << get_cooldown(c) << '\n';
        }
    }

    bool shoot(cint c) {
        fint type = get_type(c);
        fint player = get_player(c);
        fint opp = opponent(player);
        fint damage = DAMAGES[type * LEVELS_COUNT + get_l_damage(c)];
        fint range = RANGES[type * LEVELS_COUNT + get_l_range(c)];
        pff pos = get_tower_coords(c);
        fint x = pos.first, y = pos.second;
        Attacker *list;
        bool shot = false;

        if (type == HEAL) {
            swap(player, opp);
        }

        list = player == ONE ? p2_attackers : p1_attackers;

        for (fint i = 0; i < attackers_count[opp]; ++i) {
            if (list[i].health <= 0) {
                continue;
            }

            if ((list[i].x - x) * (list[i].x - x) + (list[i].y - y) * (list[i].y - y) <= range) {
                if (type == GUN) {
                    list[i].health -= damage;
                    return true;
                } else if (type == FIRE) {
                    list[i].health -= damage;
                    shot = true;
                } else if (type == GLUE) {
                    if (list[i].slowed == 0) {
                        list[i].slowed += damage;
                        return true;
                    }
                } else {
                    if (list[i].health == list[i].max_hp) {
                        continue;
                    }

                    list[i].health += damage;

                    if (list[i].health > list[i].max_hp) {
                        list[i].health = list[i].max_hp;
                    }

                    return true;
                }
            }
        }

        return shot;
    }

    void shoot_towers() {
        Attacker *list;
        fint opp;
        cint c;

        for (fint i = 0; i < towers_count; ++i) {
            c = towers[i];

            if (get_cooldown(c) == 0) {
                if (shoot(c)) {
                    towers[i] |= COOLDOWNS[get_type(c) * TYPES_COUNT + get_l_reload(c)];
                }
            } else {
                if (get_type(c) == FIRE && get_cooldown(c) == 1) {
                    if (shoot(c)) {
                        towers[i] -= 1;
                    }
                } else {
                    towers[i] -= 1;
                }
            }
        }

        for (fint p = ONE; p <= TWO; p++) {
            list = p == ONE ? p1_attackers : p2_attackers;
            opp = opponent(p);

            for (fint i = 0; i < attackers_count[p]; ++i) {
                if (list[i].health <= 0) {
                    cash[opp] += list[i].bounty;
                    remove_attacker(p, i);
                    i--;
                }
            }

            if (attackers_count[p] > 1) sort(list, list + attackers_count[p], p == ONE ? p1_attacker_comp : p2_attacker_comp);
        }

    }

    void remove_attacker(fint player, fint id) {
        if (attackers_count[player] == 0) return;

        if (attackers_count[player] == 1) {
            attackers_count[player]--;
            return;
        }

        Attacker *list = (player == ONE ? p1_attackers : p2_attackers);
        swap(list[id], list[--attackers_count[player]]);
    }

    void wave_plus() {
        if (++wave_id >= WAVES) {
            // dynamic next waves
            ++wave_count;
            ++wave_hp;
            wave_start += 10;
        } else {
            wave_count = WAVE_COUNTS[wave_id];
            wave_hp = WAVE_HPS[wave_id];
            wave_speed = WAVE_SPEEDS[wave_id];
            wave_bounty = WAVE_BOUNTIES[wave_id];
            wave_start = WAVE_STARTS[wave_id];
        }
    }

    bool can_create() {
        if (turn >= wave_start) return true;
        
        fint id, prog;

        for (fint p = ONE; p <= TWO; ++p) {
            if (attackers_count[p] == 0) continue;

            id = (p == ONE ? p1_attackers : p2_attackers)[attackers_count[p] - 1].pos;
            prog = (p == ONE ? p1_attackers : p2_attackers)[attackers_count[p] - 1].progress;

            if (((p == ONE ? p1_real_dists[id] : p2_real_dists[id]) * TILE_SIZE + TILE_SIZE - prog) * 3 > (p == ONE ? p1_path_len : p2_path_len) * TILE_SIZE * 2) return false;
        }

        return true;
    }

    Attacker make_attacker(fint p) {
        Attacker a;

        a.progress = rand_u32() % 10;
        a.pos = (p == ONE ? p1_bases : p2_bases)[bases_count == 1 ? 0 : rand_u32() & 1];

        if (a.progress == 0) {
            if (p == ONE) {
                ++a.pos;
            } else {
                --a.pos;
            }
        }

        a.health = wave_hp;
        a.max_hp = wave_hp;
        a.speed = wave_speed;
        a.bounty = wave_bounty;
        a.slowed = 0;
        a.prev = -1;
        a.next = -1;
        a.x = X_OF[a.pos] * 100 + 50;
        a.y = Y_OF[a.pos] * 100;

        if (p == ONE) {
            a.y += a.progress * 10;
        } else {
            a.y -= a.progress * 10;
        }

        a.real_id = ++max_id_spawned;

        return a;
    }

    void create_attackers() {
        fint del;

        for (fint i = 0; i < wave_count; ++i) {
            del = turn + (rand_u32() % WAVE_TIME);
            future_attackers[del].push_back({ONE, make_attacker(ONE)});
            future_attackers[del].push_back({TWO, make_attacker(TWO)});
        }

        wave_plus();
    }

    void spawn_attackers() {
        if (can_create()) create_attackers();

        for (Future &f : future_attackers[turn]) {
            if (f.p == ONE) {
                p1_attackers[attackers_count[ONE]++] = f.a;
            } else {
                p2_attackers[attackers_count[TWO]++] = f.a;
            }
        }

        future_attackers[turn].clear();
    }

    // void spawn_attackers() {
    //     if (turn < wave_start || turn < FIRST_WAVE) {
    //         return;
    //     }
    //
    //     Attacker *list, curr;
    //     fint *bases, *possibs, *pcounts;
    //     fint neigh_x, neigh_y, diff;
    //
    //     for (fint p = ONE; p <= TWO; ++p) {
    //         if (p == ONE) {
    //             list = p1_attackers;
    //             bases = p1_bases;
    //             possibs = p1_possibs;
    //             pcounts = p1_pcounts;
    //         } else {
    //             list = p2_attackers;
    //             bases = p2_bases;
    //             possibs = p2_possibs;
    //             pcounts = p2_pcounts;
    //         }
    //
    //         for (fint i = 0; i < wave_count; ++i) {
    //             curr.progress = rand_u32() % 10;
    //             curr.pos = bases[bases_count == 1 ? 0: rand_u32() % bases_count];
    //             curr.health = wave_hp;
    //             curr.max_hp = wave_hp;
    //             curr.speed = wave_speed;
    //             curr.x = (curr.pos / 10) * 100 + 50;
    //             curr.y = (curr.pos % 10) * 100 + 50;
    //             curr.bounty = wave_bounty;
    //             curr.slowed = 0;
    //             curr.prev = -1;
    //
    //             if (curr.progress > 5) {
    //                 curr.next = possibs[curr.pos * POSSIBS_COUNT + (pcounts[curr.pos] == 1 ? 0 : rand_u32() % pcounts[curr.pos])];
    //                 neigh_x = curr.next / SIDE;
    //                 neigh_y = curr.next % SIDE;
    //                 diff = curr.progress - 5;
    //
    //                 if (neigh_x == curr.x + 1) {
    //                     // DOWN
    //                     curr.x += diff * 10;
    //                 } else if (neigh_x == curr.x - 1) {
    //                     // UP
    //                     curr.x -= diff * 10;
    //                 } else if (neigh_y == curr.y + 1) {
    //                     // RIGHT
    //                     curr.y += diff * 10;
    //                 } else {
    //                     // LEFT
    //                     curr.y -= diff * 10;
    //                 }
    //             } else {
    //                 curr.next = -1;
    //                 diff = 5 - curr.progress;
    //
    //                 if (p == ONE) {
    //                     // RIGHT
    //                     curr.y += diff * 10;
    //                 } else {
    //                     // LEFT
    //                     curr.y -= diff * 10;
    //                 }
    //             }
    //
    //             if (attackers_count[p] >= ATTACKERS_BUFFOR - 1) {
    //                 cerr << "OUT OF BUFFOR ATTACKERS" << endl;
    //                 exit(1);
    //             }
    //
    //             list[attackers_count[p]++] = curr;
    //         }
    //
    //         if (attackers_count[p] > 1) sort(list, list + attackers_count[p], p == ONE ? p1_attacker_comp : p2_attacker_comp);
    //     }
    //
    //     wave_plus();
    // }

    void move_attackers() {
        Attacker *list;
        fint *possibs_list, *counts_list;
        fint count, id, neigh_x, neigh_y, x, y, to_move, progress, diff, id1 = 0, id2 = 0, p, i;
        bool removed;
        
        while (true) {
            if (id2 == attackers_count[TWO] || (id1 < attackers_count[ONE] && p1_with_p2_attacker_comp(p1_attackers[id1], p2_attackers[id2]))) {
                if (id1 == attackers_count[ONE]) {
                    break;
                }

                p = ONE;
                i = id1;
                id1++;
            } else {
                p = TWO;
                i = id2;
                id2++;
            }

            if (p == ONE) {
                list = p1_attackers;
                possibs_list = p1_possibs;
                counts_list = p1_pcounts;
            } else {
                list = p2_attackers;
                possibs_list = p2_possibs;
                counts_list = p2_pcounts;
            }

            id = list[i].pos;
            to_move = list[i].speed;
            progress = list[i].progress;
            removed = false;

            if (list[i].slowed > 0) {
                to_move /= SLOW_MULTIPLIER;
                list[i].slowed--;
            }

            while (progress + to_move >= TILE_SIZE) {
                count = counts_list[id];

                list[i].prev = id;

                if (list[i].next != -1) {
                    id = list[i].next;
                    list[i].next = -1;
                } else {
                    id = possibs_list[id * POSSIBS_COUNT + rand_u32() % count];
                }
                
                list[i].pos = id;
                to_move -= TILE_SIZE - progress;
                progress = 0;

                if ((Y_OF[id] == 0 && p == TWO) || (Y_OF[id] == WIDTH - 1 && p == ONE)) {
                    remove_attacker(p, i);
                    removed = true;
                    break;
                }
            }

            if (removed) {
                if (--lives[opponent(p)] <= 0 && lives[p] <= 0) {
                    return;
                } else {
                    if (p == ONE) {
                        id1--;
                    } else {
                        id2--;
                    }

                    continue;
                }
            }

            progress = to_move;

            list[i].progress = progress;

            x = X_OF[id]; 
            y = Y_OF[id];

            if (progress >= 5) {
                count = counts_list[id];

                if (count > 0) {
                    if (list[i].next == -1) {
                        list[i].next = possibs_list[id * POSSIBS_COUNT + rand_u32() % count];
                    }

                    neigh_x = X_OF[list[i].next];
                    neigh_y = Y_OF[list[i].next];
                } else {
                    neigh_x = X_OF[id];
                    neigh_y = p == ONE ? WIDTH - 1: 0;
                }

                diff = progress - 5;
            } else {
                if (list[i].prev == -1) {
                    neigh_x = x;
                    neigh_y = p == ONE ? 0 : WIDTH - 1;
                } else {
                    neigh_x = X_OF[list[i].prev];
                    neigh_y = Y_OF[list[i].prev];
                }

                diff = 5 - progress;
            }

            list[i].x = x * 100 + 50;
            list[i].y = y * 100 + 50;

            if (neigh_x == x + 1) {
                // DOWN
                list[i].x += diff * 10;
            } else if (neigh_x == x - 1) {
                // UP
                list[i].x -= diff * 10;
            } else if (neigh_y == y + 1) {
                // RIGHT
                list[i].y += diff * 10;
            } else {
                // LEFT
                list[i].y -= diff * 10;
            }
        }

        if (attackers_count[ONE] > 1) sort(p1_attackers, p1_attackers + attackers_count[ONE], p1_attacker_comp);
        if (attackers_count[TWO] > 1) sort(p2_attackers, p2_attackers + attackers_count[TWO], p2_attacker_comp);
    }

    // void old_move_attackers() {
    //     Attacker *list;
    //     fint curr_x, curr_y, neigh_x, neigh_y, id, len, prog, on_list, neigh, diff;
    //     fint *paths;
    //
    //     for (fint p = ONE; p <= TWO; ++p) {
    //         list = p == ONE ? p1_attackers : p2_attackers;
    //         len = p == ONE ? p1_path_len : p2_path_len;
    //         paths = p == ONE ? p1_paths : p2_paths;
    //
    //         for (fint i = 0; i < attackers_count[p]; ++i) {
    //             if (list[i].slowed > 0) {
    //                 list[i].progress += list[i].speed / SLOW_MULTIPLIER;
    //                 list[i].slowed--;
    //             } else {
    //                 list[i].progress += list[i].speed;
    //             }
    //
    //             prog = list[i].progress;
    //
    //             if (list[i].progress >= len * 10) {
    //                 remove_attacker(p, i--);
    //
    //                 if (lives[p] > 0) lives[p]--;
    //             } else {
    //                 on_list = list[i].path_id * len + prog / 10;
    //                 id = paths[on_list];
    //
    //                 curr_x = id / 10;
    //                 curr_y = id % 10;
    //                 list[i].x = curr_x * 100 + 50;
    //                 list[i].y = curr_y * 100 + 50;
    //
    //                 if (id == len - 1 || id == 0) {
    //                     neigh_x = curr_x;
    //                     neigh_y = curr_y == 1 ? 0 : SIDE - 1;
    //                 } else {
    //                     diff = prog % 10;
    //
    //                     if (diff >= 5) {
    //                         neigh = paths[on_list + 1];
    //                         diff -= 5;
    //                     } else {
    //                         neigh = paths[on_list - 1];
    //                         diff = 5 - diff;
    //                     }
    //
    //                     neigh_x = neigh / 10;
    //                     neigh_y = neigh % 10;
    //                 }
    //
    //                 if (neigh_x == curr_x + 1) {
    //                     // DOWN
    //                     list[i].x += diff * 10;
    //                 } else if (neigh_x == curr_x - 1) {
    //                     // UP
    //                     list[i].x -= diff * 10;
    //                 } else if (neigh_y == curr_y + 1) {
    //                     // RIGHT
    //                     list[i].y += diff * 10;
    //                 } else {
    //                     // LEFT
    //                     list[i].y -= diff * 10;
    //                 }
    //             }
    //         }
    //
    //         sort(list, list + attackers_count[p], attacker_comp);
    //     }
    // }

    void fill_upgradeables() {
        cint curr;
        fint id, lvl;
        Upgradeable *upgradeables;
        fint *upgradeable_count;

        p1_upgradeable_count = 0;
        p2_upgradeable_count = 0;

        for (fint i = 0; i < towers_count; ++i) {
            curr = towers[i];
            id = get_real_id(curr);

            if (get_player(curr) == ONE) {
                upgradeables = p1_upgradeables;
                upgradeable_count = &p1_upgradeable_count;
            } else {
                upgradeables = p2_upgradeables;
                upgradeable_count = &p2_upgradeable_count;
            }

            if ((lvl = get_l_damage(curr)) < LEVELS_COUNT - 1) {
                upgradeables[(*upgradeable_count)++] = Upgradeable{id, lvl, DAMAGE};
            }

            if ((lvl = get_l_range(curr)) < LEVELS_COUNT - 1) {
                upgradeables[(*upgradeable_count)++] = Upgradeable{id, lvl, RANGE};
            }

            if ((lvl = get_l_reload(curr)) < LEVELS_COUNT - 1) {
                upgradeables[(*upgradeable_count)++] = Upgradeable{id, lvl, RELOAD};
            }
        }
    }

    void generate_options() {
        fint new_cash, id, id2;
        Upgradeable *upgradeables;
        fint upgradeable_count;

        options.resize(1); // ONLY LEAVE OPT_PASS;

        if (my_id == ONE) {
            upgradeables = p1_upgradeables;
            upgradeable_count = p1_upgradeable_count;
        } else {
            upgradeables = p2_upgradeables;
            upgradeable_count = p2_upgradeable_count;
        }

        for (fint type = 0; type < TYPES_COUNT; ++type) {
            if (type != GUN) continue;
            new_cash = cash[my_id] - PLACE_COSTS[type];

            if (new_cash < 0) {
                continue;
            }

            for (fint i = 0; i < good_tiles_count; ++i) {
                id = good_tiles[i];

                if (board[id] == EMPTY) {
                    options.push_back({Opt{PLACE, id, type}, OPT_PASS});

                    for (fint type2 = 0; type2 < TYPES_COUNT; ++type2) {
                        if (type2 != GUN) continue;
                        if (new_cash >= PLACE_COSTS[type2]) {
                            for (fint j = i + 1; j < good_tiles_count; ++j) {
                                id2 = good_tiles[j];

                                if (board[id2] == EMPTY) {
                                    options.push_back({Opt{PLACE, id, type}, Opt{PLACE, id2, type2}});
                                }
                            }
                        }
                    }

                    for (fint j = 0; j < upgradeable_count; ++j) {
                        if (new_cash >= UPGRADE_COSTS[upgradeables[j].lvl]) {
                            options.push_back({Opt{PLACE, id, type}, Opt{UPGRADE, upgradeables[j].id, upgradeables[j].which}});
                        }
                    }
                }
            }
        }

        for (fint i = 0; i < upgradeable_count; ++i) {
            new_cash = cash[my_id] - UPGRADE_COSTS[upgradeables[i].lvl];
            
            if (new_cash < 0) {
                continue;
            }

            options.push_back({Opt{UPGRADE, upgradeables[i].id, upgradeables[i].which}, OPT_PASS});

            for (fint j = i + 1; j < upgradeable_count; ++j) {
                if (new_cash >= UPGRADE_COSTS[upgradeables[j].lvl]) {
                    options.push_back({Opt{UPGRADE, upgradeables[i].id, upgradeables[i].which}, Opt{UPGRADE, upgradeables[j].id, upgradeables[j].which}});
                }
            }
        }

        cerr << options.size() << endl;
    }

    void apply_opts(const pair<Opt, Opt> &opts) {
        if (opts.first.type == PASS) {
            return;
        }

        if (opts.first.type == PLACE) {
            place_tower(my_id, opts.first.id, opts.first.type);
        } else {
            upgrade_tower(opts.first.id, opts.first.type);
        }

        if (opts.second.type == PASS) {
            return;
        }

        if (opts.second.type == PLACE) {
            place_tower(my_id, opts.second.id, opts.second.type);
        } else {
            upgrade_tower(opts.second.id, opts.second.type);
        }
    }

    void random_action(fint p) {
        if (cash[p] < PLACE_COSTS[GLUE]) {
            return;
        }

        fint posss[MAX];
        fint count = 0, type;

        for (fint i = 0; i < good_tiles_count; ++i) {
            if (board[good_tiles[i]] == EMPTY) {
                posss[count++] = good_tiles[i];
            }
        }

        if (count > 0) {
            if (cash[p] >= PLACE_COSTS[GUN]) {
                type = rand_u32() % TYPES_COUNT;
            } else {
                type = GLUE;
            }
            place_tower(p, posss[rand_u32() % count], type);

            cash[p] -= PLACE_COSTS[type];
        } else if ((p == ONE ? p1_upgradeable_count : p2_upgradeable_count) > 0) {
            Upgradeable pick = p == ONE ? p1_upgradeables[rand_u32() % p1_upgradeable_count] : p2_upgradeables[rand_u32() % p2_upgradeable_count];
            fint cost = UPGRADE_COSTS[pick.lvl];

            if (cash[p] >= cost) {
                upgrade_tower(pick.id, pick.which);

                cash[p] -= cost;
            }
        }
    }

    void pick_map() {
        vector<path> files;
        path sel;
        string line;

        for (const auto &entry : directory_iterator(MAPS_FOLDER)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            cerr << "no map files found in " << MAPS_FOLDER << endl;

            exit(1);
        }

        sel = files[rand_u32() % files.size()];
        ifstream fin(sel);

        if (!fin) {
            cerr << "could not open file " << sel << endl;
            
            exit(1);
        }

        for (fint i = 0; i < HEIGHT; ++i) {
            if (!(fin >> line) || line.size() < HEIGHT) {
                cerr << "file too short " << sel << endl;

                exit(1);
            }

            for (fint j = 1; j < WIDTH - 1; ++j) {
                board[i * WIDTH + j] = line[j - 1] == '#' ? EMPTY : CANYON;
            }

            board[i * WIDTH] = board[i * WIDTH + 1];
            board[i * WIDTH + WIDTH - 1] = board[i * WIDTH + WIDTH - 2];
        }
    }

    Opt parse_one(sv s) {
        string cmd, a;
        stringstream inp{string{s}};
        fint id, type;

        inp >> cmd;

        if (cmd.empty() || cmd == "PASS") return OPT_PASS;

        if (cmd == "BUILD") {
            fint x, y;

            inp >> x >> y >> a;

            if (!inp) {
                cerr << "wrong format";
                return OPT_PASS;
            }

            if (y < 0 || y >= HEIGHT || x < 0 || x >= HEIGHT) {
                cerr << "out of board" << endl;
                return OPT_PASS;
            }

            id = y * WIDTH + x + 1;

            if (a == "HEALTOWER") {
                type = HEAL;
            } else if (a == "FIRETOWER") {
                type = FIRE;
            } else if (a == "GUNTOWER") {
                type = GUN;
            } else if (a == "GLUETOWER") {
                type = GLUE;
            } else {
                cerr << "incorrect type" << endl;
                return OPT_PASS;
            }

            return {PLACE, id, type};
        }

        if (cmd == "UPGRADE") {
            inp >> id >> a;

            if (!inp) {
                cerr << "wrong format";
                return OPT_PASS;
            }

            if (id < 0 || id >= placed_towers) {
                cerr << "no tower with such id : " << id << endl;
                return OPT_PASS;
            }

            if (a == "DAMAGE") {
                type = DAMAGE;
            } else if (a == "RANGE") {
                type = RANGE;
            } else if (a == "RELOAD") {
                type = RELOAD;
            } else {
                cerr << "incorrect type" << endl;
                return OPT_PASS;
            }

            return {UPGRADE, id, type};
        }

        cerr << "incorrent option" << endl;
        return OPT_PASS;
    }

    vector<Opt> parse(const string &line) {
        vector<Opt> res;
        uint64_t start = 0, end, len;

        while (true) {
            end = line.find(';', start);

            if (end == string::npos) break;

            len = end - start;

            if (len) res.push_back(parse_one(sv(line.data() + start, len)));

            start = end + 1;
        }

        return res;
    }

    void apply_single(const Opt &o, fint p) {
        if (o.option == PASS) return;

        if (o.option == PLACE) {
            if (o.type < 0 || o.type >= TYPES_COUNT) {
                cerr << "wrong type" << endl;
                return;
            } else if (o.id < 0 || o.id >= MAX) {
                cerr << "placing out of board" << endl;
                return;
            } else if (board[o.id] != EMPTY) {
                cerr << "place occupied" << endl;
                return;
            }

            fint cost = PLACE_COSTS[o.type];

            if (cash[p] < cost) {
                cerr << "insufficient funds" << endl;
                return;
            }

            place_tower(p, o.id, o.type);
            cash[p] -= cost;

            return;
        }

        if (o.option == UPGRADE) {
            fint lvl;
            cint c;

            for (fint i = 0; i < towers_count; ++i) {
                c = towers[i];

                if (get_real_id(c) == o.id) {
                    if (get_player(c) != p) {
                        cerr << "not your tower" << endl;
                        return;
                    }

                    switch(o.type) {
                    case DAMAGE:
                        lvl = get_l_damage(c);
                        break;
                    case RANGE:
                        lvl = get_l_range(c);
                        break;
                    case RELOAD:
                        lvl = get_l_reload(c);
                        break;
                    default:
                        cerr << "wrong type" << endl;
                        return;
                    }

                    if (lvl >= LEVELS_COUNT - 1) {
                        cerr << "already upgraded" << endl;
                        return;
                    }

                    fint cost = UPGRADE_COSTS[lvl];

                    if (cash[p] < cost) {
                        cerr << "insufficient funds" << endl;
                        return;
                    }

                    upgrade_tower(o.id, o.type);
                    cash[p] -= cost;

                    return;
                }
            }

            cerr << "tower not found" << endl;

            return;
        }

        cerr << "wrong option" << endl;

        return;
    }

public:
    void init() {
        pick_map();
        fill_starting();
        bfs_real_dists();
        fill_next_possibs();
    }

    void one_turn() {
        shoot_towers();
        move_attackers();
        spawn_attackers();

        turn++;
    }

    string txt_starting(fint p) {
        stringstream res;

        res << p << '\n' << HEIGHT << ' ' << HEIGHT << '\n';

        for (fint i = 0; i < HEIGHT; ++i) {
            for (fint j = 1; j < WIDTH - 1; ++j) {
                res << (board[i * WIDTH + j] == EMPTY ? '#' : '.');
            }

            res << '\n';
        }

        return res.str();
    }

    string txt_turn(fint player) {
        stringstream res;
        fint type, id, owner, y, x, damage, reload, cooldown, id1 = 0, id2 = 0, opp = opponent(player), range;
        string type_str;
        float fx, fy;
        cint c;
        Attacker curr;

        res << cash[player] << ' ' << lives[player] << '\n';
        res << cash[opp] << ' ' << lives[opp] << '\n';

        res << towers_count << '\n';

        if (towers_count > 1) sort(towers, towers + towers_count, [this](const cint &a, const cint &b) {
            return get_real_id(a) < get_real_id(b);
        });

        for (fint i = 0; i < towers_count; ++i) {
            c = towers[i];

            type = get_type(c);
            id = get_real_id(c);
            owner = get_player(c);
            x = X_OF[get_board_id(c)];
            y = Y_OF[get_board_id(c)] - 1;
            damage = DAMAGES[type * LEVELS_COUNT + get_l_damage(c)];
            range = COD_RANGES[type * LEVELS_COUNT + get_l_range(c)];
            reload = COOLDOWNS[type * LEVELS_COUNT + get_l_reload(c)];
            cooldown = get_cooldown(c);

            switch(type) {
            case GUN:
                type_str = "GUNTOWER";
                break;
            case GLUE:
                type_str = "GLUETOWER";
                break;
            case HEAL:
                type_str = "HEALTOWER";
                break;
            case FIRE:
                type_str = "FIRETOWER";
                break;
            }

            res << type_str << ' ' << id << ' ' << owner << ' ' << y << ' ' << x << ' ' << damage << ' ' << range / 10 << '.' << range % 10 << ' ' << reload << ' ' << cooldown << '\n';
        }

        if (towers_count > 1) sort(towers, towers + towers_count);

        res << attackers_count[ONE] + attackers_count[TWO] << '\n';

        while (true) {
            if (id2 == attackers_count[TWO] || (id1 < attackers_count[ONE] && p1_with_p2_attacker_comp(p1_attackers[id1], p2_attackers[id2]))) {
                if (id1 == attackers_count[ONE]) {
                    break;
                }

                curr = p1_attackers[id1++];
                owner = ONE;
            } else {
                curr = p2_attackers[id2++];
                owner = TWO;
            }

            fx = static_cast<float>(curr.x) / 100.0f;
            fy = static_cast<float>(curr.y - TILE_SIZE * TILE_SIZE) / 100.0f;

            res << curr.real_id << ' ' << owner << ' ' << fy << ' ' << fx << ' ' << curr.health << ' ' << curr.max_hp << ' ' << static_cast<float>((curr.slowed > 0 ? curr.speed / SLOW_MULTIPLIER : curr.speed)) / 10.0 << ' ' << static_cast<float>(curr.speed) / 10.0 << ' ' << curr.slowed << ' ' << curr.bounty << '\n';
        }

        return res.str();
    }

    fint get_turn() {
        return turn;
    }

    bool finished() {
        return lives[ONE] <= 0 || lives[TWO] <= 0;
    }

    fint get_winner() {
        if (lives[ONE] <= 0) {
            if (lives[TWO] <= 0) {
                return -1;
            }

            return TWO;
        }

        return ONE;
    }

    void pretty_print() {
        fint p1_counts[MAX] = {0}, p2_counts[MAX] = {0}, owners[MAX] = {0};
        fint pos, p1_closest = MAX, p2_closest = MAX, max_dist;
        fint *clist, *dists_list, *best;
        Attacker *list;

        for (fint p = ONE; p <= TWO; ++p) {
            if (p == ONE) {
                list = p1_attackers;
                clist = p1_counts;
                dists_list = p1_real_dists;
                best = &p1_closest;
            } else {
                list = p2_attackers;
                clist = p2_counts;
                dists_list = p2_real_dists;
                best = &p2_closest;
            }

            for (fint i = 0; i < attackers_count[p]; ++i) {
                pos = list[i].pos;

                clist[pos]++;

                if (dists_list[pos] < *best) {
                    *best = dists_list[pos];
                }
            }
        }

        for (fint i = 0; i < towers_count; ++i) {
            owners[get_board_id(towers[i])] = get_player(towers[i]);
        }
        
        for (fint i = 0; i < HEIGHT; ++i) {
            for (fint r = 0; r < 3; ++r) {
                cout << ANSI_CLEAR;

                if (r == 1) {
                    cout << setw(2) << i << "   ";
                } else {
                    cout << "     ";
                }

                for (fint j = 0; j < WIDTH; ++j) {
                    pos = i * WIDTH + j;

                    if (board[pos] == CANYON) {
                        if (r == 1) {
                            if (p1_counts[pos] == 0) {
                                cout << ANSI_YELLOW << (j == 0 || j == WIDTH - 1 ? ' ' : '.');
                            } else {
                                cout << ANSI_CYAN << (p1_counts[pos] > 9 ? '+' : p1_counts[pos]);
                            }

                            cout << ANSI_YELLOW << (j == 0 || j == WIDTH - 1 ? ' ' : '.');

                            if (p2_counts[pos] == 0) {
                                cout << ANSI_YELLOW << (j == 0 || j == WIDTH - 1 ? ' ' : '.');
                            } else {
                                cout << ANSI_MAGENTA << (p2_counts[pos] > 9 ? '+' : p2_counts[pos]);
                            }
                        } else {
                            cout << ANSI_YELLOW << (j == 0 || j == WIDTH - 1 ? "   " :  "...");
                        }
                    } else if (board[pos] == TOWER) {
                        cout << (owners[pos] == ONE ? ANSI_BLUE : ANSI_RED);

                        if (r == 1) {
                            cout << "*󰚁*";
                        } else {
                            cout << "***";
                        }
                    } else if (j == 0 || j == WIDTH - 1) {
                        cout << ANSI_CLEAR << "   ";
                    } else {
                        cout << ANSI_CLEAR << "###";
                    }

                    cout << ' ';
                }

                cout << '\n';
            }
        }

        cout << ANSI_BLUE << "\nP1:\nCASH: $" << cash[ONE] << "\nLIVES: " << lives[ONE] << "\nPROGRESS: ";

        max_dist = p1_real_dists[p1_bases[0]];

        for (fint i = 0; i < max_dist; ++i) {
            cout << (i <= max_dist - p1_closest ? '#' : '-');
        }

        cout << ANSI_RED << "\nP2:\nCASH: $" << cash[TWO] << "\nLIVES: " << lives[TWO] << "\nPROGRESS: ";

        max_dist = p2_real_dists[p2_bases[0]];

        for (fint i = 0; i < max_dist; ++i) {
            cout << (i <= max_dist - p2_closest ? '#' : '-');
        }

        cout << ANSI_CLEAR << endl;
    }

    void apply(string cmd0, string cmd1) {
        placed_towers = towers_count;

        vector<Opt> v0 = parse(cmd0);
        vector<Opt> v1 = parse(cmd1);
        Opt opt0, opt1;
        fint max_opts = max(v0.size(), v1.size());

        for (fint i = 0; i < max_opts; ++i) {
            opt0 = (i < static_cast<fint>(v0.size()) ? v0[i] : OPT_PASS);
            opt1 = (i < static_cast<fint>(v1.size()) ? v1[i] : OPT_PASS);

            if (opt0.option == PLACE && opt1.option == PLACE && opt0.id == opt1.id) {
                if (belongs[opt0.id] == ONE) {
                    apply_single(opt0, ONE);
                } else {
                    apply_single(opt1, TWO);
                }
            } else {
                apply_single(opt0, ONE);
                apply_single(opt1, TWO);
            }
        }

        if (towers_count > 1) sort(towers, towers + towers_count);

        cout << "P1: " << cmd0;
        cout << "P2: " << cmd1 << endl;
    }
};

fint Game::p1_dists[MAX];
fint Game::p2_dists[MAX];
fint Game::belongs[MAX];
fint Game::p1_paths[MAX_PATHS_BUFFOR];
fint Game::p2_paths[MAX_PATHS_BUFFOR];
fint Game::p1_path_len;
fint Game::p2_path_len;
fint Game::p1_paths_count;
fint Game::p2_paths_count;
fint Game::p1_bases[2];
fint Game::p2_bases[2];
fint Game::my_id;
fint Game::bases_count;
fint Game::good_tiles[MAX];
fint Game::good_tiles_count;
Upgradeable Game::p1_upgradeables[MAX * POSSIBS_COUNT];
Upgradeable Game::p2_upgradeables[MAX * POSSIBS_COUNT];
fint Game::p1_upgradeable_count;
fint Game::p2_upgradeable_count;
fint Game::start_time;
fint Game::end_time;
vector<pair<Opt, Opt>> Game::options;
vector<pff> Game::wr;
fint Game::p1_possibs[MAX * POSSIBS_COUNT];
fint Game::p2_possibs[MAX * POSSIBS_COUNT];
fint Game::p1_pcounts[MAX];
fint Game::p2_pcounts[MAX];
fint Game::p1_real_dists[MAX];
fint Game::p2_real_dists[MAX];
fint Game::placed_towers;
fint Game::max_id_spawned;

int main(int argc, char* argv[]) {
    if (argc != 3 || argc > 4) {
        cerr << "usage: referee bot1 bot2" << endl;//[optional: n simuls]" << endl;

        return 1;
    }

    size_t pos;
    fint n = argc == 3 ? 1 : stol(argv[3], &pos);

    if (argc == 4 && (pos != strlen(argv[3]) || n <= 0)) {
        cerr << "n should be an integer > 0";
        
        return 1;
    }
    
    fint wins0 = 0, wins1 = 0, draws = 0;

    for (fint i = 0; i < n; ++i) {
        BotIO bots[2];
        pid_t pids[2];
        Game game;
        string str;
        string cmd0, cmd1;
        bool reversed = rand_u32() & 1;
        fint winner_id, timeout;

        game.init();

        pids[0] = spawn_bot(argv[1], bots[0].stdio, bots[0].err_fd);
        pids[1] = spawn_bot(argv[2], bots[1].stdio, bots[1].err_fd);

        cmd0 = game.txt_starting(ONE);
        cmd1 = game.txt_starting(TWO);

        if (reversed) {
            swap(cmd0, cmd1);
        }

        send(bots[0].stdio.in[1], cmd0);
        send(bots[1].stdio.in[1], cmd1);

        while (!game.finished()) {
            cout << ANSI_BG_GREEN;

            if (n > 1) {
                cout << "----- GAME #" << i;
            }

            cout << "----- TURN #" << game.get_turn() << " -----\n" << ANSI_CLEAR << endl;

            cmd0 = game.txt_turn(ONE);
            cmd1 = game.txt_turn(TWO);

            if (reversed) {
                swap(cmd0, cmd1);
            }

            timeout = game.get_turn() == 0 ? FIRST_TIME_LIMIT : TURN_TIME_LIMIT;

            send(bots[0].stdio.in[1], cmd0);
            send(bots[1].stdio.in[1], cmd1);

            cmd0 = receive(bots[0].stdio.out[0], timeout);
            cmd1 = receive(bots[1].stdio.out[0], timeout);

            if (reversed) {
                swap(cmd0, cmd1);
            }

            print_err(bots[reversed ? 1 : 0].err_fd, "[BOT P0] ");
            print_err(bots[reversed ? 0 : 1].err_fd, "[BOT P1] ");

            game.apply(cmd0, cmd1);

            game.one_turn();

            game.pretty_print();
            cout << "\n\n\n";
        }

        winner_id = game.get_winner();

        if (winner_id == -1) {
            cout << ANSI_BG_GREEN << "DRAW";
            draws++;
        } else {
            cout << "WINNER: ";

            if (winner_id == ONE) {
                cout << ANSI_BG_BLUE << "P0 - ";
                cout << (reversed ? argv[2] : argv[1]);

                if (reversed) {
                    wins1++;
                } else {
                    wins0++;
                }
            } else {
                cout << ANSI_BG_RED << "P1 - ";
                cout << (reversed ? argv[1] : argv[2]);

                if (reversed) {
                    wins0++;
                } else {
                    wins1++;
                }
            }
        }

        cout << ANSI_CLEAR << " !!!\n" << endl;

        kill(pids[0], SIGTERM);
        kill(pids[1], SIGTERM);
    }

    if (n > 1) cout << ANSI_BG_BLUE << argv[1] << "'s wins: " << wins0 << " draws: " << draws << ' ' << argv[2] << "'s wins: " << wins1 << '\n' << ANSI_CLEAR << endl;

    return 0;
}
