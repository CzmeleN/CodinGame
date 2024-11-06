#include <iostream>
#include <sys/time.h>

constexpr int MAX_SIZE = 240 * 240;
constexpr int MAX_LANDMARKS = 20;
constexpr float UNO = 1.0;
constexpr float SQRT2 = 1.41421353816986083984375;

class Solver {
private:
    bool map[MAX_SIZE];
    int landmarks_count;
    float efficincy;
    int width, height;
    timeval start;
    std::pair<short, short> landmarks[MAX_LANDMARKS];

    void compute_best() {

    }

public:
    void read_input() {
        int padding;
        std::string row;

        std::cin >> landmarks_count >> efficincy; std::cin.ignore();
        gettimeofday(&start, NULL);
        std::cin >> width >> height; std::cin.ignore();

        for (int i = 0; i < height; ++i) {
            padding = i * width;
            std::cin >> row; std::cin.ignore();
            
            for (int j = 0; j < width; j++) {
                map[padding + j] = row[j] == '.' ? true : false;
            }
        }
    }

    void solve() {
        compute_best();
        
        for (int i = 0; i < landmarks_count; ++i) {
            std::cout << landmarks[i].first << ' ' << landmarks[i].second << '\n';
        }
    }
};

int main() {

    return 0;
}
