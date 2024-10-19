#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

constexpr int HEIGHT = 10;
constexpr int WIDTH = 19;
constexpr int MAX = 190;
constexpr int DIRS = 4;
constexpr double LIMIT = 995.0;
enum cell { VOID, PLATFORM, UP, RIGHT, DOWN, LEFT }; 

struct Uni {
    short x;
    short y;
    short dir;
};

class State{
private:
    static std::mt19937 generator;
    std::vector<std::vector<short>> board;
    std::vector<Uni> bots;

    std::vector<Uni> get_new_arrows(State *orig) {
        std::vector<Uni> res;

        for (short i = 0; i < HEIGHT; ++i) {
            for (short j = 0; j < WIDTH; ++j) {
                if (board[i][j] >= UP && orig->board[i][j] < UP) {
                    res.push_back(Uni{j, i, board[i][j]});
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

        voids[0] = board[x == 0 ? HEIGHT - 1 : x - 1][y] == VOID;
        voids[1] = board[x][(y + 1) % WIDTH] == VOID;
        voids[2] = board[(x + 1) % HEIGHT][y] == VOID;
        voids[3] = board[x][y == 0 ? WIDTH - 1 : y - 1] == VOID;

        return (voids[0] && voids[2] && !voids[1] && !voids[3]) || (voids[1] && voids[3] && !voids[0] && !voids[2]);
    }

    void generate_random() {
        std::vector<std::pair<short, short>> possibs;
        // std::pair<short, short> possibs[MAX]
        constexpr short dx[] = {-1, 0, 1, 0}, dy[] = {0, 1, 0, -1};
        int count;
        short x, y, nx, ny;

        for (short i = 0; i < HEIGHT; ++i) {
            for (short j = 0; j < WIDTH; ++j) {
                if (board[i][j] == PLATFORM && !pointless(i, j)) {
                    possibs.push_back({i, j});
                }
            }
        }

        std::shuffle(possibs.begin(), possibs.end(), generator);
        count = random(1, possibs.size());
        
        for (int i = 0; i < count; ++i) {
            short dirs[4];
            int it = 0;

            x = possibs[i].first;
            y = possibs[i].second;
            
            for (short i = 0; i < 4; ++i) {

                nx = x == 0 && dx[i] == -1 ? HEIGHT - 1 : (x + dx[i]) % HEIGHT;
                ny = y == 0 && dy[i] == -1 ? WIDTH - 1 : (y + dy[i]) % WIDTH;

                if (board[nx][ny] != VOID) {
                    dirs[it++] = i + 2;
                }
            }

            board[x][y] = dirs[random(0, it)];
        }
    }

    void make_change(State *orig) {
        std::pair<short, short> possibs[MAX];
        constexpr short dx[] = {-1, 0, 1, 0}, dy[] = {0, 1, 0, -1};
        int x, y, nx, ny;
        unsigned int it = 0, pick;
        short dirs[5] = {PLATFORM};

        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (board[i][j] != VOID && orig->board[i][j] == PLATFORM && !pointless(i, j)) {
                    possibs[it++] = {i, j};
                }
            }
        }

        pick = xorshift_rand() % it;
        it = 1;
        x = possibs[pick].first;
        y = possibs[pick].second;
        
        for (short i = 0; i < 4; ++i) {

            nx = x == 0 && dx[i] == -1 ? HEIGHT - 1 : (x + dx[i]) % HEIGHT;
            ny = y == 0 && dy[i] == -1 ? WIDTH - 1 : (y + dy[i]) % WIDTH;

            if (board[nx][ny] != VOID) {
                //segfault here???
                dirs[it++] = i + 2;
            }
        }

        int new_it = xorshift_rand() % it;
        board[x][y] = dirs[new_it];
    }

    int simulate() {
        int res = 0;

        for (const auto& bot : bots) {
            bool visited[HEIGHT][WIDTH][DIRS] = {};
            short x = bot.x, y = bot.y, dir = board[y][x] >= UP ? board[y][x] : bot.dir;

            visited[y][x][dir - 2] = true;

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
                        x = x == 0 ? HEIGHT - 1 : x - 1;
                        break;
                }

                if (board[y][x] == VOID) {
                    break;
                }

                if (board[y][x] >= UP) {
                    dir = board[y][x];
                }

                if (visited[y][x][dir - 2]) {
                    break;
                }

                visited[y][x][dir - 2] = true;
            }
        }

        return res;
    }

    std::vector<Uni> best_random() {
        State best;
        int curr, max = 0, count = 0;
        double time_left = 1000.0;
        auto start = std::chrono::high_resolution_clock::now();

        while (time_left > 10) {
            // zmienic na co np64
            State new_state(this);
            new_state.generate_random();
            curr = new_state.simulate();

            if (curr > max) {
                max = curr;
                best = new_state;
            }

            auto time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = time - start;
            time_left -= elapsed.count();
            count++;
        }

        // std::cerr << "simulations: " << count << " max score: " << max << std::endl;
        return best.get_new_arrows(this);
    }

    std::vector<Uni> best_sa() {
        State curr = *this;
        State best = curr;
        int curr_score = curr.simulate();
        int best_score = curr_score, delta, neigh_score; //count = 0;
        double temp = 1000.0, elapsed;
        constexpr double final_temp = 0.01, alpha = 0.99996;
        auto start = std::chrono::high_resolution_clock::now();

        do {
            State neigh(curr);

            neigh.make_change(this);
            neigh_score = neigh.simulate();
            delta = neigh_score - curr_score;

            if (delta > 0 || std::exp(static_cast<double>(delta) / temp) > static_cast<double>(xorshift_rand()) / UINT32_MAX) {
                curr = neigh;
                curr_score = neigh_score;

                if (curr_score > best_score) {
                    best = curr;
                    best_score = curr_score;
                }
            }

            temp *= alpha;

            auto time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> el = time - start;
            elapsed = el.count();
            //count++;
        }
        while (temp > final_temp && elapsed < LIMIT);
        // std::cerr << elapsed << " < " << LIMIT << std::endl;
        // std::cerr << temp << " > " << final_temp << std::endl;
        // std::cerr << count << std::endl;

        return best.get_new_arrows(this);
    }
 
public:
    State(const State *old_state = nullptr) {
        if (old_state) {
            board = old_state->board;
            bots = old_state->bots; 
        }
        else {
            board = std::vector<std::vector<short>>(HEIGHT, std::vector<short>(WIDTH));
        }
    }

    void print_board() {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                std::cerr << board[i][j];
            }
        std::cerr << std::endl;
        }
    }

    void read_input() {
        std::string line;
        char curr;
        short count;

        for (int i = 0; i < HEIGHT; ++i) {
            std::getline(std::cin, line);
            // std::cerr << line << std::endl;
            for (int j = 0; j < WIDTH; ++j) {
                curr = line[j];

                switch(curr) {
                    case '#':
                        board[i][j] = VOID;
                        break;
                    case '.':
                        board[i][j] = PLATFORM;
                        break;
                    case 'U':
                        board[i][j] = UP;
                        break;
                    case 'R':
                        board[i][j] = RIGHT;
                        break;
                    case 'D':
                        board[i][j] = DOWN;
                        break;
                    case 'L':
                        board[i][j] = LEFT;
                        break;
                }
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

            bots.push_back(new_bot);
        }
    }

    void solve() {
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
                    std::cout << "fail";
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
