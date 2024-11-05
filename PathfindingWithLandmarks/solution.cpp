#include <iostream>

constexpr int MAX_SIZE = 240 * 240;

class Solver {
private:
    bool map[MAX_SIZE];
    int landmarks_num;
    float efficincy;
    int width, height;

public:
    void read_input() {
        std::string row;

        std::cin >> landmarks_num >> efficincy; std::cin.ignore();
        std::cin >> width >> height; std::cin.ignore();

        for (int i = 0; i < height; ++i) {
            std::cin >> row; std::cin.ignore();
        }
    }

};

int main() {

    return 0;
}
