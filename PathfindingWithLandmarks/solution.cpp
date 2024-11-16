#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <queue>
#include <algorithm>
#include <sys/time.h>
#include <cmath>
#include <utility>
#include <cstdint>
#include <limits>
#include <vector>

constexpr int MAX_SIZE = 240 * 240;
constexpr int ONE_SECOND = 1000000;
constexpr int TESTS_COUNT = 200;
constexpr float UNO = 1.0;
constexpr float SQRT2 = 1.41421353816986083984375;
constexpr float INF = std::numeric_limits<float>::infinity();
constexpr float FULL_TIME = 19.9 * ONE_SECOND;
constexpr int x_all_dirs[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
constexpr int y_all_dirs[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

class Solver {
private:
    int map[MAX_SIZE];
    int color_map[MAX_SIZE];
    int width, height;
    int landmarks_count;
    float efficiency;
    int areas_count = 0;
    std::vector<int> area_sizes;
    std::vector<std::vector<std::pair<int, int>>> area_points;
    std::vector<int> landmarks_per_area;
    std::vector<std::pair<int, int>> landmarks;
    std::vector<std::pair<int, int>> best_landmarks;
    uint32_t seed = 12345;
    timeval start_time;

    struct Test {
        int x1, y1;
        int x2, y2;
        std::vector<int> start_distances;
        std::vector<int> end_distances;
    };

    std::vector<Test> tests;

    inline bool in_bounds(int x, int y) {
        return (x >= 0 && x < height && y >= 0 && y < width);
    }

    inline uint32_t xorshift_rand() {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        return seed;
    }

    void color_group(int x, int y, int color_id) {
        int curr_id = x * width + y;
        if (map[curr_id] != -2 || color_map[curr_id] != -1)
            return;
        color_map[curr_id] = color_id;
        area_points[color_id].push_back({x, y});
        area_sizes[color_id]++;
        for (int i = 0; i < 8; ++i) {
            int nx = x + x_all_dirs[i];
            int ny = y + y_all_dirs[i];
            if (in_bounds(nx, ny)) {
                color_group(nx, ny, color_id);
            }
        }
    }

    void color_map_areas() {
        std::fill(color_map, color_map + width * height, -1);
        for (int x = 0; x < height; ++x) {
            for (int y = 0; y < width; ++y) {
                int idx = x * width + y;
                if (map[idx] == -2 && color_map[idx] == -1) {
                    area_sizes.push_back(0);
                    area_points.push_back(std::vector<std::pair<int, int>>());
                    color_group(x, y, areas_count++);
                }
            }
        }
    }

    void assign_landmarks_to_areas() {
        int total_area = 0;
        for (const auto& size : area_sizes)
            total_area += size;

        landmarks_per_area.resize(areas_count, 0);
        int assigned_landmarks = 0;
        for (int i = 0; i < areas_count; ++i) {
            float proportion = static_cast<float>(area_sizes[i]) / total_area;
            int count = proportion * landmarks_count;
            landmarks_per_area[i] = count;
            assigned_landmarks += count;
        }

        while (assigned_landmarks < landmarks_count) {
            for (int i = 0; i < areas_count && assigned_landmarks < landmarks_count; ++i) {
                landmarks_per_area[i]++;
                assigned_landmarks++;
            }
        }
    }

    std::pair<int, int> find_farthest(int area_id, const std::vector<std::pair<int, int>>& existing_landmarks) {
        std::queue<std::pair<int, int>> q;
        std::vector<bool> visited(width * height, false);
        for (const auto& landmark : existing_landmarks) {
            q.push(landmark);
            visited[landmark.first * width + landmark.second] = true;
        }
        std::pair<int, int> farthest_point = existing_landmarks.empty() ? area_points[area_id][0] : existing_landmarks.back();
        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            farthest_point = {x, y};
            for (int i = 0; i < 8; ++i) {
                int nx = x + x_all_dirs[i];
                int ny = y + y_all_dirs[i];
                if (in_bounds(nx, ny)) {
                    int idx = nx * width + ny;
                    if (!visited[idx] && map[idx] != -1 && color_map[idx] == area_id) {
                        q.push({nx, ny});
                        visited[idx] = true;
                    }
                }
            }
        }
        return farthest_point;
    }

    void place_landmarks() {
        landmarks.clear();
        for (int area_id = 0; area_id < areas_count; ++area_id) {
            int count = landmarks_per_area[area_id];
            if (count == 0) continue;
            int start_index = landmarks.size();
            std::vector<std::pair<int, int>> existing_landmarks;
            int landmarks_placed = 0;
            while (landmarks_placed < count) {
                auto farthest_point = find_farthest(area_id, existing_landmarks);
                existing_landmarks.push_back(farthest_point);
                landmarks.push_back(farthest_point);
                landmarks_placed++;

                if (xorshift_rand() % 100 >= 75) {
                    if (!existing_landmarks.empty()) {
                        int idx_to_remove = xorshift_rand() % existing_landmarks.size();
                        existing_landmarks.erase(existing_landmarks.begin() + idx_to_remove);
                        landmarks.erase(landmarks.begin() + start_index + idx_to_remove);
                        landmarks_placed--;
                    }
                }
            }
        }
    }

    void generate_tests() {
        for (int i = 0; i < TESTS_COUNT; ++i) {
            int area_id = xorshift_rand() % areas_count;
            int idx1 = xorshift_rand() % area_points[area_id].size();
            int idx2 = xorshift_rand() % area_points[area_id].size();
            Test test;
            test.x1 = area_points[area_id][idx1].first;
            test.y1 = area_points[area_id][idx1].second;
            test.x2 = area_points[area_id][idx2].first;
            test.y2 = area_points[area_id][idx2].second;
            test.start_distances.resize(width * height, -1);
            test.end_distances.resize(width * height, -1);
            compute_test_distances(test);
            tests.push_back(test);
        }
    }

    void compute_test_distances(Test& test) {
        std::queue<std::pair<int, int>> q;
        q.push({test.x1, test.y1});
        test.start_distances[test.x1 * width + test.y1] = 0;
        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            int curr_dist = test.start_distances[x * width + y];
            for (int i = 0; i < 8; ++i) {
                int nx = x + x_all_dirs[i];
                int ny = y + y_all_dirs[i];
                if (in_bounds(nx, ny)) {
                    int idx = nx * width + ny;
                    if (test.start_distances[idx] == -1 && map[idx] != -1 && color_map[idx] == color_map[test.x1 * width + test.y1]) {
                        test.start_distances[idx] = curr_dist + 1;
                        q.push({nx, ny});
                    }
                }
            }
        }
        q.push({test.x2, test.y2});
        test.end_distances[test.x2 * width + test.y2] = 0;
        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            int curr_dist = test.end_distances[x * width + y];
            for (int i = 0; i < 8; ++i) {
                int nx = x + x_all_dirs[i];
                int ny = y + y_all_dirs[i];
                if (in_bounds(nx, ny)) {
                    int idx = nx * width + ny;
                    if (test.end_distances[idx] == -1 && map[idx] != -1 && color_map[idx] == color_map[test.x2 * width + test.y2]) {
                        test.end_distances[idx] = curr_dist + 1;
                        q.push({nx, ny});
                    }
                }
            }
        }
    }

    float get_score() {
        float total_h = 0.0f;
        float total_weight = 0.0f;

        for (const auto& test : tests) {
            float h = 0.0f;
            int shortest_path = test.start_distances[test.x2 * width + test.y2];

            if (shortest_path <= 0) continue;

            for (const auto& landmark : landmarks) {
                int idx = landmark.first * width + landmark.second;
                float dsrc = test.start_distances[idx];
                float ddst = test.end_distances[idx];
                float diff = fabs(dsrc - ddst);
                if (diff > h) {
                    h = diff;
                }
            }
            float weight = shortest_path;
            total_h += (h / shortest_path) * weight;
            total_weight += weight;
        }
        return total_weight > 0 ? total_h / total_weight : 0.0f;
    }


public:
    void read_input() {
        std::cin >> landmarks_count >> efficiency;
        gettimeofday(&start_time, NULL);
        std::cin >> width >> height;
        std::string row;
        for (int i = 0; i < height; ++i) {
            std::cin >> row;
            for (int j = 0; j < width; ++j) {
                int idx = i * width + j;
                map[idx] = (row[j] == '.') ? -2 : -1;
            }
        }
    }

    void solve() {
        color_map_areas();
        assign_landmarks_to_areas();
        generate_tests();
        float best_score = 0.0;
        timeval curr_time;
        int elapsed_time = 0;
        do {
            place_landmarks();
            float score = get_score();
            if (score > best_score) {
                best_score = score;
                best_landmarks = landmarks;
            }
            gettimeofday(&curr_time, NULL);
            elapsed_time = (curr_time.tv_sec - start_time.tv_sec) * ONE_SECOND + curr_time.tv_usec - start_time.tv_usec;
        } while (elapsed_time < FULL_TIME);
        for (const auto& landmark : best_landmarks) {
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

