#pragma once
/*
 * File:    RouteEngine.h
 * Author:  Marium Jamshed (25K-3073) - Part A, Route Engine
 * Purpose: All route-finding. Every public function returns one line of JSON.
 *          Nothing here prints to cout.
 *
 *   shortestByDistance / shortestByTime  -> Dijkstra with a min-heap
 *   fewestStops                          -> BFS with a queue
 *   multiStop                            -> nearest stop first, using Dijkstra
 *   alternateRoutes                      -> DFS with backtracking + merge sort
 *   blockRoad / unblockRoad              -> close / open a road
 *
 * Every algorithm skips blocked roads, so after blocking a road the next
 * search automatically finds a new way.
 *
 * Course topics: Lab 4 linked lists, Lab 5 queue, Lab 6 recursion and
 *                backtracking, Lab 7 merge sort, Lab 10 heaps, Lab 11 graphs.
 */

#include <string>
#include "Graph.h"
#include "MinHeap.h"
#include "LinkedList.h"
#include "Queue.h"
#include "Sorting.h"
#include "Json.h"
using namespace std;

const double INF = 1000000000.0;   // means "cannot reach"
const int MAX_ROUTES = 200;        // alternate routes: stop after finding 200
const int MAX_HOPS = 12;           // alternate routes: ignore paths longer than 12 roads
const int ROUTES_TO_SHOW = 5;      // alternate routes: show the best 5

// One full route (used to collect and sort alternate routes).
struct Route {
    SinglyLinkedList<int> path;   // location ids from source to destination
    double totalDist;
    double totalTime;
    int hops;                     // number of roads

    Route() {
        totalDist = 0;
        totalTime = 0;
        hops = 0;
    }
};

// Compare functions for mergeSort: true if a should come before b.
bool compareByDist(const Route& a, const Route& b) {
    return a.totalDist < b.totalDist;
}
bool compareByTime(const Route& a, const Route& b) {
    return a.totalTime < b.totalTime;
}
bool compareByHops(const Route& a, const Route& b) {
    return a.hops < b.hops;
}

class RouteEngine {
private:
    Graph& graph;                 // the one shared map

    // Used by the alternate-routes DFS (set up in alternateRoutes)
    Route* foundRoutes;           // routes found so far
    int    foundCount;
    bool*  visited;               // locations already on the current path
    int    pathIds[MAX_HOPS + 1]; // the current path being explored
    int    target;                // destination of the DFS

public:
    // Saves a reference to the map. A reference must be set this way.
    RouteEngine(Graph& g) : graph(g) {
        foundRoutes = nullptr;
        foundCount = 0;
        visited = nullptr;
        target = -1;
    }

    // ROUTE_DIST: shortest route in km.  Time: O((V + E) log V)
    string shortestByDistance(int src, int dst) {
        return shortestRoute(src, dst, false);
    }

    // ROUTE_TIME: fastest route in minutes.  Time: O((V + E) log V)
    string shortestByTime(int src, int dst) {
        return shortestRoute(src, dst, true);
    }

    // ROUTE_FEWEST: route with the fewest roads, using BFS.
    // BFS visits everything 1 road away, then 2 roads away, and so on,
    // so the first time it reaches dst it used the fewest roads.
    // Time: O(V + E)
    string fewestStops(int src, int dst) {
        if (!graph.isActive(src)) {
            return jsonError("Unknown source location");
        }
        if (!graph.isActive(dst)) {
            return jsonError("Unknown destination location");
        }

        int n = graph.nodeCount();
        bool* seen = new bool[n];
        int* parent = new int[n];       // parent[v] = location we came from
        for (int i = 0; i < n; i++) {
            seen[i] = false;
            parent[i] = -1;
        }

        Queue<int> q;
        seen[src] = true;
        q.enqueue(src);

        while (!q.isEmpty()) {
            int u = q.dequeue();
            if (u == dst) {
                break;                              // reached the destination
            }
            EdgeNode* road = graph.getEdges(u);
            while (road != nullptr) {
                int v = road->to;
                if (!road->blocked && graph.isActive(v) && !seen[v]) {
                    seen[v] = true;
                    parent[v] = u;
                    q.enqueue(v);
                }
                road = road->next;
            }
        }

        SinglyLinkedList<int> path;
        bool found = makePath(src, dst, parent, path);
        delete[] seen;
        delete[] parent;

        if (!found) {
            return noRouteError(src, dst);
        }
        return "{\"ok\":true," + routeFields(path) + "}";
    }

    // MULTI: start at src and visit every stop (2 to 6 stops).
    // Method (nearest neighbour): from where we are, run Dijkstra, go to the
    // closest stop not visited yet, and repeat until all stops are visited.
    // byTime = false uses distance, true uses time.
    // Time: O(k (V + E) log V) for k stops
    string multiStop(bool byTime, int src, int stops[], int stopCount) {
        // ---- check the input ----
        if (stopCount < 2 || stopCount > 6) {
            return jsonError("Give between 2 and 6 stops");
        }
        if (!graph.isActive(src)) {
            return jsonError("Unknown source location");
        }
        for (int i = 0; i < stopCount; i++) {
            if (!graph.isActive(stops[i])) {
                return jsonError("Unknown stop location");
            }
            if (stops[i] == src) {
                return jsonError("Stop " + graph.getName(src) + " is the same as the source");
            }
            for (int j = 0; j < i; j++) {
                if (stops[j] == stops[i]) {
                    return jsonError("Duplicate stop: " + graph.getName(stops[i]));
                }
            }
        }

        int n = graph.nodeCount();
        double* best = new double[n];
        int* parent = new int[n];
        bool* stopDone = new bool[stopCount];
        for (int i = 0; i < stopCount; i++) {
            stopDone[i] = false;
        }

        SinglyLinkedList<int> path;
        path.pushBack(src);
        string order = "";
        int current = src;

        for (int step = 0; step < stopCount; step++) {
            // costs from where we are now to every location
            dijkstra(current, byTime, best, parent);

            // find the closest stop that is not visited yet
            int nearest = -1;
            for (int i = 0; i < stopCount; i++) {
                if (stopDone[i]) {
                    continue;
                }
                if (nearest == -1 || best[stops[i]] < best[stops[nearest]]) {
                    nearest = i;
                }
            }

            // even the closest stop cannot be reached
            if (best[stops[nearest]] >= INF) {
                string name = graph.getName(stops[nearest]);
                delete[] best;
                delete[] parent;
                delete[] stopDone;
                return jsonError("Cannot reach stop: " + name);
            }

            // add this part of the trip to the full path
            SinglyLinkedList<int> leg;
            makePath(current, stops[nearest], parent, leg);
            ListNode<int>* node = leg.getHead()->next;   // skip the first one: it is already in path
            while (node != nullptr) {
                path.pushBack(node->data);
                node = node->next;
            }

            // remember the visiting order
            if (step > 0) {
                order += ",";
            }
            order += jsonQuote(graph.getName(stops[nearest]));

            stopDone[nearest] = true;
            current = stops[nearest];
        }

        delete[] best;
        delete[] parent;
        delete[] stopDone;
        return "{\"ok\":true," + routeFields(path) + ",\"order\":[" + order + "]}";
    }

    // ALTERNATES: up to 5 different routes, sorted by
    // sortKey 0 = distance, 1 = time, 2 = hops.
    // DFS tries every path; backtracking un-marks a location when we come
    // back, so other paths can use it. Limits: 200 routes, 12 roads.
    // Time: exponential in the worst case (that is why we have limits),
    //       plus O(R log R) for merge sort of R routes.
    string alternateRoutes(int src, int dst, int sortKey) {
        if (sortKey < 0 || sortKey > 2) {
            return jsonError("Sort key must be DIST, TIME or HOPS");
        }
        if (!graph.isActive(src)) {
            return jsonError("Unknown source location");
        }
        if (!graph.isActive(dst)) {
            return jsonError("Unknown destination location");
        }

        // set up the DFS
        int n = graph.nodeCount();
        foundRoutes = new Route[MAX_ROUTES];
        foundCount = 0;
        target = dst;
        visited = new bool[n];
        for (int i = 0; i < n; i++) {
            visited[i] = false;
        }

        visited[src] = true;
        pathIds[0] = src;
        findAllPaths(src, 0, 0, 0);
        delete[] visited;
        visited = nullptr;

        if (foundCount == 0) {
            delete[] foundRoutes;
            foundRoutes = nullptr;
            return noRouteError(src, dst);
        }

        // sort with merge sort
        if (sortKey == 0) {
            mergeSort(foundRoutes, foundCount, compareByDist);
        } else if (sortKey == 1) {
            mergeSort(foundRoutes, foundCount, compareByTime);
        } else {
            mergeSort(foundRoutes, foundCount, compareByHops);
        }

        // build the reply with the best 5
        int show = foundCount;
        if (show > ROUTES_TO_SHOW) {
            show = ROUTES_TO_SHOW;
        }
        string reply = "{\"ok\":true,\"routes\":[";
        for (int i = 0; i < show; i++) {
            if (i > 0) {
                reply += ",";
            }
            reply += "{" + routeFields(foundRoutes[i].path) + "}";
        }
        reply += "]}";

        delete[] foundRoutes;
        foundRoutes = nullptr;
        return reply;
    }

    // BLOCK: close a road.  Time: O(roads of u + roads of v)
    string blockRoad(int u, int v) {
        if (!graph.isActive(u) || !graph.isActive(v)) {
            return jsonError("Unknown location");
        }
        string label = graph.getName(u) + " - " + graph.getName(v);
        EdgeNode* road = findRoad(u, v);
        if (road == nullptr) {
            return jsonError("No road between " + label);
        }
        if (road->blocked) {
            return jsonError("Road " + label + " is already blocked");
        }
        graph.setBlocked(u, v, true);
        return jsonMessage("Road " + label + " blocked");
    }

    // UNBLOCK: open a road again.  Time: O(roads of u + roads of v)
    string unblockRoad(int u, int v) {
        if (!graph.isActive(u) || !graph.isActive(v)) {
            return jsonError("Unknown location");
        }
        string label = graph.getName(u) + " - " + graph.getName(v);
        EdgeNode* road = findRoad(u, v);
        if (road == nullptr) {
            return jsonError("No road between " + label);
        }
        if (!road->blocked) {
            return jsonError("Road " + label + " is not blocked");
        }
        graph.setBlocked(u, v, false);
        return jsonMessage("Road " + label + " unblocked");
    }

private:
    // Finds the road from u to v, or nullptr.  Time: O(roads of u)
    EdgeNode* findRoad(int u, int v) {
        EdgeNode* road = graph.getEdges(u);
        while (road != nullptr) {
            if (road->to == v) {
                return road;
            }
            road = road->next;
        }
        return nullptr;
    }

    // Error used when no route exists.  Time: O(1)
    string noRouteError(int src, int dst) {
        return jsonError("No route found from " + graph.getName(src) + " to " + graph.getName(dst));
    }

    // DIJKSTRA from src.
    // After it runs: best[v]   = cheapest cost from src to v (INF if unreachable)
    //                parent[v] = location just before v on that route
    // byTime = false uses distance, true uses time.
    // If a cheaper way to v is found, v is pushed again ("lazy deletion");
    // when an old copy is popped later it is skipped because done[v] is true.
    // Time: O((V + E) log V)
    void dijkstra(int src, bool byTime, double best[], int parent[]) {
        int n = graph.nodeCount();
        bool* done = new bool[n];
        for (int i = 0; i < n; i++) {
            best[i] = INF;
            parent[i] = -1;
            done[i] = false;
        }

        MinHeap heap;
        best[src] = 0;
        heap.push(src, 0);

        while (!heap.isEmpty()) {
            HeapEntry top = heap.pop();      // closest unfinished location
            int u = top.node;
            if (done[u]) {
                continue;                    // old copy: skip it
            }
            done[u] = true;                  // best[u] is now final

            EdgeNode* road = graph.getEdges(u);
            while (road != nullptr) {
                int v = road->to;
                double cost = road->dist;
                if (byTime) {
                    cost = road->time;
                }
                if (!road->blocked && graph.isActive(v) && !done[v]) {
                    if (best[u] + cost < best[v]) {   // found a cheaper way to v
                        best[v] = best[u] + cost;
                        parent[v] = u;
                        heap.push(v, best[v]);
                    }
                }
                road = road->next;
            }
        }
        delete[] done;
    }

    // Shared code for ROUTE_DIST and ROUTE_TIME.  Time: O((V + E) log V)
    string shortestRoute(int src, int dst, bool byTime) {
        if (!graph.isActive(src)) {
            return jsonError("Unknown source location");
        }
        if (!graph.isActive(dst)) {
            return jsonError("Unknown destination location");
        }

        int n = graph.nodeCount();
        double* best = new double[n];
        int* parent = new int[n];
        dijkstra(src, byTime, best, parent);

        SinglyLinkedList<int> path;
        bool found = makePath(src, dst, parent, path);
        delete[] best;
        delete[] parent;

        if (!found) {
            return noRouteError(src, dst);
        }
        return "{\"ok\":true," + routeFields(path) + "}";
    }

    // Builds the path by walking parent[] backwards from dst to src.
    // pushFront puts each location at the start, so the list reads src -> dst.
    // Returns false if src is never reached (no route).
    // Time: O(length of path)
    bool makePath(int src, int dst, int parent[], SinglyLinkedList<int>& path) {
        path.clear();
        int current = dst;
        while (current != -1) {
            path.pushFront(current);
            if (current == src) {
                return true;
            }
            current = parent[current];
        }
        path.clear();
        return false;
    }

    // Turns a path into JSON fields:
    //   "path":["A","B"],"totalDist":5,"totalTime":12,"hops":1
    // Totals are added up from the roads on the path.
    // Time: O(length of path x roads per location)
    string routeFields(SinglyLinkedList<int>& path) {
        double totalDist = 0;
        double totalTime = 0;
        int hops = 0;
        string names = "";

        ListNode<int>* node = path.getHead();
        while (node != nullptr) {
            if (names != "") {
                names += ",";
            }
            names += jsonQuote(graph.getName(node->data));

            if (node->next != nullptr) {          // there is a road to the next location
                EdgeNode* road = findRoad(node->data, node->next->data);
                if (road != nullptr) {
                    totalDist += road->dist;
                    totalTime += road->time;
                }
                hops++;
            }
            node = node->next;
        }

        return "\"path\":[" + names + "],\"totalDist\":" + numToStr(totalDist)
             + ",\"totalTime\":" + numToStr(totalTime) + ",\"hops\":" + to_string(hops);
    }

    // RECURSIVE DFS used by alternateRoutes.
    // current = where we are, hops = roads used so far,
    // dist/time = totals so far, pathIds[0..hops] = the path so far.
    void findAllPaths(int current, int hops, double dist, double time) {
        if (foundCount == MAX_ROUTES) {
            return;                                 // limit reached
        }

        if (current == target) {                    // reached: save a copy
            Route& r = foundRoutes[foundCount];
            for (int i = 0; i <= hops; i++) {
                r.path.pushBack(pathIds[i]);
            }
            r.totalDist = dist;
            r.totalTime = time;
            r.hops = hops;
            foundCount++;
            return;
        }

        if (hops == MAX_HOPS) {
            return;                                 // path too long
        }

        EdgeNode* road = graph.getEdges(current);
        while (road != nullptr) {
            int next = road->to;
            if (!road->blocked && graph.isActive(next) && !visited[next]) {
                visited[next] = true;               // step forward
                pathIds[hops + 1] = next;
                findAllPaths(next, hops + 1, dist + road->dist, time + road->time);
                visited[next] = false;              // BACKTRACK: step back
            }
            road = road->next;
        }
    }
};
