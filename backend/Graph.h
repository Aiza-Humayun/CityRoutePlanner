#pragma once
#include <string>
using namespace std;

// One road in a location's list.
struct EdgeNode {
    int       to;        
    double    dist;      
    double    time;      
    bool      blocked;   
    EdgeNode* next;      
};

// One location.
struct GraphNode {
    string    name;
    double    x;        
    double    y;
    bool      active;    
    EdgeNode* head;      
};

class Graph {
private:
    GraphNode* nodes;      
    int capacity;          
    int used;              
    int activeTotal;       

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

    void deleteAllRoads(int id) {
        EdgeNode* road = nodes[id].head;
        while (road != nullptr) {
            EdgeNode* nextRoad = road->next;
            delete road;
            road = nextRoad;
        }
        nodes[id].head = nullptr;
    }

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

    void addOneWay(int from, int to, double dist, double time) {
        EdgeNode* road = new EdgeNode;
        road->to = to;
        road->dist = dist;
        road->time = time;
        road->blocked = false;
        road->next = nodes[from].head;   
        nodes[from].head = road;        
    }

    bool removeOneWay(int from, int to) {
        EdgeNode* previous = nullptr;
        EdgeNode* road = nodes[from].head;

        // walk the list until we find the road
        while (road != nullptr && road->to != to) {
            previous = road;
            road = road->next;
        }
        if (road == nullptr) {
            return false;                      
        }

        if (previous == nullptr) {
            nodes[from].head = road->next;     
        } else {
            previous->next = road->next;        
        }
        delete road;
        return true;
    }

public:
    Graph() {
        capacity = 16;
        nodes = new GraphNode[capacity];
        used = 0;
        activeTotal = 0;
    }

    ~Graph() {
        for (int i = 0; i < used; i++) {
            deleteAllRoads(i);
        }
        delete[] nodes;
    }

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    void clear() {
        for (int i = 0; i < used; i++) {
            deleteAllRoads(i);
        }
        used = 0;
        activeTotal = 0;
    }

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

    bool removeNode(int id) {
        if (!isActive(id)) {
            return false;
        }

        EdgeNode* road = nodes[id].head;
        while (road != nullptr) {
            removeOneWay(road->to, id);
            road = road->next;
        }
        
        deleteAllRoads(id);
        nodes[id].active = false;
        activeTotal--;
        return true;
    }

    bool isActive(int id) const {
        if (id < 0 || id >= used) {
            return false;
        }
        return nodes[id].active;
    }

    int nodeCount() const {
        return used;
    }

    int activeCount() const {
        return activeTotal;
    }

    string getName(int id) const {
        if (id < 0 || id >= used) {
            return "";
        }
        return nodes[id].name;
    }

    double getX(int id) const {
        if (id < 0 || id >= used) {
            return 0;
        }
        return nodes[id].x;
    }

    double getY(int id) const {
        if (id < 0 || id >= used) {
            return 0;
        }
        return nodes[id].y;
    }

    void setName(int id, const string& name) {
        if (isActive(id)) {
            nodes[id].name = name;
        }
    }

    void setPosition(int id, double x, double y) {
        if (isActive(id)) {
            nodes[id].x = x;
            nodes[id].y = y;
        }
    }

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

    bool hasEdge(int u, int v) const {
        return findRoad(u, v) != nullptr;
    }

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

    EdgeNode* getEdges(int u) const {
        if (!isActive(u)) {
            return nullptr;
        }
        return nodes[u].head;
    }
};
