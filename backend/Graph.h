#pragma once
/*
 * File:    Graph.h
 * Author:  Marium Jamshed (25K-3073) - Part A, Route Engine
 * Purpose: Stores the city map as a graph.
 *          - Every location is a NODE with an id (0, 1, 2, ...).
 *          - Every road is an EDGE with a distance (km), a time (minutes)
 *            and a blocked flag.
 * Course topics: Lab 2 dynamic arrays, Lab 4 linked lists, Lab 11 graphs.
 *
 * How it is stored (ADJACENCY LIST):
 *   nodes is an array. nodes[id] keeps the location's name, x, y, whether it
 *   still exists (active), and "head": the first road of a linked list of
 *   all roads leaving that location.
 *
 *   A road between u and v is two-way, so we store it twice:
 *   one EdgeNode in u's list (to = v) and one in v's list (to = u).
 *
 * Rules:
 *   - Ids are never reused. A removed location keeps its id but is inactive.
 *   - To loop over locations:  for (id = 0; id < nodeCount(); id++)
 *                                  if (!isActive(id)) continue;
 */

#include <string>
using namespace std;

// One road in a location's list.
struct EdgeNode {
    int       to;        // id of the location at the other end
    double    dist;      // distance in km
    double    time;      // travel time in minutes
    bool      blocked;   // true = road is closed
    EdgeNode* next;      // next road in the same list
};

// One location.
struct GraphNode {
    string    name;
    double    x;         // position for drawing on the web page
    double    y;
    bool      active;    // false after the location is removed
    EdgeNode* head;      // first road leaving this location
};

class Graph {
private:
    GraphNode* nodes;      // the array of locations
    int capacity;          // size of the array
    int used;              // how many ids have been given out
    int activeTotal;       // how many locations still exist

    // Makes the array twice as big when it is full.
    // Time: O(n)
    void grow() {
        int newCapacity = capacity * 2;
        GraphNode* bigger = new GraphNode[newCapacity];
        for (int i = 0; i < used; i++) {
            bigger[i] = nodes[i];
        }
        delete[] nodes;
        nodes = bigger;
        capacity = newCapacity;
    }

    // Deletes all roads in one location's list.
    // Time: O(number of roads of that location)
    void deleteAllRoads(int id) {
        EdgeNode* road = nodes[id].head;
        while (road != nullptr) {
            EdgeNode* nextRoad = road->next;
            delete road;
            road = nextRoad;
        }
        nodes[id].head = nullptr;
    }

    // Finds the road from u to v. Returns nullptr if there is none.
    // Time: O(number of roads of u)
    EdgeNode* findRoad(int u, int v) const {
        if (!isActive(u)) {
            return nullptr;
        }
        EdgeNode* road = nodes[u].head;
        while (road != nullptr) {
            if (road->to == v) {
                return road;
            }
            road = road->next;
        }
        return nullptr;
    }

    // Adds ONE direction (from -> to) at the start of from's list.
    // Time: O(1)
    void addOneWay(int from, int to, double dist, double time) {
        EdgeNode* road = new EdgeNode;
        road->to = to;
        road->dist = dist;
        road->time = time;
        road->blocked = false;
        road->next = nodes[from].head;   // new road points to the old first road
        nodes[from].head = road;         // new road becomes the first road
    }

    // Removes ONE direction (from -> to). Returns false if not found.
    // Time: O(number of roads of from)
    bool removeOneWay(int from, int to) {
        EdgeNode* previous = nullptr;
        EdgeNode* road = nodes[from].head;

        // walk the list until we find the road
        while (road != nullptr && road->to != to) {
            previous = road;
            road = road->next;
        }
        if (road == nullptr) {
            return false;                       // not found
        }

        if (previous == nullptr) {
            nodes[from].head = road->next;      // it was the first road
        } else {
            previous->next = road->next;        // skip over it
        }
        delete road;
        return true;
    }

public:
    // Creates an empty map with room for 16 locations.
    Graph() {
        capacity = 16;
        nodes = new GraphNode[capacity];
        used = 0;
        activeTotal = 0;
    }

    // Frees every road and the array.
    // Time: O(V + E)
    ~Graph() {
        for (int i = 0; i < used; i++) {
            deleteAllRoads(i);
        }
        delete[] nodes;
    }

    // Rule of Three: copying is turned off. There is only one map, and a
    // copy would share the same roads and delete them twice.
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    // Removes all locations and roads. Ids start from 0 again.
    // Time: O(V + E)
    void clear() {
        for (int i = 0; i < used; i++) {
            deleteAllRoads(i);
        }
        used = 0;
        activeTotal = 0;
    }

    // Adds a location and returns its new id.
    // Time: O(1) (O(n) only when the array grows)
    int addNode(const string& name, double x, double y) {
        if (used == capacity) {
            grow();
        }
        int id = used;
        nodes[id].name = name;
        nodes[id].x = x;
        nodes[id].y = y;
        nodes[id].active = true;
        nodes[id].head = nullptr;
        used++;
        activeTotal++;
        return id;
    }

    // Removes a location and every road touching it.
    // Returns false if the id is invalid or already removed.
    // Time: O(roads of this location and of its neighbours)
    bool removeNode(int id) {
        if (!isActive(id)) {
            return false;
        }
        // first remove the copy of each road stored at the neighbour
        EdgeNode* road = nodes[id].head;
        while (road != nullptr) {
            removeOneWay(road->to, id);
            road = road->next;
        }
        // then remove this location's own roads
        deleteAllRoads(id);
        nodes[id].active = false;
        activeTotal--;
        return true;
    }

    // True if the id exists and was not removed.  Time: O(1)
    bool isActive(int id) const {
        if (id < 0 || id >= used) {
            return false;
        }
        return nodes[id].active;
    }

    // Number of ids ever given out (loop 0 .. nodeCount()-1).  Time: O(1)
    int nodeCount() const {
        return used;
    }

    // Number of locations that still exist.  Time: O(1)
    int activeCount() const {
        return activeTotal;
    }

    // Name of a location ("" for a wrong id).  Time: O(1)
    string getName(int id) const {
        if (id < 0 || id >= used) {
            return "";
        }
        return nodes[id].name;
    }

    // x position (0 for a wrong id).  Time: O(1)
    double getX(int id) const {
        if (id < 0 || id >= used) {
            return 0;
        }
        return nodes[id].x;
    }

    // y position (0 for a wrong id).  Time: O(1)
    double getY(int id) const {
        if (id < 0 || id >= used) {
            return 0;
        }
        return nodes[id].y;
    }

    // Renames a location.  Time: O(1)
    void setName(int id, const string& name) {
        if (isActive(id)) {
            nodes[id].name = name;
        }
    }

    // Moves a location.  Time: O(1)
    void setPosition(int id, double x, double y) {
        if (isActive(id)) {
            nodes[id].x = x;
            nodes[id].y = y;
        }
    }

    // Adds a two-way road. Returns false if: same location twice, a location
    // does not exist, dist or time is 0 or less, or the road already exists.
    // Time: O(roads of u)
    bool addEdge(int u, int v, double dist, double time) {
        if (u == v) {
            return false;
        }
        if (!isActive(u) || !isActive(v)) {
            return false;
        }
        if (dist <= 0 || time <= 0) {
            return false;
        }
        if (findRoad(u, v) != nullptr) {
            return false;                  // road already exists
        }
        addOneWay(u, v, dist, time);
        addOneWay(v, u, dist, time);
        return true;
    }

    // Removes the road between u and v (both directions).
    // Time: O(roads of u + roads of v)
    bool removeEdge(int u, int v) {
        if (!isActive(u) || !isActive(v)) {
            return false;
        }
        if (!removeOneWay(u, v)) {
            return false;
        }
        removeOneWay(v, u);
        return true;
    }

    // Changes distance and time of a road (both directions).
    // The blocked flag is kept as it is.
    // Time: O(roads of u + roads of v)
    bool updateEdge(int u, int v, double dist, double time) {
        if (dist <= 0 || time <= 0) {
            return false;
        }
        EdgeNode* road1 = findRoad(u, v);
        EdgeNode* road2 = findRoad(v, u);
        if (road1 == nullptr || road2 == nullptr) {
            return false;
        }
        road1->dist = dist;
        road1->time = time;
        road2->dist = dist;
        road2->time = time;
        return true;
    }

    // True if there is a road between u and v.  Time: O(roads of u)
    bool hasEdge(int u, int v) const {
        return findRoad(u, v) != nullptr;
    }

    // Closes (true) or opens (false) a road, in both directions.
    // Time: O(roads of u + roads of v)
    bool setBlocked(int u, int v, bool blocked) {
        EdgeNode* road1 = findRoad(u, v);
        EdgeNode* road2 = findRoad(v, u);
        if (road1 == nullptr || road2 == nullptr) {
            return false;
        }
        road1->blocked = blocked;
        road2->blocked = blocked;
        return true;
    }

    // First road of u's list (nullptr if none). Walk it like this:
    //   EdgeNode* road = getEdges(u);
    //   while (road != nullptr) { ...; road = road->next; }
    // Time: O(1)
    EdgeNode* getEdges(int u) const {
        if (!isActive(u)) {
            return nullptr;
        }
        return nodes[u].head;
    }
};
