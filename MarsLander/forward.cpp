#pragma GCC optimize("Ofast,inline,tracer")
#pragma GCC optimize("unroll-loops,vpt,split-loops,unswitch-loops")
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

constexpr int MODE = 2; // 0 - FLY OFF / 1 - CRASH / 2 - LAND;
constexpr double EPSILON = 1e-3;
constexpr double MICRO_STEP = 1e-4;

using namespace std;

struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};

class MarsLander {
private:
    vector<Point> surface;
    vector<vector<pair<int, int>>> instructions = {{{0, 4}, {0, 4}, {0, 4}, {0, 4}, {0, 4}, {0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},}, // FLY OFF
        { // CRASH
        {-15, 1}, {-30, 2}, {-45, 3}, {-60, 4}, {-75, 4}, {-90, 4},
        {-75, 3}, {-60, 2}, {-45, 1}, {-30, 2}, {-15, 3}, {0, 2},
        {15, 3}, {30, 4}, {45, 4}, {30, 4}, {15, 4}, {0, 3}, {0, 2},
        {0, 3}, {0, 3}
        },
        {{-15, 1}, {-30, 2}, {-45, 3}, {-60, 4},{-60, 4},{-60, 4},{-60, 4},{-60, 4},{-60, 4},{-60, 4}, {-75, 4}, {-90, 4},{-90, 4},{-90, 4},
        {-75, 4}, {-60, 4}, {-45, 4}, {-30, 4}, {-15, 4}, {0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},
        {15, 4},{15, 4},{15, 4},{15, 4}, {30, 4}, {45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4},{45, 4}, {30, 4}, {15, 4}, {0, 4}, {0, 4},
        {0, 4}, {0, 4}, {0, 4}, {0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},
        {0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},{0, 4},
    },};
    size_t current_instr = 0;
    int flat_start_x, flat_end_x, flat_y;
    bool crashed = false;
    bool landed = false;

public:
    double X, Y, hSpeed, vSpeed;
    int fuel, rotate, power;

    void read_initial_input() {
        int surfaceN;
        cin >> surfaceN;
        for (int i = 0; i < surfaceN; ++i) {
            int x, y;
            cin >> x >> y;
            surface.emplace_back(x, y);
            if (i > 0 && surface[i-1].y == surface[i].y && 
                (surface[i].x - surface[i-1].x) >= 1000) {
                flat_start_x = surface[i-1].x;
                flat_end_x = surface[i].x;
                flat_y = surface[i].y;
            }
        }
        cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power;
    }

    void process_turn() {
        int desired_rotate = 0, desired_power = 0;
        if (!crashed && !landed && current_instr < instructions[MODE].size()) {
            desired_rotate = instructions[MODE][current_instr].first;
            desired_power = instructions[MODE][current_instr].second;
            current_instr++;
        }
        cout << desired_rotate << " " << desired_power << endl;

        if (crashed || landed) {
            cerr << "SIMULATION ENDED - ";
            if (landed) cerr << "SUCCESSFUL LANDING";
            else cerr << "CRASH DETECTED";
            cerr << endl;
            return;
        }

        const double initial_X = X;
        const double initial_Y = Y;
        const double initial_hSpeed = hSpeed;
        const double initial_vSpeed = vSpeed;

        rotate = max(-90, min(90, rotate + max(-15, min(15, desired_rotate - rotate))));
        power = max(0, min(4, power + max(-1, min(1, desired_power - power))));
        power = min(power, fuel);
        fuel -= power;

        const double angle = rotate * M_PI / 180.0;
        const double thrust_x = -power * sin(angle);
        const double thrust_y = power * cos(angle) - 3.711;

        bool collision_found = false;
        double collision_t = 1.0;
        Point collision_point(0, 0);
        
        // Simulate in micro-steps
        for (double t = 0.0; t <= 1.0; t += MICRO_STEP) {
            if (t > 1.0) t = 1.0;
            
            const double current_X = initial_X + initial_hSpeed * t + 0.5 * thrust_x * t * t;
            const double current_Y = initial_Y + initial_vSpeed * t + 0.5 * thrust_y * t * t;
            
            // Check boundaries
            if (current_X < 0 || current_X >= 7000 || current_Y < 0 || current_Y >= 3000) {
                collision_t = t;
                collision_point = Point(current_X, current_Y);
                collision_found = true;
                break;
            }

            // Check collision with terrain
            for (size_t i = 1; i < surface.size(); ++i) {
                const Point& p1 = surface[i-1];
                const Point& p2 = surface[i];
                
                if (current_X < min(p1.x, p2.x) - EPSILON || current_X > max(p1.x, p2.x) + EPSILON)
                    continue;

                const double distance = point_to_line_distance(
                    current_X, current_Y,
                    p1.x, p1.y,
                    p2.x, p2.y
                );

                if (distance <= EPSILON) {
                    collision_t = t;
                    collision_point = Point(current_X, current_Y);
                    collision_found = true;
                    break;
                }
            }
            
            if (collision_found) break;
        }

        // Update final position and velocity
        if (collision_found) {
            X = initial_X + initial_hSpeed * collision_t + 0.5 * thrust_x * collision_t * collision_t;
            Y = initial_Y + initial_vSpeed * collision_t + 0.5 * thrust_y * collision_t * collision_t;
            hSpeed = initial_hSpeed + thrust_x * collision_t;
            vSpeed = initial_vSpeed + thrust_y * collision_t;
            
            cerr << "Collision detected at: " << fixed << setprecision(5) 
                 << X << " " << Y << endl;
            handle_collision();
        } else {
            X = initial_X + initial_hSpeed + 0.5 * thrust_x;
            Y = initial_Y + initial_vSpeed + 0.5 * thrust_y;
            hSpeed += thrust_x;
            vSpeed += thrust_y;
        }

        check_boundaries();
        cerr << fixed << setprecision(2) 
             << X << " " << Y << " "
             << hSpeed << " " << vSpeed << " "
             << fuel << " " << rotate << " " << power << endl;
    }

private:
    double point_to_line_distance(double x, double y, 
                                 double x1, double y1, 
                                 double x2, double y2) const {
        const double dx = x2 - x1;
        const double dy = y2 - y1;
        const double t = ((x - x1) * dx + (y - y1) * dy) / (dx * dx + dy * dy);
        
        if (t < 0) {
            // Closest to first endpoint
            return sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
        } else if (t > 1) {
            // Closest to second endpoint
            return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
        }
        
        // Closest point on segment
        const double closest_x = x1 + t * dx;
        const double closest_y = y1 + t * dy;
        return sqrt((x - closest_x) * (x - closest_x) + (y - closest_y) * (y - closest_y));
    }

    void handle_collision() {
        // Find terrain elevation at collision point
        double terrain_y = 3000.0;
        bool is_flat = false;
        
        for (size_t i = 1; i < surface.size(); ++i) {
            const Point& p1 = surface[i-1];
            const Point& p2 = surface[i];
            
            if (X >= p1.x && X <= p2.x) {
                if (p1.x == p2.x) {
                    terrain_y = min(p1.y, p2.y);
                } else {
                    terrain_y = p1.y + (X - p1.x) * (p2.y - p1.y) / (p2.x - p1.x);
                }
                is_flat = (p1.y == flat_y) && (p1.x == flat_start_x) && (p2.x == flat_end_x);
                break;
            }
        }

        // Snap to terrain
        Y = terrain_y;
        
        // Check landing conditions
        const bool success = is_flat && 
                           (rotate == 0) && 
                           (abs(hSpeed) <= 20.0) && 
                           (abs(vSpeed) <= 40.0);

        if (success) {
            landed = true;
            cerr << "LANDED SAFELY" << endl;
        } else {
            crashed = true;
            cerr << "CRASHED ON " << (is_flat ? "LANDING" : "TERRAIN") << endl;
        }
    }

    void check_boundaries() {
        if (X < 0 || X >= 7000 || Y < 0 || Y >= 3000) {
            crashed = true;
            cerr << "OUT OF BOUNDS" << endl
                 << "Position: " << (int)round(X) << " " << (int)round(Y) << endl;
        }
    }
};

int main() {
    MarsLander lander;
    lander.read_initial_input();

    while (true) {
        lander.process_turn();
        int dummy;
        for (int i = 0; i < 7; ++i) cin >> dummy;
    }

    return 0;
}
