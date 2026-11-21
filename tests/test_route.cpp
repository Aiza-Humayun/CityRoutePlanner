/*
 * File:    test_route.cpp
 * Author:  Marium Jamshed (25K-3073) - Part A, Route Engine
 * Purpose: Tests Graph, MinHeap and RouteEngine on their own, using a small
 *          hand-built map. Prints one PASS/FAIL line per check.
 *
 * Build (from the project root):
 *   g++ -std=c++17 -Wall -I backend -o tests/test_route.exe tests/test_route.cpp
 * Run:
 *   tests\test_route.exe
 *
 * Test map (km, minutes):
 *
 *        A ---2km,10min--- B ---2km,10min--- D        F  (isolated)
 *        |  \______________20km,40min_______/|
 *        |                                   |
 *        +---5km,3min----- C ---5km,3min-----+
 *
 *   Shortest distance A->D : A,B,D   (4 km, 20 min)
 *   Fastest time      A->D : A,C,D   (10 km, 6 min)   <- different from distance
 *   Fewest roads      A->D : A,D     (1 road)
 */

#include <iostream>
#include <string>
#include "Graph.h"
#include "MinHeap.h"
#include "RouteEngine.h"
using namespace std;

int passed = 0, failed = 0;

// Prints PASS or FAIL for one check and counts it.
void check(const string& name, bool ok, const string& got = "") {
    if (ok) {
        cout << "PASS  " << name << "\n";
        passed++;
    }
    else {
        cout << "FAIL  " << name << "\n      got: " << got << "\n";
        failed++;
    }
}

// True if 'text' contains 'part'.
bool has(const string& text, const string& part) {
    return text.find(part) != string::npos;
}

int main() {
    // ---------------- MinHeap ----------------
    MinHeap h;
    double values[8] = {5, 1, 9, 3, 7, 2, 8, 4};
    for (int i = 0; i < 8; i++) h.push(i, values[i]);
    bool sorted = true;
    double last = -1;
    while (!h.isEmpty()) {
        HeapEntry e = h.pop();
        if (e.priority < last) sorted = false;
        last = e.priority;
    }
    check("MinHeap pops in increasing order", sorted);
    check("MinHeap pop on empty heap does not crash", h.pop().node == -1);

    // ---------------- Graph ----------------
    Graph g;
    int A = g.addNode("A", 0, 0), B = g.addNode("B", 0, 0), C = g.addNode("C", 0, 0);
    int D = g.addNode("D", 0, 0), F = g.addNode("F", 0, 0);
    g.addEdge(A, B, 2, 10);
    g.addEdge(B, D, 2, 10);
    g.addEdge(A, C, 5, 3);
    g.addEdge(C, D, 5, 3);
    g.addEdge(A, D, 20, 40);

    check("Graph ids start at 0", A == 0 && F == 4);
    check("Graph rejects duplicate road", !g.addEdge(B, A, 1, 1));
    check("Graph rejects road to itself", !g.addEdge(A, A, 1, 1));
    check("Graph rejects distance <= 0", !g.addEdge(B, C, 0, 5));
    check("Graph road is two-way", g.hasEdge(A, B) && g.hasEdge(B, A));
    check("Graph nodeCount / activeCount", g.nodeCount() == 5 && g.activeCount() == 5);

    for (int i = 0; i < 40; i++) g.addNode("X" + to_string(i), 0, 0);   // forces growth past 16
    check("Graph grows past 16 locations", g.nodeCount() == 45 && g.getName(44) == "X39");
    for (int i = 5; i < 45; i++) g.removeNode(i);
    check("Graph removeNode marks inactive", !g.isActive(10) && g.activeCount() == 5);

    RouteEngine engine(g);
    string r;

    // ---------------- Dijkstra ----------------
    r = engine.shortestByDistance(A, D);
    check("Shortest by distance A->D is A,B,D",
          has(r, "\"path\":[\"A\",\"B\",\"D\"]") && has(r, "\"totalDist\":4") &&
          has(r, "\"totalTime\":20") && has(r, "\"hops\":2"), r);

    r = engine.shortestByTime(A, D);
    check("Fastest by time A->D is A,C,D (differs from distance)",
          has(r, "\"path\":[\"A\",\"C\",\"D\"]") && has(r, "\"totalTime\":6") &&
          has(r, "\"totalDist\":10"), r);

    r = engine.shortestByDistance(A, A);
    check("Same source and destination gives one location, zero totals",
          r == "{\"ok\":true,\"path\":[\"A\"],\"totalDist\":0,\"totalTime\":0,\"hops\":0}", r);

    r = engine.shortestByDistance(A, F);
    check("Unreachable location gives an error",
          r == "{\"ok\":false,\"error\":\"No route found from A to F\"}", r);

    // ---------------- BFS ----------------
    r = engine.fewestStops(A, D);
    check("Fewest stops A->D is the direct road",
          has(r, "\"path\":[\"A\",\"D\"]") && has(r, "\"hops\":1"), r);
    r = engine.fewestStops(A, F);
    check("Fewest stops to unreachable gives an error", has(r, "\"ok\":false"), r);

    // ---------------- Block / unblock ----------------
    r = engine.blockRoad(B, D);
    check("Block road B-D", r == "{\"ok\":true,\"message\":\"Road B - D blocked\"}", r);
    r = engine.shortestByDistance(A, D);
    check("After blocking B-D, distance route re-plans to A,C,D",
          has(r, "\"path\":[\"A\",\"C\",\"D\"]"), r);
    r = engine.blockRoad(D, B);
    check("Blocking an already blocked road is an error", has(r, "already blocked"), r);
    r = engine.blockRoad(A, F);
    check("Blocking a road that does not exist is an error", has(r, "No road between A - F"), r);

    engine.blockRoad(C, D);
    engine.blockRoad(A, D);
    r = engine.shortestByTime(A, D);
    check("All roads into D blocked: no route", has(r, "No route found from A to D"), r);
    r = engine.fewestStops(A, D);
    check("BFS also ignores blocked roads", has(r, "\"ok\":false"), r);

    engine.unblockRoad(C, D);
    engine.unblockRoad(A, D);
    r = engine.unblockRoad(B, D);
    check("Unblock road B-D", r == "{\"ok\":true,\"message\":\"Road B - D unblocked\"}", r);
    r = engine.unblockRoad(B, D);
    check("Unblocking an open road is an error", has(r, "is not blocked"), r);
    r = engine.shortestByDistance(A, D);
    check("After unblocking, A,B,D is back", has(r, "\"path\":[\"A\",\"B\",\"D\"]"), r);

    // ---------------- Multi-stop ----------------
    int stops1[2] = {D, B};
    r = engine.multiStop(false, A, stops1, 2);
    check("Multi-stop A then {D,B} by distance visits B first",
          has(r, "\"path\":[\"A\",\"B\",\"D\"]") && has(r, "\"order\":[\"B\",\"D\"]") &&
          has(r, "\"totalDist\":4"), r);

    int stops2[2] = {D, C};
    r = engine.multiStop(true, A, stops2, 2);
    check("Multi-stop by time joins legs without repeating C",
          has(r, "\"path\":[\"A\",\"C\",\"D\"]") && has(r, "\"order\":[\"C\",\"D\"]"), r);

    int dup[2] = {B, B};
    r = engine.multiStop(false, A, dup, 2);
    check("Multi-stop duplicate stop is an error", has(r, "Duplicate stop: B"), r);

    int unreachable[2] = {B, F};
    r = engine.multiStop(false, A, unreachable, 2);
    check("Multi-stop unreachable stop is named", has(r, "Cannot reach stop: F"), r);

    int one[1] = {B};
    r = engine.multiStop(false, A, one, 1);
    check("Multi-stop needs at least 2 stops", has(r, "between 2 and 6"), r);

    // ---------------- Alternate routes ----------------
    r = engine.alternateRoutes(A, D, 0);
    check("Alternates by DIST: best first is A,B,D, 3 routes total",
          has(r, "\"routes\":[{\"path\":[\"A\",\"B\",\"D\"]") &&
          has(r, "[\"A\",\"C\",\"D\"]") && has(r, "[\"A\",\"D\"]"), r);
    r = engine.alternateRoutes(A, D, 1);
    check("Alternates by TIME: A,C,D first", has(r, "\"routes\":[{\"path\":[\"A\",\"C\",\"D\"]"), r);
    r = engine.alternateRoutes(A, D, 2);
    check("Alternates by HOPS: A,D first", has(r, "\"routes\":[{\"path\":[\"A\",\"D\"]"), r);
    r = engine.alternateRoutes(A, F, 0);
    check("Alternates to unreachable gives an error", has(r, "\"ok\":false"), r);
    r = engine.alternateRoutes(A, D, 7);
    check("Alternates with a bad sort key gives an error", has(r, "Sort key"), r);

    // ---------------- Removed location ----------------
    g.removeNode(B);
    r = engine.shortestByDistance(A, D);
    check("Removed location B is skipped: A,C,D", has(r, "\"path\":[\"A\",\"C\",\"D\"]"), r);
    r = engine.shortestByDistance(A, B);
    check("Route to a removed location is an error", has(r, "Unknown destination"), r);

    // ---------------- clear ----------------
    g.clear();
    check("Graph clear empties the map", g.nodeCount() == 0 && g.activeCount() == 0);

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    if (failed == 0) {
        return 0;
    }
    return 1;
}
