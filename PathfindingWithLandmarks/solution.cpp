#include <iostream>
#include <queue>
#include <algorithm>
#include <sys/time.h>
#include <cmath>
#include <utility>
#include <cstdint>

constexpr int MAX_SIZE = 240 * 240;
constexpr int ONE_SECOND = 1000000;
constexpr int PASS_CHANCE = 75;
constexpr float UNO = 1.0;
constexpr float SQRT2 = 1.41421353816986083984375;
constexpr float INF = std::numeric_limits<float>::infinity();
constexpr int x_dirs[4] = {1, -1, 0, 0};
constexpr int y_dirs[4] = {0, 0, 1, -1};
constexpr int diag_x_dirs[4] = {1, 1, -1, -1};
constexpr int diag_y_dirs[4] = {1, -1, 1, -1};
constexpr int x_all_dirs[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
constexpr int y_all_dirs[8] = {-1, 0, 1, -1, 1, -1, 0, -1};

class Solver {
private:
    int map[MAX_SIZE]; // -1 - wall 0, 1, 2, 3 - groups, -2 - passable (start)
    bool visited[MAX_SIZE];
    float dists[MAX_SIZE];
    std::vector<int> groups = {}, landmarks_per_group = {};
    std::vector<std::pair<int, int>> group_starts = {};
    std::vector<std::vector<std::pair<int,int>>> group_fields = {};
    int landmarks_count;
    float efficincy;
    int width, height, groups_count = 0, sum_empty = 0, full_time;
    uint32_t seed = 12345;
    timeval start;
    std::vector<std::pair<int, int>> landmarks = {};

    inline bool in_bounds(int x, int y) {
        if (x >= 0 && x < height && y >= 0 && y < width) return true;
        return false;
    }

    inline uint32_t xorshift_rand() {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;

        return seed;
    }

    void print_map() {
        int x_id;
        std::cerr << std::endl;
        for (int x = 0; x < height; ++x) {
            x_id = x * width;
            for (int y = 0; y < width; ++y) {
                std::cerr << map[x_id + y] << ' ';
            }
            std::cerr << std::endl;
        }
        std::cerr << std::endl;
    }
    
    std::pair<int, int> djikstra_farthest(std::vector<std::pair<int,int>> &curr_landmarks) {
        typedef std::tuple<float, int, int> queue_elem;
        int curr_id, nx, ny, x, y;
        float new_line_cost, new_diag_cost, curr_cost, max_dist = -1.0;
        std::priority_queue<queue_elem, std::vector<queue_elem>, std::greater<queue_elem>> pq;
        for (int i = 0; i < width * height; ++i) {
            dists[i] = INF;
        }

        for (const auto& landmark : curr_landmarks) {
            pq.emplace(0.0, landmark.first, landmark.second);
            dists[landmark.first * width + landmark.second] = 0.0;
        }

        while (!pq.empty()) {
            std::tie(curr_cost, x, y) = pq.top(); 
            pq.pop();
            curr_id = x * width + y;

            if (curr_cost > dists[curr_id]) {
                continue;
            }

            new_line_cost = curr_cost + UNO;
            new_diag_cost = curr_cost + SQRT2;

            for (int i = 0; i < 4; ++i) {
                nx = x + x_dirs[i];
                ny = y + y_dirs[i];

                if (in_bounds (nx, ny)) {
                    curr_id = nx * width + ny;
                    if (map[curr_id] != -1 && new_line_cost < dists[curr_id]) {
                        dists[curr_id] = new_line_cost;
                        pq.emplace(new_line_cost, nx, ny);
                    }
                }

                nx = x + diag_x_dirs[i];
                ny = x + diag_y_dirs[i];

                if (in_bounds(nx, ny)) {
                    curr_id = nx * width + ny;
                    if (map[curr_id] != -1 && new_diag_cost < dists[curr_id]) {
                        dists[curr_id] = new_diag_cost;
                        pq.emplace(new_diag_cost, nx, ny);
                    }
                }
            }
        }

        return {x, y};
    }

    int bfs_fill(int start_x, int start_y, int id) {
        std::queue<std::pair<int, int>> q;
        int nx, ny, curr_id, count = 1, x_sum = 0, y_sum = 0;
        bool set = false;

        q.push({start_x, start_y});
        map[start_x * width + start_y] = id;

        while (!q.empty()) {
            auto [x, y] = q.front();
            x_sum += x;
            y_sum += y;
            group_fields[id].push_back({x, y});
            q.pop();

            for (int i = 0; i < 4; ++i) {
                nx = x + x_dirs[i];
                ny = y + y_dirs[i];

                if (in_bounds (nx, ny)) {
                    curr_id = nx * width + ny;
                    if (map[curr_id] == -2) {
                        map[curr_id] = id;
                        count++;
                        q.push({nx, ny});
                    }
                }

                nx = x + diag_x_dirs[i];
                ny = x + diag_y_dirs[i];

                if (in_bounds (nx, ny)) {
                    curr_id = nx * width + ny;
                    if (map[curr_id] == -2) {
                        map[curr_id] = id;
                        count++;
                        q.push({nx, ny});
                    }
                }
            }

            // if (!set && (xorshift_rand() % 20 == 0 || q.empty())) {
            //     group_starts.push_back({x, y});
            //     set = true;
            // }
        }

        // group_starts.push_back({x_sum / count, y_sum / count});

        return count;
    }

    void djikstra_find_first(int group_id) {
        typedef std::tuple<float, int, int> queue_elem;
        int curr_id, nx, ny, x, y;
        float new_line_cost, new_diag_cost, curr_cost, max_dist = -1.0;
        std::priority_queue<queue_elem, std::vector<queue_elem>, std::greater<queue_elem>> pq;

        std::fill(visited, visited + width * height, false);
        pq.emplace(0.0, group_starts[group_id].first, group_starts[group_id].second);

        while (!pq.empty()) {
            std::tie(curr_cost, x, y) = pq.top(); 
            pq.pop();
            curr_id = x * width + y;

            if (map[curr_id] == group_id) {
                break;
            }

            if (visited[curr_id]) {
                continue;
            }

            visited[curr_id] = true;

            new_line_cost = curr_cost + UNO;
            new_diag_cost = curr_cost + SQRT2;

            for (int i = 0; i < 4; ++i) {
                nx = x + x_dirs[i];
                ny = y + y_dirs[i];

                if (in_bounds (nx, ny)) {
                    curr_id = nx * width + ny;
                    pq.emplace(new_line_cost, nx, ny);
                }

                nx = x + diag_x_dirs[i];
                ny = x + diag_y_dirs[i];

                if (in_bounds(nx, ny)) {
                    curr_id = nx * width + ny;
                    pq.emplace(new_diag_cost, nx, ny);
                }
            }
        }

        group_starts[group_id] = {x, y}; 
    }

    void find_groups() {
        std::vector<std::pair<float, int>> rests_ids = {};
        int x_id, this_group, assigned_landmarks = 0, curr_landmarks, count;
        float res;

        for (int x = 0; x < height; ++x) {
            x_id = x * width;
            for (int y = 0; y < width; ++y) {
                if (map[x_id + y] == -2) {
                    group_fields.push_back(std::vector<std::pair<int, int>>());
                    this_group = bfs_fill(x, y, groups_count++);
                    sum_empty += this_group;
                    groups.push_back(this_group);
                    group_starts.push_back({x, y});
                }
            }
        }

        for (int i = 0; i < groups_count; ++i) {
            count = groups[i];
            res = count * landmarks_count / static_cast<float>(sum_empty);
            curr_landmarks = res;
            assigned_landmarks += curr_landmarks;
            landmarks_per_group.push_back(curr_landmarks);
            rests_ids.push_back({res - curr_landmarks, i});
        }

        std::sort(rests_ids.begin(), rests_ids.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
            return a.first > b.first;
        });

        for (int i = 0; assigned_landmarks < landmarks_count && i < rests_ids.size(); ++i) {
            if (landmarks_per_group[rests_ids[i].second] == 0) {
                landmarks_per_group[rests_ids[i].second] += 1;
                ++assigned_landmarks;
            }
        }

        for (int i = 0; assigned_landmarks < landmarks_count; ++i, ++assigned_landmarks) {
            std::cerr<< rests_ids[i].first << std::endl;
            landmarks_per_group[rests_ids[i].second] += 1;
        }

        std::cerr << "we got " << assigned_landmarks << '\n';

        for (int i = 0; i < groups_count; ++i) {
            count = landmarks_per_group[i];
            std::cerr << "this group val: " << groups[i]/ static_cast<float>(sum_empty);
            std::cerr<< ", this group count: " << count << std::endl;
        }
    }

    std::pair<int, int> bfs_farthest(std::vector<std::pair<int,int>> &curr_landmarks) {
        std::queue<std::pair<int, int>> q;
        int nx, ny, curr_id, count = 1, x, y;
        std::pair<int, int> res;

        std::fill(visited, visited + width * height, false);

        for (const auto& landmark : curr_landmarks) {
            q.push(landmark);
        }

        while (!q.empty()) {
            res = q.front();
            x = res.first;
            y = res.second;
            q.pop();

            curr_id = x * width + y;
            if (visited[curr_id]) continue;
            visited[curr_id] = true;

            for (int i = 0; i < 8; ++i) {
                nx = x + x_all_dirs[i];
                ny = y + y_all_dirs[i];

                if (in_bounds (nx, ny)) {
                    curr_id = nx * width + ny;
                    if (map[curr_id] != -1 && !visited[curr_id]) {
                        q.push({nx, ny});
                    }
                }

                // nx = x + diag_x_dirs[i];
                // ny = x + diag_y_dirs[i];

                // if (in_bounds (nx, ny)) {
                //     curr_id = nx * width + ny;
                //     if (map[curr_id] != -1 && !visited[curr_id]) {
                //         q.push({nx, ny});
                //     }
                // }
            }
        }

        return res;
    }

    std::pair<int, int> find_farthest(std::vector<std::pair<int,int>> &curr_landmarks, int group_id) {
        if (curr_landmarks.empty()) {
            return {group_fields[group_id][xorshift_rand() % group_fields[group_id].size()]};
        }
        return bfs_farthest(curr_landmarks);
    }

    void compute_best() {
        find_groups();
        std::vector<std::pair<int, int>> curr_landmarks;
        std::pair<int, int> curr_best;
        timeval curr_timer_start, curr_timer_end;
        int curr_landmarks_count, x, y, elapsed, landmarks_per_this, limit;

        gettimeofday(&curr_timer_end, NULL);

        full_time = 19.99 * ONE_SECOND - ((curr_timer_end.tv_sec - start.tv_sec) * ONE_SECOND + curr_timer_end.tv_usec - start.tv_usec);
        std::cerr << "full time left: " << full_time / static_cast<float>(ONE_SECOND) << std::endl;

        for (int i = 0; i < groups_count; ++i) {
            gettimeofday(&curr_timer_start, NULL);

            landmarks_per_this = landmarks_per_group[i];
            if (landmarks_per_this == 0) continue;

            limit = landmarks_per_this / static_cast<float>(landmarks_count) * full_time;
            elapsed = 0;

            curr_landmarks.clear();
            curr_landmarks_count = 1;
            // djikstra_find_first(i);
            curr_landmarks.push_back({group_starts[i].first, group_starts[i].second});

            while (elapsed < limit && curr_landmarks_count < landmarks_per_this) {
                curr_best = find_farthest(curr_landmarks, i);
                curr_landmarks.push_back(curr_best);
                curr_landmarks_count++;

                if (xorshift_rand() % 100 > PASS_CHANCE) {
                    if (landmarks_per_this > 1) {
                        curr_landmarks.erase(curr_landmarks.begin() + (xorshift_rand() % curr_landmarks_count));
                        curr_landmarks_count--;
                    }
                }

                gettimeofday(&curr_timer_end, NULL);
                elapsed = (curr_timer_end.tv_sec - curr_timer_start.tv_sec) * ONE_SECOND + curr_timer_end.tv_usec - curr_timer_start.tv_usec;
            }

            for (const auto& landmark : curr_landmarks) {
                landmarks.push_back(landmark);
            }

            std::cerr << "elapsed this group: " << elapsed / static_cast<float>(ONE_SECOND) << std::endl;
        }

        gettimeofday(&curr_timer_end, NULL);
        std::cerr << "elapsed total: " << ((curr_timer_end.tv_sec - start.tv_sec) * ONE_SECOND + curr_timer_end.tv_usec - start.tv_usec) / static_cast<float>(ONE_SECOND) << std::endl;
    }

public:
    void read_input() {
        int padding;
        std::string row;

        std::cin >> landmarks_count >> efficincy; std::cin.ignore();
        std::cerr << "we need " << landmarks_count << " landmarks" << std::endl;
        gettimeofday(&start, NULL);
        std::cin >> width >> height; std::cin.ignore();
        // std::cout << width << '\n' << height << '\n';

        for (int i = 0; i < height; ++i) {
            padding = i * width;
            std::cin >> row; std::cin.ignore();

            for (int j = 0; j < width; j++) {
                map[padding + j] = row[j] == '.' ? -2 : -1;
            }
        }
    }

    void solve() {
        compute_best();

        for (auto const& landmark : landmarks) {
            std::cout << landmark.second << ' ' << landmark.first << '\n';
        }
    }
};

int main() {
    Solver solver;
    solver.read_input();
    solver.solve();
    return 0;
}
