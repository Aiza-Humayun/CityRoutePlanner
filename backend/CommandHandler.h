#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include "Json.h"
#include "Graph.h"
#include "MapManager.h"
#include "RouteEngine.h"
#include "NetworkAnalysis.h"
#include "TripHistory.h"

class CommandHandler {
private:
    Graph           graph;
    MapManager      map;
    RouteEngine     engine;
    NetworkAnalysis net;
    TripHistory     history;

    static constexpr int MAX_PARTS = 32;      

    static const char* mapPath() { return "data/map.txt"; }

    // ------------------------------------------------------------ text helpers
    static std::string trim(const std::string& s) {
        size_t start = 0;
        size_t end = s.size();
        while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) start++;
        while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n')) end--;
        return s.substr(start, end - start);
    }

    static std::string toUpper(const std::string& s) {
        std::string out = s;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] >= 'a' && out[i] <= 'z') out[i] = (char)(out[i] - ('a' - 'A'));
        }
        return out;
    }

    static int splitLine(const std::string& line, std::string*& parts) {
        int count = 1;
        for (size_t i = 0; i < line.size(); i++) {
            if (line[i] == '|') count++;
        }
        if (count > MAX_PARTS) {
            parts = nullptr;
            return count;
        }
        parts = new std::string[count];
        int index = 0;
        std::string current;
        for (size_t i = 0; i < line.size(); i++) {
            if (line[i] == '|') {
                parts[index++] = trim(current);
                current.clear();
            } else {
                current += line[i];
            }
        }
        parts[index] = trim(current);
        return count;
    }

    static bool parseNumber(const std::string& text, double& out) {
        std::string t = trim(text);
        if (t.empty()) return false;
        std::istringstream in(t);
        double value = 0;
        in >> value;
        if (in.fail()) return false;
        in >> std::ws;
        if (!in.eof()) return false;          
        out = value;
        return true;
    }

    // ----------------------------------------------------------- reply helpers
    static std::string wrongArgs(const std::string& cmd) {
        return jsonError("Wrong number of arguments for " + cmd);
    }

    static std::string notANumber(const std::string& text) {
        return jsonError("Not a number: " + text);
    }

    // True if a reply is a success reply (starts with {"ok":true).
    static bool isSuccess(const std::string& reply) {
        return reply.rfind("{\"ok\":true", 0) == 0;
    }

    // Adds a history line, but only after a successful route search.
    void remember(const std::string& reply, const std::string& text) {
        if (isSuccess(reply)) history.add(text);
    }

    /*
     * lookup: name -> id through the hash table (case-insensitive).
     * On failure returns -1 and puts the ready-made JSON error in 'err'.
     */
    int lookup(const std::string& name, std::string& err) {
        int id = map.findId(name);
        if (id == -1) err = jsonError("Unknown location: " + trim(name));
        return id;
    }

    // ------------------------------------------------------------ the commands
    std::string dispatch(const std::string& cmd, std::string* a, int argc) {
        std::string err;

        // ---------------- commands without arguments ----------------
        if (cmd == "GET_MAP")       { if (argc != 0) return wrongArgs(cmd); return map.getMapJson(); }
        if (cmd == "LIST_SORTED")   { if (argc != 0) return wrongArgs(cmd); return map.listSorted(); }
        if (cmd == "UNDO")          { if (argc != 0) return wrongArgs(cmd); return map.undo(); }
        if (cmd == "SAVE")          { if (argc != 0) return wrongArgs(cmd); return map.saveToFile(mapPath()); }
        if (cmd == "LOAD")          { if (argc != 0) return wrongArgs(cmd); return map.loadFromFile(mapPath()); }
        if (cmd == "CONNECTIVITY")  { if (argc != 0) return wrongArgs(cmd); return net.checkConnectivity(); }
        if (cmd == "MST")           { if (argc != 0) return wrongArgs(cmd); return net.minimumSpanningTree(); }
        if (cmd == "HISTORY")       { if (argc != 0) return wrongArgs(cmd); return history.toJson(); }

        // ---------------- queries ----------------
        if (cmd == "FIND")   { if (argc != 1) return wrongArgs(cmd); return map.findLocation(a[0]); }
        if (cmd == "SEARCH") { if (argc != 1) return wrongArgs(cmd); return map.searchPrefix(a[0]); }

        // ---------------- location edits ----------------
        if (cmd == "ADD_LOCATION") {                      
            if (argc != 3) return wrongArgs(cmd);
            double x = 0, y = 0;
            if (!parseNumber(a[1], x)) return notANumber(a[1]);
            if (!parseNumber(a[2], y)) return notANumber(a[2]);
            return map.addLocation(a[0], x, y);            
        }
        if (cmd == "UPDATE_LOCATION") {                    
            if (argc != 4) return wrongArgs(cmd);
            int id = lookup(a[0], err);
            if (id == -1) return err;
            double x = 0, y = 0;
            if (!parseNumber(a[2], x)) return notANumber(a[2]);
            if (!parseNumber(a[3], y)) return notANumber(a[3]);
            return map.updateLocation(id, a[1], x, y);
        }
        if (cmd == "REMOVE_LOCATION") {                    // name
            if (argc != 1) return wrongArgs(cmd);
            int id = lookup(a[0], err);
            if (id == -1) return err;
            return map.removeLocation(id);
        }

        // ---------------- road edits ----------------
        if (cmd == "ADD_ROAD" || cmd == "UPDATE_ROAD") 
            if (argc != 4) return wrongArgs(cmd);
            int u = lookup(a[0], err);
            if (u == -1) return err;
            int v = lookup(a[1], err);
            if (v == -1) return err;
            double dist = 0, time = 0;
            if (!parseNumber(a[2], dist)) return notANumber(a[2]);
            if (!parseNumber(a[3], time)) return notANumber(a[3]);
            if (cmd == "ADD_ROAD") return map.addRoad(u, v, dist, time);
            return map.updateRoad(u, v, dist, time);
        }
        if (cmd == "REMOVE_ROAD") {                        
            if (argc != 2) return wrongArgs(cmd);
            int u = lookup(a[0], err);
            if (u == -1) return err;
            int v = lookup(a[1], err);
            if (v == -1) return err;
            return map.removeRoad(u, v);
        }

        // ---------------- road closures ----------------
        if (cmd == "BLOCK" || cmd == "UNBLOCK") {         
            if (argc != 2) return wrongArgs(cmd);
            int u = lookup(a[0], err);
            if (u == -1) return err;
            int v = lookup(a[1], err);
            if (v == -1) return err;
            return (cmd == "BLOCK") ? engine.blockRoad(u, v) : engine.unblockRoad(u, v);
        }

        // ---------------- single route searches ----------------
        if (cmd == "ROUTE_DIST" || cmd == "ROUTE_TIME" || cmd == "ROUTE_FEWEST") {
            if (argc != 2) return wrongArgs(cmd);          // source | destination
            int src = lookup(a[0], err);
            if (src == -1) return err;
            int dst = lookup(a[1], err);
            if (dst == -1) return err;
            std::string reply, label;
            if (cmd == "ROUTE_DIST")      { reply = engine.shortestByDistance(src, dst); label = "Shortest by distance"; }
            else if (cmd == "ROUTE_TIME") { reply = engine.shortestByTime(src, dst);     label = "Fastest by time"; }
            else                          { reply = engine.fewestStops(src, dst);        label = "Fewest roads"; }
            remember(reply, label + ": " + graph.getName(src) + " to " + graph.getName(dst));
            return reply;
        }

        // ---------------- multi-stop trip ----------------
        if (cmd == "MULTI") {                              // mode | source | stop1 | ... | stop6
            if (argc < 4 || argc > 8) return wrongArgs(cmd);   // mode + source + 2 to 6 stops
            std::string mode = toUpper(a[0]);
            if (mode != "DIST" && mode != "TIME") return jsonError("Mode must be DIST or TIME");
            int src = lookup(a[1], err);
            if (src == -1) return err;
            int stopCount = argc - 2;
            int stops[6];
            std::string stopNames;
            for (int i = 0; i < stopCount; i++) {
                stops[i] = lookup(a[2 + i], err);
                if (stops[i] == -1) return err;
                stopNames += (i == 0 ? "" : ", ") + graph.getName(stops[i]);
            }
            std::string reply = engine.multiStop(mode == "TIME", src, stops, stopCount);
            remember(reply, std::string("Multi-stop by ") + (mode == "TIME" ? "time" : "distance") +
                            ": " + graph.getName(src) + " then " + stopNames);
            return reply;
        }

        // ---------------- alternate routes ----------------
        if (cmd == "ALTERNATES") {                        
            if (argc != 3) return wrongArgs(cmd);
            int src = lookup(a[0], err);
            if (src == -1) return err;
            int dst = lookup(a[1], err);
            if (dst == -1) return err;
            std::string key = toUpper(a[2]);
            int sortKey = -1;
            std::string keyText;
            if (key == "DIST")      { sortKey = 0; keyText = "distance"; }
            else if (key == "TIME") { sortKey = 1; keyText = "time"; }
            else if (key == "HOPS") { sortKey = 2; keyText = "number of roads"; }
            else return jsonError("Sort key must be DIST, TIME or HOPS");
            std::string reply = engine.alternateRoutes(src, dst, sortKey);
            remember(reply, "Alternate routes by " + keyText + ": " + graph.getName(src) + " to " + graph.getName(dst));
            return reply;
        }

        return jsonError("Unknown command: " + cmd);
    }

public:
    CommandHandler() : graph(), map(graph), engine(graph), net(graph), history() {
        std::string reply = map.loadFromFile(mapPath());
        if (!isSuccess(reply)) std::cerr << "Warning: " << reply << "\n";
    }

    std::string handle(const std::string& rawLine) {
        std::string line = trim(rawLine);
        if (line.empty()) return "";

        std::string* parts = nullptr;
        int count = splitLine(line, parts);
        if (parts == nullptr) return jsonError("Too many arguments");

        std::string cmd = toUpper(parts[0]);
        std::string reply = dispatch(cmd, parts + 1, count - 1);
        delete[] parts;
        return reply;
    }

    static bool isExit(const std::string& line) {
        return toUpper(trim(line)) == "EXIT";
    }
};
