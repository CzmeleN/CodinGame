#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <cstring>
#include <sys/time.h>
#include <string>
#include <cstdint>

constexpr int HEIGHT = 10;
constexpr int WIDTH = 19;
constexpr int MAX = 190;
constexpr int DIRS = 4;
constexpr int INTERVAL = 1024;
constexpr int LIMIT = 1007500;
constexpr int RESET = 120000;
constexpr float INIT_TEMP = 45.0;
constexpr float ALPHA = 0.9999895;

enum cell { VOID, PLATFORM, UP, RIGHT, DOWN, LEFT }; 

struct Uni {
    short x;
    short y;
    short dir;
};

class State{
private:
    struct timeval start_tv;
    std::pair<short, short> possibs[MAX];
    short og_board[MAX];
    short curr_board[MAX];
    short best_board[MAX];
    Uni bots[HEIGHT];
    int bot_count = 0, possibs_count = 0;
    uint32_t seed;

    void print_new_arrows() {
        for (short i = 0; i < HEIGHT; ++i) {
            for (short j = 0; j < WIDTH; ++j) {
                if (best_board[i * WIDTH + j] >= UP && og_board[i * WIDTH + j] < UP) {
                    std::cout << j << ' ' << i << ' ';

                    switch(best_board[i * WIDTH + j]) {
                        case UP:
                            std::cout << 'U';
                            break;
                        case RIGHT:
                            std::cout << 'R';
                            break;
                        case DOWN:
                            std::cout << 'D';
                            break;
                        case LEFT:
                            std::cout << 'L';
                            break;
                        default:
                            std::cout << "FAIL";
                            break;
                    }

                    std::cout << ' ';
                }
            }
        }

        std::cout << '\n';
    }

    inline uint32_t xorshift_rand() {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;

        return seed;
    }

    inline uint32_t randU32() {
        seed = (0x5DEECE66DUL * seed + 0xBUL) & ((1UL << 48) - 1);
        return seed >> 16;
    }

    inline float fast_exp(float x) {
        union { float f; int i; } u;
        u.i = (int)(12102203 * x + 1064866805);
        return u.f;
    }


    bool pointless(short x, short y) {
        bool voids[DIRS];
        bool not_platforms[DIRS];
        short curr = og_board[(x == 0 ? HEIGHT - 1 : x - 1) * WIDTH + y];
        
        if (curr >= UP) return false;
        voids[0] = curr == VOID;
        curr = og_board[(x <= 1 ? HEIGHT - 2 : x - 2) * WIDTH + y];
        not_platforms[0] = curr != PLATFORM;

        curr = og_board[x * WIDTH + (y + 1) % WIDTH];
        if (curr >= UP) return false;
        voids[1] = curr == VOID;
        curr = og_board[x * WIDTH + (y + 2) % WIDTH];
        not_platforms[1] = curr != PLATFORM;

        curr = og_board[((x + 1) % HEIGHT) * WIDTH + y];
        if (curr >= UP) return false;
        voids[2] = curr == VOID; 
        curr = og_board[((x + 2) % HEIGHT) * WIDTH + y];
        not_platforms[2] = curr != PLATFORM;

        curr = og_board[x * WIDTH + (y == 0 ? WIDTH - 1 : y - 1)];
        if (curr >= UP) return false;
        voids[3] = curr == VOID;
        curr = og_board[x * WIDTH + (y <= 1 ? WIDTH - 2 : y - 2)];
        not_platforms[3] = curr != PLATFORM;

        // tunnel
        if ((voids[0] && voids[2] && !voids[1] && !voids[3]) || (voids[1] && voids[3] && !voids[0] && !voids[2])) {
            return true;
        }

        if (!voids[0] && !voids[1] && !voids[2] && !voids[3] && og_board[((x + 1) % HEIGHT) * WIDTH + (y + 1) % WIDTH] == VOID) {
            return false; 
        }

        if (voids[0] || voids[1] || voids[2] || voids[3] || not_platforms[0] || not_platforms[1] || not_platforms[2] || not_platforms[3]) {
            return false;
        }

        return true;
    }

    void preprocess() {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (og_board[i * WIDTH + j] == PLATFORM && !pointless(i, j)) {
                    possibs[possibs_count++] = {i, j};
                }
            }
        }
    }

    Uni make_change() {
        constexpr short dx[] = {-1, 0, 1, 0}, dy[] = {0, 1, 0, -1};
        Uni res;
        short x, y, nx, ny, curr;
        unsigned int it = 0, new_it, pick;
        short dirs[5], op;

        pick = xorshift_rand() % possibs_count;
        x = possibs[pick].first;
        y = possibs[pick].second;
        curr = curr_board[x * WIDTH + y];

        if (curr != PLATFORM) {
            dirs[0] = PLATFORM;
            it = 1;
        }
        else {
            it = 0;
        }

        for (short i = 0; i < 4; ++i) {
            if (curr == i + 2) continue;

            nx = x == 0 && dx[i] == -1 ? HEIGHT - 1 : (x + dx[i]) % HEIGHT;
            ny = y == 0 && dy[i] == -1 ? WIDTH - 1 : (y + dy[i]) % WIDTH;

            if (curr_board[nx * WIDTH + ny] != VOID) {
                dirs[it++] = i + 2;
            }
        }

        new_it = xorshift_rand() % it;
        res = {x, y, curr_board[x * WIDTH + y]};
        curr_board[x * WIDTH + y] = dirs[new_it];

        return res;
    }

    int simulate() {
        int res = 0;
        short x, y, dir, bot_mult, id;
        uint64_t visited[MAX] = {};

        for (int i = 0; i < bot_count; ++i) {
            bot_mult = i * 4;
            x = bots[i].x;
            y = bots[i].y;
            id = y * WIDTH + x;
            dir = curr_board[id] >= UP ? curr_board[id] : bots[i].dir;

            visited[id] |= (1 << (dir - 2 + bot_mult));

            while(true) {
                res++;

                switch(dir) {
                    case UP:
                        y = y == 0 ? HEIGHT - 1 : y - 1;
                        break;
                    case RIGHT:
                        ++x %= WIDTH;
                        break;
                    case DOWN:
                        ++y %= HEIGHT;
                        break;
                    case LEFT:
                        x = x == 0 ? WIDTH - 1 : x - 1;
                        break;
                }

                id = y * WIDTH + x;

                if (curr_board[id] == VOID) {
                    break;
                }

                if (curr_board[id] >= UP) {
                    dir = curr_board[y * WIDTH + x];
                }

                if (visited[id] & (1 << (dir - 2 + bot_mult))) {
                    break;
                }

                visited[id] |= (1 << (dir - 2 + bot_mult));
            }
        }

        return res;
    }

    void update_best() {
        std::copy(curr_board, curr_board + MAX, best_board);
    }

    void best_sa() {
        Uni last;
        int curr_score = simulate(), since_best = 0;
        int best_score = curr_score, new_score, delta, count = 0, start_sec = start_tv.tv_sec, start_usec = start_tv.tv_usec, elapsed = 0;
        float temp = INIT_TEMP;
        struct timeval curr_tv; 

        do {
            last = make_change();
            new_score = simulate();
            delta = new_score - curr_score;

            if (delta > 0 || fast_exp(static_cast<float>(delta) / temp) > static_cast<float>(xorshift_rand()) / UINT32_MAX) {
                curr_score = new_score;

                if (curr_score > best_score) {
                    update_best();
                    best_score = curr_score;
                    since_best = 0;
                }
                else if (since_best == RESET) {
                    std::copy(best_board, best_board + MAX, curr_board);
                    curr_score = best_score;
                    temp = INIT_TEMP;
                    since_best = 0;
                }
                else {
                    since_best++;
                }
            }
            else {
                curr_board[last.x * WIDTH + last.y] = last.dir;
            }

            temp *= ALPHA;
            count++;

            if (count % INTERVAL == 0) {
                gettimeofday(&curr_tv, NULL);
                elapsed = (curr_tv.tv_sec - start_tv.tv_sec) * 1000000 + curr_tv.tv_usec - start_usec;
            }
        }
        while (elapsed < LIMIT);
    }
 
public:
    void print_board() {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                std::cerr << curr_board[i * WIDTH + j];
            }
        std::cerr << std::endl;
        }
    }

    void read_input() {
        std::string line;
        char curr;
        short count, cell;

        for (int i = 0; i < HEIGHT; ++i) {
            std::getline(std::cin, line);
            
            if (i == 0) {
                gettimeofday(&start_tv, NULL);
                seed = (start_tv.tv_sec ^ start_tv.tv_usec) & 0xFFFFFFFF;

                if (seed == 0) {
                    seed = 123456U;
                }
            }

            for (int j = 0; j < WIDTH; ++j) {
                curr = line[j];

                switch(curr) {
                    case '#':
                        cell = VOID;
                        break;
                    case '.':
                        cell = PLATFORM;
                        break;
                    case 'U':
                        cell = UP;
                        break;
                    case 'R':
                        cell = RIGHT;
                        break;
                    case 'D':
                        cell = DOWN;
                        break;
                    case 'L':
                        cell = LEFT;
                        break;
                }

                og_board[i * WIDTH + j] = cell;
                curr_board[i * WIDTH + j] = cell;
            }
        }
        
        std::cin >> count;
        
        while(count--) {
            Uni new_bot;

            std::cin >> new_bot.x >> new_bot.y;
            curr = getchar();
            curr = getchar();

            switch(curr) {
                case 'U':
                    new_bot.dir = UP;
                    break;
                case 'R':
                    new_bot.dir = RIGHT;
                    break;
                case 'D':
                    new_bot.dir = DOWN;
                    break;
                case 'L':
                    new_bot.dir = LEFT;
                    break;
            }

            bots[bot_count++] = new_bot;
        }
    }

    void solve() {
        preprocess();
        best_sa();
        print_new_arrows();
    }
};

int main() {
    State game;
    game.read_input();
    game.solve();

    return 0;
}
