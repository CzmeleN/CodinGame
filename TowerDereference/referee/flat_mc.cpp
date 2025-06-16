// LINK: https://www.codingame.com/multiplayer/bot-programming/tower-dereference
#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <sys/time.h>

using namespace std;
using fint = int_fast16_t; // however will be enlarged to 64 bits
using cint = uint64_t; // TOWER CODE INT
using pff = pair<fint, fint>;

// MAP
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
constexpr fint FIRST_TIME_LIMIT = 999'900;
constexpr fint TURN_TIME_LIMIT = 49'900;
constexpr fint RAND_MOVES_COUNT = 4;

// MISC
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

struct Upgradeable {
    fint id;
    fint lvl;
    fint which;
};

constexpr Opt OPT_PASS = {PASS, 0, 0};

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
    static fint max_id_prev;
    static fint max_id_curr;
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
    fint spawning_left;
    bool started;
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
        return p1_real_dists[a.pos] * 10 - a.progress < p1_real_dists[b.pos] * 10 - b.progress;
    }

    static bool p2_attacker_comp(const Attacker &a, const Attacker &b) {
        return p2_real_dists[a.pos] * 10 - a.progress < p2_real_dists[b.pos] * 10 - b.progress;
    }

    static bool p1_with_p2_attacker_comp(const Attacker &a, const Attacker &b) {
        return p1_real_dists[a.pos] * 10 - a.progress < p2_real_dists[b.pos] * 10 - b.progress;
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

        cerr << p1_pcounts[p2_bases[0] - 1] << p1_pcounts[p2_bases[1] - 1] << endl;
        cerr << p2_pcounts[p1_bases[0] + 1] << p2_pcounts[p1_bases[1] + 1] << endl;

        options.push_back({OPT_PASS, OPT_PASS});

        // for (fint i = 0; i < HEIGHT; ++i) {
        //     for (fint j = 0; j < WIDTH; ++j) {
        //         cerr << p1_pcounts[i * WIDTH + j] << ' ';
        //     }
        //
        //     cerr << endl << endl;
        // }
        //
        // for (fint i = 0; i < HEIGHT; ++i) {
        //     for (fint j = 0; j < WIDTH; ++j) {
        //         cerr << p2_pcounts[i * WIDTH + j] << ' ';
        //     }
        //
        //     cerr << endl;
        // }
    }

    void fill_good_tiles() {
        fint next;

        for (fint i = 0; i < MAX; ++i) {
            if (Y_OF[i] == 0 || Y_OF[i] == WIDTH - 1) continue;

            if (board[i] != EMPTY) {
                // cout << " . ";
                // if (i % SIDE == SIDE-  1) cout << endl;

                continue;
            }

            for (fint d = 0; d < DIRS; ++d) {
                next = i + DX[d] * WIDTH + DY[d];
                
                if (out_of_bounds(d, i)) continue;
                if (Y_OF[next] == 0 || Y_OF[next] == WIDTH - 1) continue;
                if (board[next] == EMPTY) continue;

                good_tiles[good_tiles_count++] = i;
                d = DIRS;
            }

            // if (good_tiles[good_tiles_count - 1] == i) cout << " G "; else {
            //     cout << " b ";
            // }
            //
            // if (i % SIDE == SIDE-  1) cout << endl;
        }
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
        future_attackers.assign(250, {});
        max_id_prev = -1;
        max_id_curr = -1;
        spawning_left = WAVE_TIME;
    }

    // void print_statics() {
    //     for (fint i = 0; i < MAX; ++i) {
    //         cout << setw(2) << p1_dists[i] << (i % SIDE == SIDE - 1 ? '\n' : ' ');
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

    void provide_data() {
        return;
    }

    void get_input() {
        return;
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

        spawning_left = WAVE_TIME;
        started = false;
    }

    bool can_create() {
        if (started) return false;

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

        return a;
    }

    void create_attackers() {
        fint del;

        for (fint i = 0; i < wave_count; ++i) {
            del = turn + (rand_u32() % WAVE_TIME);
            future_attackers[del].push_back({ONE, make_attacker(ONE)});
            future_attackers[del].push_back({TWO, make_attacker(TWO)});
        }

        started = true;
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

        if (started) {
            if (--spawning_left == 0) wave_plus();
        }

        future_attackers[turn].clear();
    }

    void update_futures() {
        fint del;

        for (fint t = turn; t < turn + spawning_left; ++t) {
            future_attackers[t].clear();
        }

        for (fint i = 0; i < wave_count; ++i) {
            del = turn + (rand_u32() % spawning_left);
            future_attackers[del].push_back({ONE, make_attacker(ONE)});
            future_attackers[del].push_back({TWO, make_attacker(TWO)});
        }
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
    //             curr.pos = bases[bases_count == 1 ? 0 : rand_u32() % bases_count];
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
    //     // Attacker *list;
    //     // Attacker stats = get_stats();
    //     // fint count = get_count();
    //     //
    //     // for (fint p = ONE; p <= TWO; ++p) {
    //     //     list = p == ONE ? p1_attackers : p2_attackers;
    //     //
    //     //     for (fint c = 0; c < count; ++c) {
    //     //         list[attackers_count[p]++] = stats;
    //     //         list[attackers_count[p]].path_id = rand_u32() % (p == ONE ? p1_paths_count : p2_paths_count);
    //     //         list[attackers_count[p]].pos = (p == ONE ? p1_paths : p2_paths)[0];
    //     //     }
    //     // }
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

    pair<Opt, Opt> best_option() {
        fint id = 0, count = options.size(), best_id = 0, simuls = 0, won = 0, res, draws = 0;
        float best = -1.0, curr_wr;
        Game curr;
        struct timeval tv;

        wr.resize(count, {0, 0});
        
        while (start_time < end_time) {
            curr = *this;
            curr.apply_opts(options[id]);
            res = curr.random_game();

            if (res == 1) {
                ++wr[id].first;
                draws++;
            } else if (res == 2) {
                wr[id].first += 2;
                won++;
            }

            wr[id].second += 2;
            simuls++;

            if (++id == count) {
                id = 0;
            }

            gettimeofday(&tv, nullptr);
            start_time = tv.tv_sec * SEC + tv.tv_usec;
        }


        for (fint i = 0; i < simuls && i < count; ++i) {
            curr_wr = static_cast<float>(wr[i].first) / static_cast<float>(wr[i].second);

            if (curr_wr > best) {
                best = curr_wr;
                best_id = i;
            }
        }

        cerr << "Simulated: " << simuls << " times" << '\n';
        cerr << "W: " << won << " D: " << draws << " L: " << simuls - won - draws << endl;

        return options[best_id];
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

    fint random_game() {
        fint opp = opponent(my_id);
        bool first = true;

        while (true) {
            fill_upgradeables();

            for (fint i = 0; i < RAND_MOVES_COUNT; ++i) {
                if (!first) {
                    random_action(my_id);
                }

                random_action(opp);
            }
            
            first = false;

            shoot_towers();
            move_attackers();

            if (lives[my_id] <= 0) {
                if (lives[opp] <= 0) {
                    return 1;
                }
                // cerr << "lost! len: " << turn << endl;
                // cerr << "cash: " << cash[opp] << " towers: " << towers_count << endl;
                return 0;
            } else if (lives[opp] <= 0) {
                // cerr << "won! len: " << turn << endl;
                // cerr << "cash: " << cash[opp] << " towers: " << towers_count << endl;
                return 2;
            }

            spawn_attackers();

            turn++;
        }
    }

    void print_opt(Opt o) {
        if (o.option == PASS) {
            return;
        }

        if (o.option == PLACE) {
            cout << "BUILD " << Y_OF[o.id] - 1 << ' ' << X_OF[o.id] << ' ';

            switch (o.type) {
            case GUN:
                cout << "GUNTOWER";
                break;
            case GLUE:
                cout << "GLUETOWER";
                break;
            case HEAL:
                cout << "HEALTOWER";
                break;
            case FIRE:
                cout << "FIRETOWER";
                break;
            }
        } else {
            cout << "UPGRADE " << o.id << ' ';

            switch (o.type) {
            case RELOAD:
                cout << "RELOAD";
                break;
            case RANGE:
                cout << "RANGE";
                break;
            case DAMAGE:
                cout << "DAMAGE";
                break;
            }
        }

        cout << ';';
    }

    void find_write_best() {
        fill_upgradeables();
        generate_options();
        pair<Opt, Opt> best = best_option();

        if (best.first.option == PASS) {
            cout << "PASS;";
        } else {
            print_opt(best.first);
            print_opt(best.second);
        }

        cerr << "wave:" << wave_id << endl;
        cout << endl;
    }

    void read_starting() {
        fint id;
        struct timeval tv;

        cin >> my_id;

        gettimeofday(&tv, nullptr);
        start_time = tv.tv_sec * SEC + tv.tv_usec;
        end_time = start_time + FIRST_TIME_LIMIT;

        cin >> id >> id;

        for (fint i = 0; i < HEIGHT; ++i) {
            string line;
            cin >> line; 

            for (fint j = 1; j < WIDTH - 1; ++j) {
                board[i * WIDTH + j] = line[j - 1] == '#' ? EMPTY : CANYON;
            }

            board[i * WIDTH] = board[i * WIDTH + 1];
            board[i * WIDTH + WIDTH - 1] = board[i * WIDTH + WIDTH - 2];
        }
    }

    void swap_seen_ids() {
        max_id_prev = max_id_curr;
        max_id_curr = -1;
    }

    void read_turn() {
        fint opp = opponent(my_id), id, owner, x, y, damage, reload, cooldown, type_id;
        fint damage_level = 0, range_level = 0, reload_level = 0, list_offset, attackers, hp, max_hp, slow_time, bounty;
        fint real_x, real_y, dir, neigh, diff, spawned_this_round = 0, curr_max = -1;
        fint *dists;
        float fx, fy, curr_speed, max_speed, range;
        string type;
        Attacker *list, curr;
        struct timeval tv;

        cin >> cash[my_id];
        cerr << "after turn: " << turn << endl;

        if (turn != 0) {
            gettimeofday(&tv, nullptr);
            start_time = tv.tv_sec * SEC + tv.tv_usec;
            end_time = start_time + TURN_TIME_LIMIT;
        }

        cin >> lives[my_id];

        cin >> cash[opp] >> lives[opp];
        cin >> towers_count;

        for (fint i = 0; i < towers_count; ++i) {
            cin >> type >> id >> owner >> y >> x >> damage >> range >> reload >> cooldown;

            ++y;

            if (type[0] == 'F') {
                type_id = FIRE;
                cooldown++;
            } else if (type[0] == 'H') {
                type_id = HEAL;
            } else if (type[1] == 'U') {
                type_id = GUN;
            } else {
                type_id = GLUE;
            }

            list_offset = type_id * LEVELS_COUNT;

            for (fint j = 0; j < LEVELS_COUNT; ++j) {
                if (DAMAGES[list_offset] == damage) {
                    damage_level = j;
                }

                if (COOLDOWNS[list_offset] == reload) {
                    reload_level = j;
                }

                if (COD_RANGES[list_offset] == static_cast<fint>(range * 10.0f)) {
                    range_level = j;
                }

                list_offset++;
            }

            towers[i] = code_tower(type_id, i, x, y, owner, damage_level, range_level, reload_level, cooldown);
            board[x * WIDTH + y] = TOWER;
        }

        if (towers_count > 1) sort(towers, towers + towers_count);

        cin >> attackers;

        attackers_count[ONE] = 0;
        attackers_count[TWO] = 0;

        for (fint i = 0; i < attackers; ++i) {
            cin >> id >> owner >> fy >> fx >> hp >> max_hp >> curr_speed >> max_speed >> slow_time >> bounty;

            fy += 1.0f;

            x = static_cast<fint>(fx);
            y = static_cast<fint>(fy);
            real_x = static_cast<fint>(fx * 10.0);
            real_y = static_cast<fint>(fy * 10.0);

            if ((owner == ONE && real_y >= 180) || (owner == TWO && real_y < 10)) continue; // codingame referee shouldnt even print this

            if (owner == ONE) {
                list = p1_attackers;
                dists = p1_real_dists;
            } else {
                list = p2_attackers;
                dists = p2_real_dists;
            }

            if (id > max_id_prev) {
                ++spawned_this_round;
            }

            if (id > curr_max) {
                curr_max = id;
            }

            curr.bounty = bounty;
            curr.x = real_x;
            curr.y = real_y;
            curr.max_hp = max_hp;
            curr.health = hp;
            curr.slowed = slow_time;
            id = x * WIDTH + y;
            curr.pos = id;
            curr.speed = static_cast<fint>(max_speed * 10.0);

            real_x %= 10;

            if (real_x > 5) {
                dir = DOWN;
            } else if (real_x < 5) {
                dir = UP;
            } else {
                real_y %= 10;

                if (real_y > 5) {
                    dir = RIGHT;
                } else if (real_y < 5) {
                    dir = LEFT;
                } else {
                    dir = -1;
                }
            }

            curr.prev = -1;
            curr.next = -1;

            if (dir == -1) {
                curr.progress = 5;
            } else {
                switch (dir) {
                case UP:
                    if (id < WIDTH) {
                        neigh = -1;
                    } else {
                        neigh = id - WIDTH;
                        diff = 10 - real_x;
                    }
                    break;
                case DOWN:
                    if (id >= MAX - WIDTH) {
                        neigh = -1;
                    } else {
                        neigh = id + WIDTH;
                        diff = real_x;
                    }
                    break;
                case RIGHT:
                    if (Y_OF[id] == WIDTH - 1) {
                        neigh = -1;
                    } else {
                        neigh = id + 1;
                        diff = real_y;
                    }
                    break;
                case LEFT:
                    if (Y_OF[id] == 0) {
                        neigh = -1;
                    } else {
                        neigh = id - 1;
                        diff = 10 - real_y;
                    }
                    break;
                }

                if (neigh != -1) {
                    if (dists[neigh] < dists[id]) {
                        curr.progress = diff;
                        curr.next = neigh;
                    } else {
                        curr.progress = 10 - diff;
                        curr.prev = neigh;
                    }
                }
            }

            list[attackers_count[owner]++] = curr;
        }

        turn++;

        if (curr_max > max_id_curr) {
            max_id_curr = curr_max;
        }
        
        if (curr_max <= max_id_prev) {
            if (started && --spawning_left == 0) {
                swap_seen_ids();
                wave_plus();
            }
            return;
        }

        cerr << "spawning" << endl;

        started = true;

        if (--spawning_left == 0) {
            swap_seen_ids();
            wave_plus();
            return;
        }

        wave_count -= spawned_this_round / 2;

        update_futures();
    }

public:
    void init() {
        read_starting();
        fill_starting();
        bfs_real_dists();
        fill_next_possibs();
        fill_good_tiles();

        read_turn();
        find_write_best();
    }

    void one_turn() {
        read_turn();
        find_write_best();
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
fint Game::max_id_prev;
fint Game::max_id_curr;

int main() {
    Game game;
    game.init();

    while (true) {
        game.one_turn();
    }

    return 0;
}
