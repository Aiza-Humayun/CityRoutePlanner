
#include <iostream>
#include <string>
#include <fstream>

#include "Json.h"
#include "Graph.h"
#include "Stack.h"
#include "LinkedList.h"
#include "Sorting.h"
#include "HashTable.h"
#include "AVLTree.h"
#include "DisjointSet.h"
#include "TripHistory.h"
#include "MapManager.h"
#include "NetworkAnalysis.h"

// ------------------------------------------------------------ tiny test helpers
static int passed = 0;
static int failed = 0;

// Records one check. 'condition' must be true to pass.
static void check(bool condition, const std::string& description) {
    if (condition) { passed++; std::cout << "  PASS  " << description << "\n"; }
    else           { failed++; std::cout << "  FAIL  " << description << "\n"; }
}

// true if 'text' contains 'part'
static bool has(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// how many times 'part' appears in 'text'
static int countOf(const std::string& text, const std::string& part) {
    int n = 0;
    size_t pos = 0;
    while ((pos = text.find(part, pos)) != std::string::npos) { n++; pos += part.size(); }
    return n;
}

static bool isOk(const std::string& reply)  { return has(reply, "\"ok\":true"); }
static bool isErr(const std::string& reply) { return has(reply, "\"ok\":false"); }

static std::string lower(const std::string& s) {
    std::string o = s;
    for (size_t i = 0; i < o.size(); i++) if (o[i] >= 'A' && o[i] <= 'Z') o[i] = (char)(o[i] + 32);
    return o;
}

// ============================================================== HashTable tests
static void testHashTable() {
    std::cout << "\n[HashTable]\n";
    HashTable h;
    h.insert("Saddar", 2);
    check(h.find("Saddar") == 2, "find returns the stored id");
    check(h.find("saddar") == 2 && h.find("SADDAR") == 2, "lookup ignores capitalisation");
    check(h.find("Nowhere") == -1, "unknown name returns -1");
    check(h.insert("Saddar", 9) == false && h.find("Saddar") == 9, "inserting an existing name updates it");
    check(h.size() == 1, "size is 1 after updating (no duplicate)");

    // Insert many names so the table must rehash (collisions and growth).
    HashTable big;
    for (int i = 0; i < 100; i++) big.insert("Place" + std::to_string(i), i);
    bool allFound = true;
    for (int i = 0; i < 100; i++) if (big.find("place" + std::to_string(i)) != i) allFound = false;
    check(allFound, "100 names all found after several rehashes");
    check(big.getRehashCount() >= 2, "table rehashed (grew) at least twice");
    check(big.getCapacity() > 16, "capacity grew beyond the initial 16");
    check(big.size() == 100, "size is 100");

    check(big.remove("PLACE5") == true && big.find("Place5") == -1, "remove works (case-insensitive)");
    check(big.remove("Place5") == false, "removing a missing name returns false");
    check(big.size() == 99, "size decreased after remove");

    // Rule of Three: copy must be independent of the original.
    HashTable copy(big);
    copy.remove("Place7");
    check(big.find("Place7") == 7 && copy.find("Place7") == -1, "copy constructor makes an independent copy");
    HashTable assigned;
    assigned = big;
    assigned.insert("Extra", 1);
    check(big.find("Extra") == -1 && assigned.find("Extra") == 1, "copy assignment makes an independent copy");
}

// ================================================================ AVLTree tests
static void testAVL() {
    std::cout << "\n[AVLTree]\n";
    AVLTree t;
    const char* sample[] = {"Saddar", "Clifton", "DHA", "Korangi", "Malir", "Airport", "Lyari",
                            "Keamari", "Nazimabad", "Gulshan", "Johar", "University", "Orangi",
                            "Baldia", "Defence", "Zamzama", "Burns Road", "Sea View", "Shah Faisal", "Tariq Road"};
    for (int i = 0; i < 20; i++) t.insert(sample[i], i);
    check(t.size() == 20, "20 names inserted");
    check(t.isBalanced(), "tree is balanced after 20 inserts");
    check(t.insert("SADDAR", 99) == false && t.size() == 20, "duplicate (different case) is rejected");
    check(t.find("saddar") == 0 && t.find("Tariq Road") == 19, "find returns the right ids");
    check(t.find("Nothing") == -1, "find returns -1 for a missing name");

    // In-order must be alphabetical.
    std::string names[20]; int ids[20];
    int n = t.toArray(names, ids);
    bool sortedOk = (n == 20);
    for (int i = 1; i < n; i++) if (lower(names[i - 1]) > lower(names[i])) sortedOk = false;
    check(sortedOk, "toArray returns names in alphabetical order");

    // Prefix search.
    std::string pn[20]; int pi[20];
    int c = t.prefixSearch("s", pn, pi);
    check(c == 3, "prefix 's' finds exactly 3 names (Saddar, Sea View, Shah Faisal)");
    check(c == 3 && pn[0] == "Saddar" && pn[1] == "Sea View" && pn[2] == "Shah Faisal", "prefix matches come out in alphabetical order");
    check(t.prefixSearch("SA", pn, pi) == 1, "prefix search ignores capitalisation");
    check(t.prefixSearch("x", pn, pi) == 0, "prefix with no match returns 0");
    check(t.prefixSearch("", pn, pi) == 20, "empty prefix returns everything");

    // Sorted insertion is the worst case for a plain BST; AVL must stay shallow.
    AVLTree seq;
    for (int i = 0; i < 100; i++) {
        std::string s = std::to_string(1000 + i);   // "1000", "1001", ... already in order
        seq.insert("N" + s, i);
    }
    check(seq.isBalanced(), "100 names inserted in sorted order stay balanced");
    check(seq.height() <= 9, "height of 100 sorted inserts is at most 9 (plain BST would be 100)");

    // Remove half and check balance again.
    for (int i = 0; i < 20; i += 2) t.remove(sample[i]);
    check(t.size() == 10, "10 names left after removing 10");
    check(t.isBalanced(), "tree still balanced after 10 removals");
    check(t.find(sample[0]) == -1 && t.find(sample[1]) == 1, "removed names gone, others still found");
    for (int i = 0; i < 100; i += 3) seq.remove("N" + std::to_string(1000 + i));
    check(seq.isBalanced(), "large tree still balanced after many removals");
    check(t.remove("Nothing") == false, "removing a missing name returns false");

    AVLTree copy(t);
    copy.remove(sample[1]);
    check(t.find(sample[1]) == 1 && copy.find(sample[1]) == -1, "copy constructor makes an independent copy");
}

// ============================================================= DisjointSet tests
static void testDisjointSet() {
    std::cout << "\n[DisjointSet]\n";
    DisjointSet d(6);
    check(d.setCount() == 6, "starts with 6 separate sets");
    check(d.unite(0, 1) && d.unite(2, 3), "two merges succeed");
    check(d.connected(0, 1) && !d.connected(0, 2), "connected / not connected reported correctly");
    check(d.unite(1, 2), "merging two groups succeeds");
    check(d.connected(0, 3), "0 and 3 now connected through the merge");
    check(d.unite(0, 3) == false, "uniting elements already together returns false (would be a loop)");
    check(d.setCount() == 3, "3 sets left (0-1-2-3, 4, 5)");
    check(d.find(-1) == -1 && d.find(6) == -1, "out of range find returns -1");
}

// ============================================================= TripHistory tests
static void testHistory() {
    std::cout << "\n[TripHistory]\n";
    TripHistory h;
    check(h.toJson() == "{\"ok\":true,\"history\":[]}", "empty history gives an empty list");
    for (int i = 0; i < 55; i++) h.add("trip " + std::to_string(i));
    check(h.size() == 50, "only the 50 most recent entries are kept");
    std::string j = h.toJson();
    check(j.find("{\"ok\":true,\"history\":[\"trip 54\"") == 0, "newest entry comes first");
    check(!has(j, "\"trip 4\"") && has(j, "\"trip 5\""), "oldest entries were dropped");
    h.add("line one\nline two");
    check(!has(h.toJson(), "\\n"), "line breaks inside an entry are replaced");

    h.saveToFile("tests/_tmp_history.txt");
    TripHistory h2;
    check(h2.loadFromFile("tests/_tmp_history.txt") && h2.size() == 50, "history save/load round trip (bonus)");
}

// ============================================================ MapManager tests
static void testMapManager() {
    std::cout << "\n[MapManager]\n";
    Graph g;
    MapManager m(g);

    std::string r = m.loadFromFile("data/map.txt");
    check(isOk(r), "data/map.txt loads: " + r);
    check(g.activeCount() >= 12, "sample map has at least 12 locations");
    int roads = countOf(m.getMapJson(), "\"dist\":");
    check(roads >= 20, "sample map has at least 20 roads");
    check(has(m.getMapJson(), "\"blocked\":false"), "GET_MAP reports the blocked flag");

    int saddar = m.findId("saddar");
    int clifton = m.findId("CLIFTON");
    check(saddar >= 0 && clifton >= 0, "findId works for any capitalisation");
    check(m.findId("  Saddar  ") == saddar, "findId ignores spaces around the name");
    check(m.findId("Atlantis") == -1, "unknown location gives -1");
    check(has(m.findLocation("Saddar"), "\"found\":true"), "FIND finds an existing location");
    check(has(m.findLocation("Atlantis"), "\"found\":false") && isOk(m.findLocation("Atlantis")), "FIND of a missing name is ok with found:false");

    // ---------- adding locations
    check(isOk(m.addLocation("Test Town", 100, 100)), "add a location");
    check(m.findId("test town") >= 0, "new location can be found by name");
    check(isErr(m.addLocation("TEST TOWN", 50, 50)), "duplicate name (any case) is rejected");
    check(isErr(m.addLocation("", 1, 1)), "empty name is rejected");
    check(isErr(m.addLocation("   ", 1, 1)), "name of only spaces is rejected");
    check(isErr(m.addLocation("A|B", 1, 1)), "name with | is rejected");
    check(isErr(m.addLocation("A,B", 1, 1)), "name with a comma is rejected");
    check(isErr(m.addLocation("1234567890123456789012345678901", 1, 1)), "name of 31 characters is rejected");
    check(isErr(m.addLocation("Bad X", -5, 10)), "negative x is rejected");
    check(isErr(m.addLocation("Bad Y", 10, 6000)), "y above 5000 is rejected");
    check(m.findId("Bad X") == -1 && m.findId("Bad Y") == -1, "rejected locations were not added");

    std::string listing = m.listSorted();
    check(listing.find("\"Saddar\"") < listing.find("\"Test Town\""), "LIST_SORTED puts Saddar before Test Town");
    check(has(m.searchPrefix("test"), "\"Test Town\"") && !has(m.searchPrefix("test"), "Saddar"), "SEARCH by prefix works");

    // ---------- roads
    int town = m.findId("Test Town");
    check(isErr(m.addRoad(town, town, 1, 1)), "road from a location to itself is rejected");
    check(isErr(m.addRoad(town, 9999, 1, 1)), "road to an unknown location is rejected");
    check(isErr(m.addRoad(town, saddar, 0, 5)), "distance 0 is rejected");
    check(isErr(m.addRoad(town, saddar, 5, -2)), "negative time is rejected");
    check(isOk(m.addRoad(town, saddar, 4.5, 12)) && g.hasEdge(town, saddar), "add a road");
    check(isErr(m.addRoad(saddar, town, 3, 3)), "duplicate road (either direction) is rejected");
    check(isOk(m.updateRoad(town, saddar, 6, 15)), "update a road");
    check(isErr(m.updateRoad(town, clifton, 1, 1)), "updating a road that does not exist is rejected");
    check(isOk(m.removeRoad(saddar, town)) && !g.hasEdge(town, saddar), "remove a road");
    check(isErr(m.removeRoad(saddar, town)), "removing a missing road is rejected");

    // ---------- rename keeps hash table and AVL tree in sync
    check(isOk(m.updateLocation(town, "Sample Town", 120, 130)), "rename a location");
    check(m.findId("Sample Town") == town && m.findId("Test Town") == -1, "hash table updated after rename");
    check(has(m.searchPrefix("sample"), "Sample Town") && !has(m.listSorted(), "Test Town"), "AVL tree updated after rename");
    check(isErr(m.updateLocation(town, "Saddar", 1, 1)), "renaming to another location's name is rejected");
    check(isOk(m.updateLocation(town, "SAMPLE TOWN", 1, 1)), "changing only capitalisation of its own name is allowed");
    check(isErr(m.updateLocation(9999, "X", 1, 1)), "updating an unknown id is rejected");

    // ---------- undo
    // (Start from a clean load so the undo stack is empty.)
    check(isOk(m.loadFromFile("data/map.txt")), "reload the sample map");
    check(isErr(m.undo()) && has(m.undo(), "Nothing to undo"), "undo with nothing to undo gives an error");

    std::string before = m.getMapJson();
    saddar = m.findId("Saddar");
    check(isOk(m.removeLocation(saddar)), "remove Saddar (it has several roads)");
    check(m.findId("Saddar") == -1 && !has(m.getMapJson(), "\"name\":\"Saddar\""), "Saddar is gone from graph and hash table");
    check(!has(m.listSorted(), "\"Saddar\""), "Saddar is gone from the AVL tree");
    check(isOk(m.undo()), "undo the removal");
    check(m.getMapJson() == before, "map is exactly as before after undo (locations, roads, values)");
    check(m.findId("Saddar") >= 0 && has(m.listSorted(), "\"Saddar\""), "hash table and AVL tree are rebuilt after undo");

    // Several edits, then undo them all in reverse order.
    std::string start = m.getMapJson();
    m.addLocation("Undo One", 10, 10);
    m.addLocation("Undo Two", 20, 20);
    m.addRoad(m.findId("Undo One"), m.findId("Undo Two"), 2, 3);
    m.updateRoad(m.findId("Undo One"), m.findId("Undo Two"), 5, 6);
    m.removeRoad(m.findId("Undo One"), m.findId("Undo Two"));
    for (int i = 0; i < 5; i++) m.undo();
    check(m.getMapJson() == start, "5 edits undone one by one return to the start");
    check(isErr(m.undo()), "undo stack is empty again");

    // A failed edit must not create an undo step.
    m.addLocation("", 1, 1);
    check(isErr(m.undo()), "a rejected edit does not add an undo step");

    // ---------- save and load
    m.addLocation("Saved Place", 300, 300);
    m.addRoad(m.findId("Saved Place"), m.findId("Clifton"), 3.25, 9.5);
    g.setBlocked(m.findId("Saddar"), m.findId("Clifton"), true);     // a blocked road must survive too
    std::string beforeSave = m.getMapJson();
    check(isOk(m.saveToFile("tests/_tmp_save.txt")), "save the map to a file");

    Graph g2;
    MapManager m2(g2);
    check(isOk(m2.loadFromFile("tests/_tmp_save.txt")), "load the saved file into a new map");
    check(m2.getMapJson() == beforeSave, "loaded map equals the saved map (incl. decimals and blocked road)");
    check(has(m2.getMapJson(), "\"blocked\":true"), "blocked flag survived save and load");
    check(m2.findId("saved place") >= 0 && has(m2.listSorted(), "Saved Place"), "indexes are built after load");
    check(isErr(m2.undo()), "load clears the undo stack");

    // ---------- bad files must never crash or damage the current map
    std::string safe = m2.getMapJson();
    check(isErr(m2.loadFromFile("data/no_such_file.txt")), "missing file gives an error");
    {
        std::ofstream bad("tests/_tmp_bad.txt");
        bad << "LOCATIONS\n0,A,10,10\n1,B,notanumber,20\nROADS\nEND\n";
    }
    std::string badReply = m2.loadFromFile("tests/_tmp_bad.txt");
    check(isErr(badReply) && has(badReply, "Line 3"), "corrupt file gives an error with the line number");
    check(m2.getMapJson() == safe, "the previous map is kept after a failed load");
    {
        std::ofstream bad("tests/_tmp_bad.txt");
        bad << "LOCATIONS\n0,A,10,10\n";                     // no END line
    }
    check(has(m2.loadFromFile("tests/_tmp_bad.txt"), "missing the END"), "file without END is reported");
    {
        std::ofstream bad("tests/_tmp_bad.txt");
        bad << "LOCATIONS\n0,A,10,10\n1,A,20,20\nROADS\nEND\n";   // duplicate name
    }
    check(has(m2.loadFromFile("tests/_tmp_bad.txt"), "duplicate"), "duplicate names in a file are reported");
    {
        std::ofstream bad("tests/_tmp_bad.txt");
        bad << "LOCATIONS\n0,A,10,10\n1,B,20,20\nROADS\n0,5,1,1,0\nEND\n";   // road to a missing location
    }
    check(isErr(m2.loadFromFile("tests/_tmp_bad.txt")), "road to a missing location is reported");
    check(m2.getMapJson() == safe, "the map is still intact after all the bad files");
}

// ========================================================= NetworkAnalysis tests
static void testNetwork() {
    std::cout << "\n[NetworkAnalysis]\n";
    // ---------- the sample map
    Graph g;
    MapManager m(g);
    NetworkAnalysis net(g);
    m.loadFromFile("data/map.txt");
    std::string conn = net.checkConnectivity();
    check(has(conn, "\"connected\":true"), "sample map is connected");
    std::string mst = net.minimumSpanningTree();
    check(has(mst, "\"connected\":true"), "MST of the sample map spans every location");
    check(countOf(mst, "\"a\":") == g.activeCount() - 1, "MST has exactly (locations - 1) roads");

    // ---------- a small map with a known answer
    //      A --1-- B
    //      |  \    |         roads: A-B 1, B-C 2, C-D 3, D-A 4, A-C 5
    //      4    5  2         MST must be A-B, B-C, C-D  = 6 km  (D-A and A-C would make loops)
    //      |      \ |
    //      D --3-- C
    Graph sq;
    MapManager sm(sq);
    NetworkAnalysis snet(sq);
    sm.addLocation("A", 0, 0); sm.addLocation("B", 100, 0);
    sm.addLocation("C", 100, 100); sm.addLocation("D", 0, 100);
    int A = sm.findId("A"), B = sm.findId("B"), C = sm.findId("C"), D = sm.findId("D");
    sm.addRoad(A, B, 1, 1); sm.addRoad(B, C, 2, 2); sm.addRoad(C, D, 3, 3);
    sm.addRoad(D, A, 4, 4); sm.addRoad(A, C, 5, 5);
    std::string sq1 = snet.minimumSpanningTree();
    check(has(sq1, "\"totalDist\":6,") && countOf(sq1, "\"a\":") == 3, "Kruskal gives total 6 km with 3 roads on the square");
    check(!has(sq1, "\"dist\":4}") && !has(sq1, "\"dist\":5}"), "the longer road of each loop is skipped");

    // ---------- blocked roads count as closed
    sq.setBlocked(B, C, true);
    sq.setBlocked(D, A, true);
    sq.setBlocked(A, C, true);                  // now: A-B and C-D only -> two groups
    std::string split = snet.checkConnectivity();
    check(has(split, "\"connected\":false"), "blocking roads can disconnect the map");
    check(has(split, "[\"A\",\"B\"]") || has(split, "[\"B\",\"A\"]"), "component list shows A and B together");
    check(has(split, "[\"C\",\"D\"]") || has(split, "[\"D\",\"C\"]"), "component list shows C and D together");
    std::string forest = snet.minimumSpanningTree();
    check(has(forest, "\"connected\":false") && countOf(forest, "\"a\":") == 2, "MST of a split map is a forest (not connected)");
    sq.setBlocked(B, C, false);
    check(has(snet.checkConnectivity(), "\"connected\":true"), "unblocking one road reconnects the map");

    // ---------- disconnected sample file
    Graph g3;
    MapManager m3(g3);
    NetworkAnalysis net3(g3);
    check(isOk(m3.loadFromFile("data/map_disconnected.txt")), "data/map_disconnected.txt loads");
    std::string dis = net3.checkConnectivity();
    check(has(dis, "\"connected\":false") && has(dis, "[\"Hawks Bay\"]"), "isolated Hawks Bay is reported as its own component");

    // ---------- edge cases
    Graph empty;
    NetworkAnalysis enet(empty);
    check(isOk(enet.checkConnectivity()) && isOk(enet.minimumSpanningTree()), "empty map does not crash");
    Graph one;
    MapManager om(one);
    NetworkAnalysis onet(one);
    om.addLocation("Only", 1, 1);
    check(has(onet.minimumSpanningTree(), "\"connected\":true") && has(onet.checkConnectivity(), "\"connected\":true"),
          "a map with one location is connected");
}

int main() {
    std::cout << "=== Part B tests ===\n";
    testHashTable();
    testAVL();
    testDisjointSet();
    testHistory();
    testMapManager();
    testNetwork();
    std::cout << "\n=== Result: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
