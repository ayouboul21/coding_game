#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>

using namespace std;

// global variable oppenent root coordinates
pair<int, int> opponent_root;

struct my_proteins {
    int my_a;
    int my_b;
    int my_c;
    int my_d;

    // Parameterized Constructor
    my_proteins(int my_a, int my_b, int my_c, int my_d)
        : my_a(my_a), my_b(my_b), my_c(my_c), my_d(my_d) {}

    // Copy Constructor
    my_proteins(const my_proteins &other)
        : my_a(other.my_a), my_b(other.my_b), my_c(other.my_c), my_d(other.my_d) {}
};


typedef struct block
{
    int x;
    int y; // grid coordinate
    int harvested; // 1 if your organ, -1 if enemy organ, 0 if neither
    int owner; // 1 if your organ, 0 if enemy organ, -1 if neither
    int organ_id; // id of this entity if it's an organ, 0 otherwise
    string organ_dir; // N,E,S,W or X if not an organ
    int organ_parent_id;
    int organ_root_id;
    int protein_found;
    string type; // WALL, ROOT, BASIC, TENTACLE, HARVESTER, SPORER A, B, C, D, WALL or EMPTY
    block() : x(0), y(0), type("EMPTY") {};
    block(int organ_id, int x, int y, string type) : organ_id(organ_id), x(x), y(y), type(type) {};
    block(int x, int y, string type, int harvested) : x(x), y(y), type(type), harvested(harvested), protein_found(0)  {};
    block(int x, int y) : x(x), y(y), type("EMPTY") {};
    block(int x, int y, string type) : x(x), y(y), type(type) {};
    block(int x, int y, string type, int owner, int harvested) : x(x), y(y), type(type), protein_found(owner), harvested(harvested) {};
    block(int organ_id, int x, int y, string type, int owner, string organ_dir, int organ_parent_id, int organ_root_id) : organ_id(organ_id), x(x), y(y), type(type), owner(owner), organ_dir(organ_dir), organ_parent_id(organ_parent_id), organ_root_id(organ_root_id) {};
    bool operator==(const block& other) const {
        return x == other.x && y == other.y && type == other.type;
    }
    block& operator=(const block& other) {
        x = other.x;
        y = other.y;
        harvested = other.harvested;
        owner = other.owner;
        organ_id = other.organ_id;
        organ_dir = other.organ_dir;
        organ_parent_id = other.organ_parent_id;
        organ_root_id = other.organ_root_id;
        protein_found = other.protein_found;
        type = other.type;
        return *this;
    }
    virtual ~block() {};
} block;

struct Node {
    int x, y;
    double cost;
    bool operator<(const Node& other) const {
        return cost > other.cost; // Min-heap priority queue
    }
};

vector<pair<int, int>> get_neighbors(int x, int y) {
    return {
        {x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1} // Cardinal directions
        // Add diagonal movements if needed
    };
}

bool is_walkable(map<pair<int, int>, block>& grid, pair<int, int> coord) {
    if (grid.find(coord) == grid.end()) return false;
    return grid[coord].type != "WALL";
}

void print_path(map<pair<int, int>, pair<int, int>>& came_from, pair<int, int> start, pair<int, int> target) {
    pair<int, int> current = target;
    while (current != start) {
        cerr << current.first << " " << current.second << endl;
        current = came_from[current];
    }
}

double heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2); // Manhattan distance
}

vector<pair<int, int>> find_path(map<pair<int, int>, block>& grid, pair<int, int> start, pair<int, int> target) {
    priority_queue<Node> pq;
    map<pair<int, int>, double> cost_so_far;
    map<pair<int, int>, pair<int, int>> came_from;

    pq.push({start.first, start.second, 0});
    cost_so_far[start] = 0;

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (make_pair(current.x, current.y) == target) break;

        for (auto [nx, ny] : get_neighbors(current.x, current.y)) {
            pair<int, int> neighbor = {nx, ny};
            if (!is_walkable(grid, neighbor)) continue;

            double new_cost = cost_so_far[{current.x, current.y}] + 1; // Assuming uniform cost
            if (cost_so_far.find(neighbor) == cost_so_far.end() || new_cost < cost_so_far[neighbor]) {
                cost_so_far[neighbor] = new_cost;
                double priority = new_cost + heuristic(nx, ny, target.first, target.second);
                pq.push({nx, ny, priority});
                came_from[neighbor] = {current.x, current.y};
            }
        }
    }

    // Reconstruct path
    vector<pair<int, int>> path;
    if (came_from.find(target) == came_from.end())
    {
        cerr << "No path found" << endl;
    }


    pair<int, int> current = came_from[target];
    while (current != start) {
        if (grid[current].owner == 1 && (grid[current].type == "BASIC" || grid[current].type == "TENTACLE" || grid[current].type == "SPORER" || grid[current].type == "HARVESTER")) {
            cerr << "got here" << endl;
            break;
        }
        path.push_back(current);
        current = came_from[current];
    }
    // path.push_back(start);
    reverse(path.begin(), path.end());

    return path;
}

int Organ_exists(block curr_organ, vector<block> my_organs)
{
    for (int i = 0; i < my_organs.size(); i++)
    {
        if (my_organs[i] == curr_organ)
            return 1;
    }
    return 0;
}

int check_if_adjacent(block &organ, block &protein)
{
    if (organ.x == protein.x && organ.y == protein.y + 1 && organ.organ_dir == "N")
        return 1;
    if (organ.x == protein.x && organ.y == protein.y - 1 && organ.organ_dir == "S")
        return 1;
    if (organ.x == protein.x + 1 && organ.y == protein.y && organ.organ_dir == "W")
        return 1;
    if (organ.x == protein.x - 1 && organ.y == protein.y && organ.organ_dir == "E")
        return 1;
    return 0;
}

void check_proteins(vector<block> &my_organs, vector<block> &proteins)
{
    for (int i = 0; i < my_organs.size(); i++)
    {
        if (my_organs[i].type == "HARVESTER")
        {
            for (int j = 0; j < proteins.size(); j++)
            {
                proteins[j].protein_found = 0;
                if (check_if_adjacent(my_organs[i], proteins[j]))
                {
                    proteins[j].harvested = 1;
                }
            }
        }
    }
}

map<pair<int, int>, block> update_map(vector<block> my_organs, vector<block> enemy_organs, vector<block> proteins, vector<block> walls, int width, int height)
{
    map<pair<int, int>, block> grid;
    for (int i = 0; i < my_organs.size(); i++)
    {
        grid[make_pair(my_organs[i].x, my_organs[i].y)] = my_organs[i];
    }
    for (int i = 0; i < enemy_organs.size(); i++)
    {
        grid[make_pair(enemy_organs[i].x, enemy_organs[i].y)] = enemy_organs[i];
    }
    for (int i = 0; i < proteins.size(); i++)
    {
        grid[make_pair(proteins[i].x, proteins[i].y)] = proteins[i];
    }
    for (int i = 0; i < walls.size(); i++)
    {
        grid[make_pair(walls[i].x, walls[i].y)] = walls[i];
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (grid.find(make_pair(j, i)) == grid.end())
            {
                grid[make_pair(j, i)] = block(j, i);
            }
        }
    }
    return grid;
}

// Directions for BFS (up, down, left, right)
const vector<pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

// BFS to find the closest protein type, avoiding walls
pair<int, int> bfs_closest_protein(const map<pair<int, int>, block> &grid, int start_x, int start_y, const string& target_type) {
    queue<pair<int, int>> q;
    set<pair<int, int>> visited;

    q.push({start_x, start_y});
    visited.insert({start_x, start_y});

    while (!q.empty()) {
        auto [cur_x, cur_y] = q.front();
        q.pop();

        // Check if the current block is the target protein
        auto it = grid.find({cur_x, cur_y});
        if (it != grid.end() && it->second.type == target_type) {
            return {cur_x, cur_y};
        }

        // Explore neighbors
        for (const auto& dir : directions) {
            int new_x = cur_x + dir.first;
            int new_y = cur_y + dir.second;

            // Skip if already visited
            if (visited.count({new_x, new_y})) continue;

            // Check if the new block is valid (not a wall)
            auto neighbor = grid.find({new_x, new_y});
            if (neighbor != grid.end() && neighbor->second.type != "WALL") {
                q.push({new_x, new_y});
                visited.insert({new_x, new_y});
            }
        }
    }

    // Return invalid coordinates if no target protein is found
    return {-1, -1};
}

// Function to find the closest coordinates of each protein type
map<string, pair<int, int>> find_closest_proteins(const map<pair<int, int>, block>& grid, int start_x, int start_y) {
    map<string, pair<int, int>> closest_coordinates;

    // Define protein types to search for
    vector<string> protein_types = {"A", "B", "C", "D"};

    for (const auto& protein_type : protein_types) {
        closest_coordinates[protein_type] = bfs_closest_protein(grid, start_x, start_y, protein_type);
    }

    return closest_coordinates;
}

pair<int, int> get_root_cords(const vector<block>& my_organs) {
    for (const auto& organ : my_organs) {
        if (organ.type == "ROOT") {
            return {organ.x, organ.y};
        }
    }
    return {-1, -1};
}

void print_organ_type(my_proteins &pt)
{
    cerr << "Proteins: " << pt.my_a << " " << pt.my_b << " " << pt.my_c << " " << pt.my_d << endl;
    if (pt.my_a > 0)
        cout << "BASIC" << endl;
    else if (pt.my_c > 1 && !pt.my_b)
        cout << "TENTACLE W" << endl;
    else if (pt.my_d > 1 && !pt.my_b)
        cout << "SPORER W" << endl;
}

string getDirection(pair<int, int> current, pair<int, int> target) {
    // Extract the x and y coordinates
    int currentX = current.first;
    int currentY = current.second;
    int targetX = target.first;
    int targetY = target.second;

    // Determine the direction
    if (targetX > currentX) {
        return "E";  // East
    } else if (targetX < currentX) {
        return "W";  // West
    } else if (targetY > currentY) {
        return "S";  // South
    } else if (targetY < currentY) {
        return "N";  // North
    }

    // If the current position equals the target position
    return "X";  // No movement
}

int get_id_to_grow_from(map<pair<int, int>, block> &grid, pair<int, int> toGrowTo)
{
    block &toGrowToBlock = grid[toGrowTo];
    pair<int, int> parentCords = {toGrowToBlock.x - 1, toGrowToBlock.y};
    pair<int, int> parentCords2 = {toGrowToBlock.x + 1, toGrowToBlock.y};
    pair<int, int> parentCords3 = {toGrowToBlock.x, toGrowToBlock.y - 1};
    pair<int, int> parentCords4 = {toGrowToBlock.x, toGrowToBlock.y + 1};
    if (grid.find(parentCords) != grid.end() && grid[parentCords].owner == 1)
    {
        return grid[parentCords].organ_id;
    }
    else if (grid.find(parentCords2) != grid.end() && grid[parentCords2].owner == 1)
    {
        return grid[parentCords2].organ_id;
    }
    else if (grid.find(parentCords3) != grid.end() && grid[parentCords3].owner == 1)
    {
        return grid[parentCords3].organ_id;
    }
    else if (grid.find(parentCords4) != grid.end() && grid[parentCords4].owner == 1)
    {
        return grid[parentCords4].organ_id;
    }
    return 1;
}

void find_next_move(map<pair<int, int>, block> &grid, vector<pair<int, int>> path, pair<int, int> rootCords, my_proteins &pt, pair<int, int> proteinCord)
{
    if (path.size() > 1)
    {
        cout << "GROW " << get_id_to_grow_from(grid, path[0]) << " " << path[0].first << " " << path[0].second << " ";
        print_organ_type(pt);
    }
    else if (!path.size())
    {
        cout << "GROW " << get_id_to_grow_from(grid, proteinCord) << " " << proteinCord.first << " " << proteinCord.second << " ";
        print_organ_type(pt);
    }
    else if (pt.my_c > 1 && pt.my_d > 1)
    {
        cout << "GROW " << get_id_to_grow_from(grid, path[0]) << " " << path[0].first << " " << path[0].second << " HARVESTER " << getDirection(path[0], proteinCord) << endl;
    }
}
// Function to find the next move to go in the path

void simulate(map<pair<int, int>, block> &grid, my_proteins &pt, map<string, pair<int, int>>& closest, pair<int, int> rootCords)
{
    int m;
    vector<int> list;
    if (!grid[closest["A"]].harvested)
        list.push_back(pt.my_a);
    if (!grid[closest["B"]].harvested)
        list.push_back(pt.my_b);
    if (!grid[closest["C"]].harvested)
        list.push_back(pt.my_c);
    if (!grid[closest["D"]].harvested)
        list.push_back(pt.my_d);
    if (!list.size())
        m = -1;
    else
        m = *min_element(list.begin(), list.end());
    if(m == pt.my_a && !grid[closest["A"]].harvested)
    {
        vector<pair<int, int>> path = find_path(grid, rootCords, closest["A"]);
        //print the path in cerr
        cerr << "A ";
        for(auto p : path)
        {
            cerr << p.first << " " << p.second << " ";
        }
        cerr << endl;
        find_next_move(grid, path, rootCords, pt, closest["A"]);
    }
    else if(m == pt.my_b && !grid[closest["B"]].harvested)
    {
        vector<pair<int, int>> path = find_path(grid, rootCords, closest["B"]);
        cerr << "B ";
        for(auto p : path)
        {
            cerr << p.first << " " << p.second << " ";
        }
        cerr << endl;
        find_next_move(grid, path, rootCords, pt, closest["B"]);
    }
    else if(m == pt.my_c && !grid[closest["C"]].harvested)
    {
        vector<pair<int, int>> path = find_path(grid, rootCords, closest["C"]);
        cerr << "C ";
        for(auto p : path)
        {
            cerr << p.first << " " << p.second << " ";
        }
        cerr << endl;
        find_next_move(grid, path, rootCords, pt, closest["C"]);
    }
    else if(m == pt.my_d && !grid[closest["D"]].harvested)
    {
        vector<pair<int, int>> path = find_path(grid, rootCords, closest["D"]);
        cerr << "D ";
        for(auto p : path)
        {
            cerr << p.first << " " << p.second << endl;
        }
        cerr << endl;
        find_next_move(grid, path, rootCords, pt, closest["D"]);
    }
    else
    {
        vector<pair<int, int>> path = find_path(grid, rootCords, opponent_root);
        if (!path.size())
            cout << "WAIT" << endl;
        else
        {
            cerr << "ATTACK ";
            for(auto p : path)
            {
                cerr << p.first << " " << p.second << " ";
                cerr << grid[p].type << " " << grid[p].owner << " ";
            }
            cerr << endl;

            cout << "GROW " << get_id_to_grow_from(grid, path[0]) << " " << path[0].first << " " << path[0].second << " TENTACLE " << getDirection(path[0], path[1]) << endl;
        }
    }
}

void print_proteins_by_type(vector<block> proteins, string type)
{
    cerr << "Proteins of type " << type << " " << endl;
    for (int i = 0; i < proteins.size(); i++)
    {
        if (proteins[i].type == type)
        {
            cerr << proteins[i].x << " " << proteins[i].y << " " << proteins[i].harvested << endl;
        }
    }
}

int protein_exists(block protein, vector<block> &proteins)
{
    for (int i = 0; i < proteins.size(); i++)
    {
        if (proteins[i] == protein)
        {
            proteins[i].protein_found = 1;
            return 1;
        }
    }
    return 0;
}

void print_grid(map<pair<int, int>, block> grid, int width, int height)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (grid[make_pair(j, i)].type == "EMPTY")
                cerr << ".";
            else if (grid[make_pair(j, i)].type == "ROOT")
                cerr << "R";
            else if (grid[make_pair(j, i)].type == "BASIC")
                cerr << "B";
            else if (grid[make_pair(j, i)].type == "TENTACLE")
                cerr << "T";
            else if (grid[make_pair(j, i)].type == "HARVESTER")
                cerr << "H";
            else if (grid[make_pair(j, i)].type == "SPORER")
                cerr << "S";
            else if (grid[make_pair(j, i)].type == "A")
                cerr << "a";
            else if (grid[make_pair(j, i)].type == "B")
                cerr << "b";
            else if (grid[make_pair(j, i)].type == "C")
                cerr << "c";
            else if (grid[make_pair(j, i)].type == "D")
                cerr << "d";
            else if (grid[make_pair(j, i)].type == "WALL")
                cerr << "W";
        }
        cerr << endl;
    }
}

void empty(vector<block> &proteins)
{
    for (int i = 0; i < proteins.size(); i++)
        proteins.erase(proteins.begin() + i);
}

int main()
{
    int width; // columns in the game grid
    int height; // rows in the game grid
    cin >> width >> height; cin.ignore();
    vector<block> my_organs;
    vector<block> enemy_organs;
    vector<block> proteins;
    vector<block> walls;
    map<pair<int, int>, block> grid;
    int walls_added = 0;

    // game loop
    while (1) {
        int entity_count;
        cin >> entity_count; cin.ignore();
        for (int i = 0; i < entity_count; i++) {
            int x;
            int y; // grid coordinate
            string type; // WALL, ROOT, BASIC, TENTACLE, HARVESTER, SPORER, A, B, C, D
            int owner; // 1 if your organ, 0 if enemy organ, -1 if neither
            int organ_id; // id of this entity if it's an organ, 0 otherwise
            string organ_dir; // N,E,S,W or X if not an organ
            int organ_parent_id;
            int organ_root_id;
            cin >> x >> y >> type >> owner >> organ_id >> organ_dir >> organ_parent_id >> organ_root_id; cin.ignore();
            if (owner == 1 && !Organ_exists(block(organ_id, x, y, type), my_organs))
                my_organs.push_back(block(organ_id, x, y, type, owner, organ_dir, organ_parent_id, organ_root_id));
            if (owner == 0 && !Organ_exists(block(organ_id, x, y, type), enemy_organs))
                enemy_organs.push_back(block(organ_id, x, y, type, owner, organ_dir, organ_parent_id, organ_root_id));
            if ((type == "A" || type == "B" || type == "C" || type == "D") && !protein_exists(block(x, y, type), proteins))
                proteins.push_back(block(x, y, type, owner, 0));
            if (type == "WALL" && walls_added == 0)
                walls.push_back(block(x, y, type));
        }
        if (!walls_added)
            walls_added++;
        pair<int, int> rootCords = get_root_cords(my_organs);
        opponent_root = get_root_cords(enemy_organs);
        check_proteins(my_organs, proteins);
        // print_proteins_by_type(proteins, "A");
        grid = update_map(my_organs, enemy_organs, proteins, walls, width, height);
        // print_grid(grid, width, height);
        map<string, pair<int, int>> closest = find_closest_proteins(grid, rootCords.first, rootCords.second);
        int my_a;
        int my_b;
        int my_c;
        int my_d; // your protein stock
        cin >> my_a >> my_b >> my_c >> my_d; cin.ignore();
        int opp_a;
        int opp_b;
        int opp_c;
        int opp_d; // opponent's protein stock
        cin >> opp_a >> opp_b >> opp_c >> opp_d; cin.ignore();
        int required_actions_count; // your number of organisms, output an action for each one in any order
        cin >> required_actions_count; cin.ignore();
        my_proteins pt(my_a, my_b, my_c, my_d);
        for (int i = 0; i < required_actions_count; i++) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;
            if (i == 0)
                simulate(grid, pt, closest, rootCords);
            else if (i == 1)
                cout << "WAIT" << endl;
        }
        empty(proteins);
        empty(my_organs);
        empty(enemy_organs);
    }
}
