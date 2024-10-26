#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

constexpr int HEIGHT = 10;
constexpr int WIDTH = 19;
constexpr int MAX = 190;
constexpr int DIRS = 4;
constexpr int INTERVAL = 128;
constexpr double LIMIT = 999.0;
constexpr double INIT_TEMP = 100000.0;
constexpr double ALPHA = 0.99999;

enum cell { VOID, PLATFORM, UP, RIGHT, DOWN, LEFT }; 

struct Uni {
    short x;
    short y;
    short dir;
};

class State{
private:
    static std::mt19937 generator;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::pair<short, short> possibs[MAX];
    short og_board[HEIGHT][WIDTH];
    short curr_board[HEIGHT][WIDTH];
    short best_board[HEIGHT][WIDTH];
    Uni bots[HEIGHT];
    int bot_count = 0, possibs_count = 0;

    std::vector<Uni> get_new_arrows() {
        std::vector<Uni> res;

        for (short i = 0; i < HEIGHT; ++i) {
            for (short j = 0; j < WIDTH; ++j) {
                if (best_board[i][j] >= UP && og_board[i][j] < UP) {
                    res.push_back(Uni{j, i, best_board[i][j]});
                }
            }
        }

        return res;
    }

    static int random(int min, int max) {
        std::uniform_int_distribution<> dist(min, max);

        return dist(generator);
    }

    uint32_t xorshift_rand() {
        static uint32_t state = std::chrono::steady_clock::now().time_since_epoch().count() & 0xFFFFFFFF;

        if (state == 0) {
            state = 1;
        }

        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;

        return state;
    }

    bool pointless(short x, short y) {
        bool voids[DIRS];
        bool not_platforms[DIRS];
        short curr = og_board[x == 0 ? HEIGHT - 1 : x - 1][y];
        
        if (curr >= UP) return false;
        voids[0] = curr == VOID;
        curr = og_board[x <= 1 ? HEIGHT - 2 : x - 2][y];
        not_platforms[0] = curr != PLATFORM;

        curr = og_board[x][(y + 1) % WIDTH];
        if (curr >= UP) return false;
        voids[1] = curr == VOID;
        curr = og_board[x][(y + 2) % WIDTH];
        not_platforms[1] = curr != PLATFORM;

        curr = og_board[(x + 1) % HEIGHT][y];
        if (curr >= UP) return false;
        voids[2] = curr == VOID; 
        curr = og_board[(x + 2) % HEIGHT][y];
        not_platforms[2] = curr != PLATFORM;

        curr = og_board[x][y == 0 ? WIDTH - 1 : y - 1];
        if (curr >= UP) return false;
        voids[3] = curr == VOID;
        curr = og_board[x][y <= 1 ? WIDTH - 2 : y - 2];
        not_platforms[3] = curr != PLATFORM;

        // tunnel
        if ((voids[0] && voids[2] && !voids[1] && !voids[3]) || (voids[1] && voids[3] && !voids[0] && !voids[2])) {
            return true;
        }

        if (!voids[0] && !voids[1] && !voids[2] && !voids[3] && og_board[(x + 1) % HEIGHT][(y + 1) % WIDTH] == VOID) {
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
                if (og_board[i][j] == PLATFORM && !pointless(i, j)) {
                    possibs[possibs_count++] = {i, j};
                }
            }
        }
    }

    void generate_random() {
        std::vector<std::pair<short, short>> possibs;
        // std::pair<short, short> possibs[MAX]
        constexpr short dx[] = {-1, 0, 1, 0}, dy[] = {0, 1, 0, -1};
        int count;
        short x, y, nx, ny;

        for (short i = 0; i < HEIGHT; ++i) {
            for (short j = 0; j < WIDTH; ++j) {
                if (curr_board[i][j] == PLATFORM && !pointless(i, j)) {
                    possibs.push_back({i, j});
                }
            }
        }

        std::shuffle(possibs.begin(), possibs.end(), generator);
        count = random(1, possibs.size());
        
        for (int i = 0; i < count; ++i) {
            short dirs[4], op;
            int it = 0;

            x = possibs[i].first;
            y = possibs[i].second;
            
            for (short i = 0; i < 4; ++i) {

                nx = x == 0 && dx[i] == -1 ? HEIGHT - 1 : (x + dx[i]) % HEIGHT;
                ny = y == 0 && dy[i] == -1 ? WIDTH - 1 : (y + dy[i]) % WIDTH;

                if (curr_board[nx][ny] != VOID && curr_board[nx][ny] != (i + 2) % DIRS + 2) {
                    dirs[it++] = i + 2;
                }
            }

            curr_board[x][y] = dirs[random(0, it)];
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
        curr = curr_board[x][y];

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

            if (curr_board[nx][ny] != VOID) {
                dirs[it++] = i + 2;
            }
        }

        new_it = xorshift_rand() % it;
        res = {x, y, curr_board[x][y]};
        curr_board[x][y] = dirs[new_it];

        return res;
    }

    int simulate() {
        int res = 0;
        short x, y, dir, bot_mult;
        uint64_t visited[HEIGHT][WIDTH] = {};

        for (int i = 0; i < bot_count; ++i) {
            bot_mult = i * 4;
            x = bots[i].x;
            y = bots[i].y;
            dir = curr_board[y][x] >= UP ? curr_board[y][x] : bots[i].dir;

            visited[y][x] |= (1 << (dir - 2 + bot_mult));

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

                if (curr_board[y][x] == VOID) {
                    break;
                }

                if (curr_board[y][x] >= UP) {
                    dir = curr_board[y][x];
                }

                if (visited[y][x] & (1 << (dir - 2 + bot_mult))) {
                    break;
                }

                visited[y][x] |= (1 << (dir - 2 + bot_mult));
            }
        }

        return res;
    }

    std::vector<Uni> best_random() {
        State best;
        int curr, max = 0, count = 0;
        double time_left = 1000.0;

        while (time_left > 10) {
            // zmienic na co np64
            generate_random();
            curr = simulate();

            if (curr > max) {
                max = curr;
                update_best();
            }

            auto time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = time - start;
            time_left -= elapsed.count();
            count++;
        }

        // std::cerr << "simulations: " << count << " max score: " << max << std::endl;
        return best.get_new_arrows();
    }

    void update_best() {
        std::memcpy(best_board, curr_board, sizeof(curr_board));
        // for (int i = 0; i < HEIGHT; ++i) {
        //     for (int j = 0; j < WIDTH; ++j) {
        //         std::cerr << best_board[i][j] << ' ';
        //     }
        //     std::cerr << std::endl;
        // }

    }

    std::vector<Uni> best_sa() {
        Uni last;
        int curr_score = simulate();
        int best_score = curr_score, new_score, delta, count = 0;
        double temp = INIT_TEMP, elapsed = 0.0;
        
        auto start = std::chrono::high_resolution_clock::now();

        do {
            last = make_change();
            new_score = simulate();
            delta = new_score - curr_score;

            if (delta > 0 || std::exp(static_cast<double>(delta) / temp) > static_cast<double>(xorshift_rand()) / UINT32_MAX) {
                curr_score = new_score;

                if (curr_score > best_score) {
                    update_best();
                    best_score = curr_score;
                }
            }
            else {
                curr_board[last.x][last.y] = last.dir;
                //print_board();
            }

            temp *= ALPHA;
            count++;

            if (count % INTERVAL == 0) {
                auto time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> el = time - start;
                elapsed = el.count();
            }
        }
        while (elapsed < LIMIT);

        std::cerr << elapsed << " < " << LIMIT << std::endl;
        std::cerr << count << std::endl;
        std::cerr << best_score << std::endl;

        return get_new_arrows();
    }
 
public:
    void print_board() {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                std::cerr << curr_board[i][j];
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
            // std::cerr << line << std::endl;
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

                og_board[i][j] = cell;
                curr_board[i][j] = cell;
            }
        }
        
        std::cin >> count;
        // std::cerr << count << std::endl;
        
        while(count--) {
            Uni new_bot;
            
            std::cin >> new_bot.x >> new_bot.y; //>> curr >> curr;
            curr = getchar();
            curr = getchar();
            // std::cerr << new_bot.x << new_bot.y << ' ' << curr << std::endl;

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
        start = std::chrono::high_resolution_clock::now();
        preprocess();
        std::vector<Uni> res = best_sa();
        int size = res.size() - 1;

        for (const auto& arrow : res) {
            std::cout << arrow.x << ' ' << arrow.y << ' ';

            switch(arrow.dir) {
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

        std::cout << std::endl;
    }
};

std::mt19937 State::generator(std::random_device{}());

int main() {
    State game;
    game.read_input();
    game.solve();

    return 0;
}
