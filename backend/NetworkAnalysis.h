/*
 * =============================================================================
 * File    : NetworkAnalysis.h
 * Part    : B (Map Data and Analysis)
 * Purpose : Two questions about the road network as a whole:
 *             1. CONNECTIVITY - can you reach every location from every other one?
 *                If not, which groups of locations are cut off from each other?
 *             2. MST (minimum spanning tree) - what is the cheapest set of roads
 *                that still connects every location?
 * Course  : Lab 11 - graph traversal (DFS), minimum spanning trees (Kruskal).
 *           Lab 5  - Stack (DFS uses an explicit stack).
 *           Lab 7  - Quick sort (Kruskal sorts the roads by distance).
 *
 * BLOCKED ROADS: a blocked road is a CLOSED road, so both analyses ignore it.
 *
 * COMPLEXITY
 *   checkConnectivity : O(V + E)   (every location and road is visited once)
 *   minimumSpanningTree: O(E log E) (dominated by sorting the roads)
 * =============================================================================
 */
#pragma once
#include <string>
#include "Graph.h"
#include "Json.h"
#include "Stack.h"
#include "Sorting.h"
#include "DisjointSet.h"

// One road, used only inside the MST calculation.
struct MstEdge {
    int    u;       // one end
    int    v;       // other end
    double dist;    // road length in km (the weight Kruskal minimises)
};

// Comparator for quickSort: true when road a is SHORTER than road b.
inline bool mstEdgeShorter(const MstEdge& a, const MstEdge& b) {
    return a.dist < b.dist;
}

class NetworkAnalysis {
private:
    Graph& graph;     // the map (shared with the other parts, never copied)

public:
    explicit NetworkAnalysis(Graph& g) : graph(g) {}

    /*
     * CONNECTIVITY command.
     * Uses iterative DFS (depth-first search) with a Stack:
     *   - pick an unvisited location, give it a component number, push it
     *   - pop a location, look at its open roads, push every unvisited neighbour
     *   - when the stack is empty, one whole component has been found
     *   - repeat from any location not yet visited
     * Reply: {"ok":true,"connected":false,"components":[["A","B"],["C"]]}
     */
    std::string checkConnectivity() {
        int n = graph.nodeCount();
        if (graph.activeCount() == 0) {
            return "{\"ok\":true,\"connected\":true,\"components\":[]}";
        }

        int* component = new int[n];                 // component number per location, -1 = not visited
        for (int i = 0; i < n; i++) component[i] = -1;

        int numComponents = 0;
        Stack<int> stack;

        for (int start = 0; start < n; start++) {
            if (!graph.isActive(start) || component[start] != -1) continue;

            // Found a location we have not reached yet: it starts a new component.
            component[start] = numComponents;
            stack.push(start);
            while (!stack.isEmpty()) {
                int u = stack.pop();
                for (EdgeNode* e = graph.getEdges(u); e != nullptr; e = e->next) {
                    if (e->blocked) continue;                    // closed road: cannot travel on it
                    if (component[e->to] == -1) {
                        component[e->to] = numComponents;        // mark when pushed so it is pushed once
                        stack.push(e->to);
                    }
                }
            }
            numComponents++;
        }

        // Build the JSON: one name list per component.
        std::string out = "{\"ok\":true,\"connected\":";
        out += (numComponents <= 1) ? "true" : "false";
        out += ",\"components\":[";
        for (int c = 0; c < numComponents; c++) {
            if (c > 0) out += ",";
            out += "[";
            bool first = true;
            for (int i = 0; i < n; i++) {
                if (component[i] != c) continue;
                if (!first) out += ",";
                first = false;
                out += jsonQuote(graph.getName(i));
            }
            out += "]";
        }
        out += "]}";
        delete[] component;
        return out;
    }

    /*
     * MST command - Kruskal's algorithm:
     *   1. collect every open road once
     *   2. sort the roads from shortest to longest (quick sort)
     *   3. go through them in that order; keep a road only if its two ends are not
     *      already connected (checked with the DisjointSet). Otherwise it would
     *      create a loop, so skip it.
     *   4. stop when all locations are joined (V - 1 roads), or roads run out
     * If the map is not connected, the result is a "spanning forest" (one tree per
     * component) and "connected" is false.
     * Reply: {"ok":true,"connected":true,"totalDist":41.2,
     *         "edges":[{"a":"Saddar","b":"Clifton","dist":6}, ...]}
     */
    std::string minimumSpanningTree() {
        int n = graph.nodeCount();
        int active = graph.activeCount();

        // ---- Step 1: count, then collect, every open road once (u < v).
        int roadCount = 0;
        for (int u = 0; u < n; u++) {
            if (!graph.isActive(u)) continue;
            for (EdgeNode* e = graph.getEdges(u); e != nullptr; e = e->next) {
                if (!e->blocked && u < e->to) roadCount++;
            }
        }
        MstEdge* roads = new MstEdge[roadCount > 0 ? roadCount : 1];
        int filled = 0;
        for (int u = 0; u < n; u++) {
            if (!graph.isActive(u)) continue;
            for (EdgeNode* e = graph.getEdges(u); e != nullptr; e = e->next) {
                if (!e->blocked && u < e->to) {
                    roads[filled].u = u;
                    roads[filled].v = e->to;
                    roads[filled].dist = e->dist;
                    filled++;
                }
            }
        }

        // ---- Step 2: sort by distance (Lab 7 quick sort, from Sorting.h).
        quickSort(roads, roadCount, mstEdgeShorter);

        // ---- Step 3: Kruskal. DisjointSet starts with every location in its own set.
        DisjointSet sets(n);
        std::string edgesJson;
        double total = 0;
        int chosen = 0;
        for (int i = 0; i < roadCount; i++) {
            if (sets.unite(roads[i].u, roads[i].v)) {            // joins two different sets: keep it
                if (chosen > 0) edgesJson += ",";
                edgesJson += "{\"a\":" + jsonQuote(graph.getName(roads[i].u)) +
                             ",\"b\":" + jsonQuote(graph.getName(roads[i].v)) +
                             ",\"dist\":" + numToStr(roads[i].dist) + "}";
                total += roads[i].dist;
                chosen++;
                if (chosen == active - 1) break;                 // all locations joined: done
            }
        }
        delete[] roads;

        // A spanning tree of V locations always has exactly V - 1 roads.
        bool connected = (active <= 1) || (chosen == active - 1);
        return std::string("{\"ok\":true,\"connected\":") + (connected ? "true" : "false") +
               ",\"totalDist\":" + numToStr(total) +
               ",\"edges\":[" + edgesJson + "]}";
    }
};
