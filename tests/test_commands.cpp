

#include <iostream>
#include <string>
#include "CommandHandler.h"
using namespace std;

int passed = 0, failed = 0;

void check(const string& name, bool ok, const string& got = "") {
    if (ok) {
        cout << "PASS  " << name << "\n";
        passed++;
    } else {
        cout << "FAIL  " << name << "\n      got: " << got << "\n";
        failed++;
    }
}

bool has(const string& text, const string& part) { return text.find(part) != string::npos; }
bool isOk(const string& r) { return r.rfind("{\"ok\":true", 0) == 0; }
bool isErr(const string& r) { return r.rfind("{\"ok\":false", 0) == 0; }
bool oneLine(const string& r) { return !r.empty() && r.find('\n') == string::npos; }

int main() {
    CommandHandler h;
    string r;

    // ---------------- basics ----------------
    r = h.handle("GET_MAP");
    check("GET_MAP lists locations and roads", isOk(r) && has(r, "\"Saddar\"") && has(r, "\"roads\":["), r);
    check("empty and blank lines are ignored", h.handle("").empty() && h.handle("   \r").empty());
    check("EXIT is recognised (any case, with spaces)", CommandHandler::isExit(" exit ") && !CommandHandler::isExit("GET_MAP"));
    r = h.handle("DANCE|a");
    check("unknown command", r == "{\"ok\":false,\"error\":\"Unknown command: DANCE\"}", r);
    r = h.handle("get_map");
    check("command names are case-insensitive", isOk(r), r);

    // ---------------- argument and name checks ----------------
    r = h.handle("ROUTE_DIST|Saddar");
    check("missing argument", r == "{\"ok\":false,\"error\":\"Wrong number of arguments for ROUTE_DIST\"}", r);
    r = h.handle("GET_MAP|extra");
    check("extra argument", has(r, "Wrong number of arguments for GET_MAP"), r);
    r = h.handle("ROUTE_DIST|Sadar|Clifton");
    check("unknown location is named", r == "{\"ok\":false,\"error\":\"Unknown location: Sadar\"}", r);
    r = h.handle("ADD_LOCATION|Test Town|abc|100");
    check("text instead of a number", r == "{\"ok\":false,\"error\":\"Not a number: abc\"}", r);
    r = h.handle("ADD_ROAD|Saddar|Clifton|5|3x");
    check("number with trailing junk is rejected", has(r, "Not a number: 3x"), r);
    r = h.handle("ADD_ROAD|Saddar|Keamari|-4|10");
    check("negative distance gives an error", isErr(r), r);
    r = h.handle("a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|1|2|3|4|5|6|7");
    check("far too many parts is rejected", isErr(r), r);

    // ---------------- routes ----------------
    r = h.handle("ROUTE_DIST|Saddar|Clifton");
    check("ROUTE_DIST Saddar -> Clifton", isOk(r) && has(r, "\"path\":[\"Saddar\",\"Clifton\"]") && has(r, "\"totalDist\":6"), r);
    r = h.handle("  route_dist | saddar | CLIFTON ");
    check("names and command ignore case and spaces", isOk(r) && has(r, "\"Clifton\""), r);
    r = h.handle("ROUTE_FEWEST|Keamari|Clifton");
    check("ROUTE_FEWEST works", isOk(r) && has(r, "\"hops\":2"), r);

    const char* names[15] = {"Keamari", "Lyari", "Saddar", "Clifton", "DHA", "Korangi", "FAST-NUCES", "Malir",
                             "Jinnah Airport", "Gulistan-e-Johar", "Gulshan-e-Iqbal", "Karachi University",
                             "North Nazimabad", "Nazimabad", "Liaquatabad"};
    bool differs = false;
    string sample;
    for (int i = 0; i < 15 && !differs; i++) {
        for (int j = i + 1; j < 15 && !differs; j++) {
            string d = h.handle(string("ROUTE_DIST|") + names[i] + "|" + names[j]);
            string t = h.handle(string("ROUTE_TIME|") + names[i] + "|" + names[j]);
            if (isOk(d) && isOk(t) && d.substr(0, d.find("\"totalDist\"")) != t.substr(0, t.find("\"totalDist\""))) {
                differs = true;
                sample = string(names[i]) + " -> " + names[j];
            }
        }
    }
    check("ROUTE_TIME differs from ROUTE_DIST for at least one pair (" + sample + ")", differs);

    r = h.handle("ROUTE_DIST|Keamari|Malir");
    check("route reply is one single line", oneLine(r) && isOk(r), r);
    r = h.handle("ROUTE_DIST|Saddar|Saddar");
    check("same source and destination is handled (Part A answers with a one-location path)", isOk(r) || isErr(r), r);

    // ---------------- multi-stop and alternates ----------------
    r = h.handle("MULTI|DIST|Saddar|Clifton|DHA|Korangi");
    check("MULTI with 3 stops shows order and path", isOk(r) && has(r, "\"order\":[") && has(r, "\"path\":["), r);
    r = h.handle("MULTI|TIME|Saddar|Clifton|DHA");
    check("MULTI by time works", isOk(r), r);
    r = h.handle("MULTI|FAST|Saddar|Clifton|DHA");
    check("MULTI with a bad mode", has(r, "Mode must be DIST or TIME"), r);
    r = h.handle("MULTI|DIST|Saddar|Clifton");
    check("MULTI with only 1 stop is a wrong-argument error", has(r, "Wrong number of arguments for MULTI"), r);
    r = h.handle("MULTI|DIST|Saddar|Clifton|Clifton");
    check("MULTI duplicate stop is reported", has(r, "Duplicate stop"), r);
    r = h.handle("MULTI|DIST|Saddar|Clifton|Nowhere");
    check("MULTI unknown stop is named", has(r, "Unknown location: Nowhere"), r);
    r = h.handle("ALTERNATES|Saddar|Clifton|DIST");
    check("ALTERNATES returns routes", isOk(r) && has(r, "\"routes\":["), r);
    r = h.handle("ALTERNATES|Saddar|Clifton|hops");
    check("ALTERNATES sort key is case-insensitive", isOk(r), r);
    r = h.handle("ALTERNATES|Saddar|Clifton|SPEED");
    check("ALTERNATES bad sort key", has(r, "Sort key must be DIST, TIME or HOPS"), r);

    // ---------------- block / unblock ----------------
    r = h.handle("BLOCK|Saddar|Clifton");
    check("BLOCK a road", isOk(r) && has(r, "blocked"), r);
    r = h.handle("ROUTE_DIST|Saddar|Clifton");
    check("route avoids the blocked road", isOk(r) && !has(r, "\"path\":[\"Saddar\",\"Clifton\"]"), r);
    r = h.handle("UNBLOCK|Saddar|Clifton");
    check("UNBLOCK a road", isOk(r), r);
    r = h.handle("ROUTE_DIST|Saddar|Clifton");
    check("old route is back after UNBLOCK", has(r, "\"path\":[\"Saddar\",\"Clifton\"]"), r);
    r = h.handle("BLOCK|Saddar|Hawks Bay");
    check("BLOCK with an unknown location", has(r, "Unknown location: Hawks Bay"), r);

    // ---------------- history ----------------
    h.handle("ROUTE_DIST|Saddar|Nowhere");           // failed: must NOT be recorded
    r = h.handle("HISTORY");
    check("HISTORY has the route searches, newest first",
          isOk(r) && has(r, "Shortest by distance: Saddar to Clifton") && has(r, "Fewest roads: Keamari to Clifton") &&
          r.find("Shortest by distance: Saddar to Clifton") < r.find("Fewest roads: Keamari to Clifton"), r);
    check("failed searches are not in the history", !has(r, "Nowhere"), r);
    check("MULTI and ALTERNATES are in the history", has(r, "Multi-stop by distance") && has(r, "Alternate routes by distance"), r);

    // ---------------- editing, undo ----------------
    r = h.handle("ADD_LOCATION|Test Town|100|100");
    check("ADD_LOCATION", isOk(r), r);
    r = h.handle("ADD_ROAD|Test Town|Saddar|3.5|9");
    check("ADD_ROAD to the new location", isOk(r), r);
    r = h.handle("ROUTE_DIST|Saddar|Test Town");
    check("route to the new location", isOk(r) && has(r, "\"totalDist\":3.5"), r);
    r = h.handle("UPDATE_ROAD|Test Town|Saddar|4|10");
    check("UPDATE_ROAD", isOk(r), r);
    r = h.handle("UPDATE_LOCATION|Test Town|Test City|120|130");
    check("UPDATE_LOCATION renames", isOk(r), r);
    r = h.handle("FIND|test city");
    check("FIND works for any capitalisation", has(r, "\"found\":true") && has(r, "Test City"), r);
    r = h.handle("SEARCH|test");
    check("SEARCH by prefix", isOk(r) && has(r, "Test City"), r);
    r = h.handle("REMOVE_ROAD|Test City|Saddar");
    check("REMOVE_ROAD", isOk(r), r);
    r = h.handle("REMOVE_LOCATION|Test City");
    check("REMOVE_LOCATION", isOk(r), r);
    r = h.handle("ROUTE_DIST|Saddar|Test City");
    check("routes cannot reach a removed location", has(r, "Unknown location: Test City"), r);

    for (int i = 0; i < 6; i++) h.handle("UNDO");    // add location, add road, 2 updates, remove road, remove location
    r = h.handle("FIND|Test Town");
    check("after undoing all edits the new location is gone", has(r, "\"found\":false"), r);
    r = h.handle("UNDO");
    check("UNDO with nothing to undo gives an error", isErr(r) && has(r, "Nothing to undo"), r);

    // ---------------- lists, analysis ----------------
    r = h.handle("LIST_SORTED");
    check("LIST_SORTED is alphabetical", isOk(r) && r.find("\"Clifton\"") < r.find("\"Saddar\""), r);
    r = h.handle("CONNECTIVITY");
    check("CONNECTIVITY says connected on the sample map", has(r, "\"connected\":true"), r);
    r = h.handle("MST");
    check("MST returns edges and total", isOk(r) && has(r, "\"totalDist\":") && has(r, "\"edges\":["), r);
    r = h.handle("LOAD");
    check("LOAD reloads the map file", isOk(r) && has(r, "Map loaded"), r);

    // ---------------- 20 commands quickly ----------------
    bool allOne = true;
    for (int i = 0; i < 20; i++) {
        string reply = h.handle(i % 2 == 0 ? "ROUTE_TIME|Keamari|Malir" : "GET_MAP");
        if (!oneLine(reply) || !isOk(reply)) allOne = false;
    }
    check("20 commands in a row: one OK line each", allOne);

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    if (failed == 0) {
        return 0;
    }
    return 1;
}
