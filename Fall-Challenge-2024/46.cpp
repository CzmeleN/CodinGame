#include <iostream>
#include <limits.h>
#include <deque>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

static const int TYPES_COUNT = 20;
static const int DAYS_COUNT = 20;
static const int BALANCING_MAX = 50;
static const int SPEED_MAX = 50;
static const int MAX_POD_CAPACITY = 10;
static const int MAX_TUBES = 5;
static const int PORTAL_COST = 5000; 
static const int POD_COST = 1000;
static const int KM_COST = 10;
static const float EPSILON = 0.0000001;

struct LandingPad {
    int id;
    vector<int> astronauts = vector<int>(TYPES_COUNT, 0);
};

struct LunarModule {
    int type;
    int id;
    int bonus_left = BALANCING_MAX;
};

struct Building {
    int x, y;
    vector<int> pods_ids;
    vector<int> tubes_ids;
    vector<pair<int, int>> lowest_distances = vector<pair<int, int>>(TYPES_COUNT, {-1, -1}); // lowest distances to pads of given type - dist, building_id;
    int portal_id = -1;
    int type; // -1 - landing pad, lunar module otherwise
    int id_on_list;
};

struct Line {
    int src;
    int dst;
};

struct Pod {
    int src;
    int dst;
    int pos;
};

struct Astronaut {
    int pos;
    int start;
    int type;
};

struct GameState {
    int resources;
    vector<LandingPad> pads;
    vector<LunarModule> modules;
    vector<Building> buildings;
    vector<Line> portals;
    vector<Line> tubes;
    vector<Pod> pods;
    vector<Astronaut> astronauts;

    void add_portal(Line new_portal) {
        int src = new_portal.src, dst = new_portal.dst;
        buildings[src].portal_id = portals.size();
        buildings[dst].portal_id = portals.size();
        portals.push_back(new_portal);
    }

    void add_tube(Line new_tube) {
        int src = new_tube.src, dst = new_tube.dst;
        buildings[src].tubes_ids.push_back(tubes.size());
        buildings[dst].tubes_ids.push_back(tubes.size());
        tubes.push_back(new_tube);
    }

    // GameState copy_self() {
    //     GameState copy = *this;
    //     return copy;
    // }

    float dist(pair<int, int> p1, pair<int,int> p2) {
        return sqrt((p2.first - p1.first) * (p2.first - p1.first) + (p2.second - p1.second) * (p2.second - p1.second));
    }

    bool point_segment(pair<int, int> a, pair<int, int> b, pair<int, int> c) {
        float sum = dist(b, a) + dist(a, c) - dist(b, c);
        return (-EPSILON < sum && sum < EPSILON);
    }

    int orientation(pair<int, int> p1, pair<int, int> p2, pair<int, int> p3) {
        int prod = (p3.second - p1.second) * (p2.first - p1.first) - (p2.second - p1. second) * (p3.first - p1. first);
        return (prod > 0) - (prod < 0);
    }

    bool segment_segment(pair<int, int> a, pair<int, int> b, pair<int, int> c, pair<int, int> d) {
        return orientation(a, b, c) * orientation(a, b, d) < 0 && orientation(c, d, a) * orientation(c, d, b) < 0; 
    }

    bool collides(Line tube) {
        int x1 = buildings[tube.src].x, y1 = buildings[tube.src].y, 
        x2 = buildings[tube.dst].x, y2 = buildings[tube.dst].y,
        x3, y3, x4, y4;
        int bx1 = x1, bx2 = x2, by1 = y1, by2 = y2;

        if (x1 > x2) {
            swap(bx1, bx2);
        }
        if (y1 > y2) {
            swap(by1, by2);
        }

        for (int i = 0; i < buildings.size(); ++i) {
            if (i == tube.src || i == tube.dst) continue;

            x3 = buildings[i].x;
            y3 = buildings[i].y;

            if (x3 >= bx1 && x3 <= bx2 && y3 >= by1 && y3 <= by2) {
                if (point_segment({x3, y3}, {x1, y1}, {x2, y2})) {
                    //cerr << x3 << ' ' << y3 << endl;
                    return true;
                }
            }
        }

        for (const auto& tb: tubes) {
            x3 = buildings[tb.src].x;
            y3 = buildings[tb.src].y;
            x4 = buildings[tb.dst].x;
            y4 = buildings[tb.dst].y;

            if (segment_segment({x1, y1}, {x2, y2}, {x3, y3}, {x4, y4})) {
                return true;
            }
        }

        return false;
    }

    int astronauts_heur(int start, int type) {
        vector<bool> visited(buildings.size(), false);
        deque<int> dq;
        int res = 0, curr, id, next;

        dq.push_front(start);
        visited[start] = true;

        while (!dq.empty()) {
            curr = dq.front();
            dq.pop_front();
            visited[curr] = true;

            //cerr << "visiting " << curr << "of type " << buildings[curr].type << endl;
            if (buildings[curr].type == 0) {
                res += pads[buildings[curr].id_on_list].astronauts[type];
            }
            
            id = buildings[curr].portal_id;
            if (id != -1 && portals[id].dst == curr) {
                next = portals[id].src;
                if (!visited[next]) {
                    dq.push_back(next);
                }
            }

            for (const auto id : buildings[curr].tubes_ids) {
                next = tubes[id].src == curr ? tubes[id].dst : tubes[id].src;
                if (!visited[next]) {
                    dq.push_back(next);
                }
            }
        }

        return res;
    }

    int pad_heur(int id, int type) {
        int res = 0;

        if (buildings[id].type == 0) {
            res += pads[buildings[id].id_on_list].astronauts[type];
        }

        return res;
    }

    bool tube_exists(int building_id, Line tube) {
        int src, dst;

        for (const int tube_id : buildings[building_id].tubes_ids) {
            src = tubes[tube_id].src;
            dst = tubes[tube_id].dst;

            if ((tube.src == src && tube.dst == dst) || (tube.src == dst && tube.dst == src)) {
                return true;
            }
        }

        return false;
    }

    int tube_cost(int src, int dst) {
        return dist({buildings[src].x, buildings[src].y}, {buildings[dst].x, buildings[dst].y}) * KM_COST + POD_COST;
    }

    Line tube_heur() {
        Line res = {-1, -1};

        if (resources < POD_COST + 10) {
            return res;
        }

        int pads_count = pads.size();
        int modules_count = modules.size();
        int max = 0, src, dst, curr, type, cost;

        for (int i = 0; i < pads_count; ++i) {
            src = pads[i].id;

            if (buildings[src].tubes_ids.size() != MAX_TUBES) {
                for (int j = 0; j < modules_count; ++j) {
                    dst = modules[j].id;
                    if (buildings[dst].tubes_ids.size() != MAX_TUBES) {
                        cost = tube_cost(src, dst);
                        if (cost > resources) continue;

                        if (!tube_exists(src, {src, dst}) && !collides({src, dst})) {
                            type = modules[j].type;
                            curr = pad_heur(src, type);

                            if (curr > max) {
                                max = curr;
                                res = {src, dst};
                            }
                        }
                    }
                }
            }
        }

        return res;
    }

    Line portal_heur() {
        Line res = {-1, -1};

        if (resources < PORTAL_COST) {
            return res;
        }

        int pads_count = pads.size();
        int modules_count = modules.size();
        int max = 0, src, dst, curr, type;

        for (int i = 0; i < pads_count; ++i) {
            src = pads[i].id;

            if (buildings[src].portal_id == -1) {
                for (int j = 0; j < modules_count; ++j) {
                    dst = modules[j].id;
                    //cerr << src << " " << pads[i].astronauts[0] << ", " <<pads[i].astronauts[1] << " -> " << modules[j].type << endl;
                    if (buildings[dst].portal_id == -1) {
                        type = modules[j].type;
                        curr = astronauts_heur(src, type);

                        if (curr > max) {
                            max = curr;
                            res = {src, dst};
                        }
                    }
                }
            }
        }

        return res;
    }

    void solve() {
        vector<Line> portals, tubes;
        int last;
        bool place_tubes = true, place_portals = true;

        while (place_tubes || place_portals) {
            if (place_tubes) {
                Line new_tube = tube_heur();
                //cerr << new_tube.src << "->" << new_tube.dst << endl;

                if (new_tube.src == -1) {
                    place_tubes = false;
                }
                else {
                    add_tube(new_tube);
                    tubes.push_back(new_tube);
                    resources -= tube_cost(new_tube.src, new_tube.dst);
                }
            }
            else if (place_portals) {
                Line new_portal = portal_heur();

                if (new_portal.src == -1) {
                    place_portals = false;
                }
                else {
                    add_portal(new_portal);
                    portals.push_back(new_portal);
                    resources -= PORTAL_COST;
                }
            }
        }

        if (tubes.size() == 0 && portals.size() == 0) {
            cout << "WAIT"; 
        }

        last = tubes.size() - 1;

        for (int i = 0; i <= last; ++i) {
            cout << "TUBE " << tubes[i].src << ' ' << tubes[i].dst << ';' << "POD " << this->tubes.size() - i << ' ' << tubes[i].src << ' ' << tubes[i].dst << ' ' << tubes[i].src;
            if (i != last || portals.size() != 0) {
                cout << ';';
            }
        }

        last = portals.size() - 1;

        for (int i = 0; i <= last; ++i) {
            cout << "TELEPORT " << portals[i].src << ' ' << portals[i].dst;
            if (i != last) {
                cout << ';';
            }
        }

        cout << endl;
    }
};

int main() {
    GameState state;
    int source, destination, capacity, count, new_buildings_count, id, type, ast_type, x, y, curr_id;
    string pod_properties;

    ios_base::sync_with_stdio(0);
    cin.tie(0);
    cout.tie(0);

    // game loop
    while (1) {
        cin >> state.resources; cin.ignore();
        
        cin >> count; cin.ignore();
        for (int i = 0; i < count; i++) {
            cin >> source >> destination >> capacity; cin.ignore();
        }

        cin >> count; cin.ignore();
        for (int i = 0; i < count; i++) {
            getline(cin, pod_properties);
        }

        cin >> new_buildings_count; cin.ignore();
        for (int i = 0; i < new_buildings_count; i++) {
            cin >> type >> id >> x >> y;
            if (type == 0) {
                cin >> count;
                LandingPad new_pad = {.id = id};

                while (count--) {
                    cin >> ast_type;
                    new_pad.astronauts[--ast_type]++;
                    //state.astronauts.push_back(Astronaut{id, id, ast_type});
                }

                curr_id = state.pads.size();
                state.pads.push_back(new_pad);
            }
            else {
                LunarModule new_module = {.type = --type, .id = id};
                curr_id = state.modules.size();
                state.modules.push_back(new_module);
            }
            state.buildings.push_back(Building{.x = x, .y = y, .type = type, .id_on_list = curr_id});
        }

        state.solve();

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        // cout << "TUBE 0 1;TUBE 0 2;POD 42 0 1 0 2 0 1 0 2" << endl; // TUBE | UPGRADE | TELEPORT | POD | DESTROY | WAIT
    }
}
