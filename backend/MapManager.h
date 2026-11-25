
#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include "Graph.h"
#include "Json.h"
#include "HashTable.h"
#include "AVLTree.h"
#include "Stack.h"

class MapManager {
private:
    Graph&             graph;       
    HashTable          index;       



    AVLTree            sorted;
    Stack<std::string> undoStack;   

    static constexpr int    MAX_NAME_LENGTH = 30;
    static constexpr double MAX_COORDINATE  = 5000;     // x and y must be 0..5000
    static constexpr double MAX_ROAD_VALUE  = 100000;   // distance/time upper limit

    
    static std::string trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) a++;
        while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) b--;
        return s.substr(a, b - a);
    }

    // Lowercase copy (A-Z only), used to compare names ignoring case.
    static std::string lowerOf(const std::string& s) {
        std::string out = s;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] >= 'A' && out[i] <= 'Z') out[i] = (char)(out[i] + ('a' - 'A'));
        }
        return out;
    }

   
    static bool validateName(const std::string& name, std::string& err) {
        if (name.empty()) { err = "Location name cannot be empty"; return false; }
        if ((int)name.size() > MAX_NAME_LENGTH) {
            err = "Location name must be at most 30 characters";
            return false;
        }
        for (size_t i = 0; i < name.size(); i++) {
            char ch = name[i];
            if (ch == '|' || ch == ',') {
                err = "Location name cannot contain | or ,";
                return false;
            }
            if ((unsigned char)ch < 32) {            // control characters (line breaks, tabs...)
                err = "Location name contains an invalid character";
                return false;
            }
        }
        return true;
    }

   
    static bool parseDouble(const std::string& text, double& out) {
        std::istringstream in(text);
        if (!(in >> out)) return false;
        in >> std::ws;                               // skip trailing spaces
        return in.eof();                             // anything left over = not a pure number
    }

    // Same for a whole number ("1.5" is rejected).
    static bool parseInt(const std::string& text, int& out) {
        std::istringstream in(text);
        if (!(in >> out)) return false;
        in >> std::ws;
        return in.eof();
    }

    
    static int splitCommas(const std::string& line, std::string* fields, int maxFields) {
        int count = 0;
        size_t start = 0;
        while (true) {
            size_t comma = line.find(',', start);
            std::string piece = (comma == std::string::npos) ? line.substr(start)
                                                             : line.substr(start, comma - start);
            if (count < maxFields) fields[count] = piece;
            count++;
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
        return count;
    }

    // Validation rules shared by add/update road: positive and not absurdly large.
    // Written as !(x > 0) so that "not a number" values are rejected too.
    static bool validRoadValue(double v) { return v > 0 && v <= MAX_ROAD_VALUE; }
    static bool validCoordinate(double v) { return v >= 0 && v <= MAX_COORDINATE; }

   
     
    EdgeNode* nextRoadFrom(int u, int after) const {
        EdgeNode* best = nullptr;
        for (EdgeNode* e = graph.getEdges(u); e != nullptr; e = e->next) {
            if (e->to > after && (best == nullptr || e->to < best->to)) best = e;
        }
        return best;
    }

    // ====================================================== counting helpers
    int countRoads() const {
        int roads = 0;
        for (int u = 0; u < graph.nodeCount(); u++) {
            if (!graph.isActive(u)) continue;
            for (EdgeNode* e = graph.getEdges(u); e != nullptr; e = e->next) {
                if (u < e->to) roads++;              // each two-way road is stored twice; count once
            }
        }
        return roads;
    }

    
    std::string serialize() const {
        int n = graph.nodeCount();
        int* newId = new int[n > 0 ? n : 1];         // oldId -> sequential id
        int next = 0;
        for (int i = 0; i < n; i++) newId[i] = graph.isActive(i) ? next++ : -1;

        std::ostringstream out;
        out.precision(10);                           // plenty for km/minutes/pixels
        out << "# City Route Planner map file\n";
        out << "LOCATIONS\n";
        for (int i = 0; i < n; i++) {
            if (!graph.isActive(i)) continue;
            out << newId[i] << "," << graph.getName(i) << ","
                << graph.getX(i) << "," << graph.getY(i) << "\n";
        }
        out << "ROADS\n";
        for (int u = 0; u < n; u++) {
            if (!graph.isActive(u)) continue;
            // Roads of u that go to a HIGHER id, in increasing order: each road once.
            for (EdgeNode* e = nextRoadFrom(u, u); e != nullptr; e = nextRoadFrom(u, e->to)) {
                out << newId[u] << "," << newId[e->to] << ","
                    << e->dist << "," << e->time << ","
                    << (e->blocked ? 1 : 0) << "\n";
            }
        }
        out << "END\n";
        delete[] newId;
        return out.str();
    }

   
    std::string parseInto(const std::string& text) {
        graph.clear();
        index.clear();
        sorted.clear();

        std::istringstream in(text);
        std::string line;
        int lineNo = 0;
        int section = 0;                 // 0 = before any heading, 1 = LOCATIONS, 2 = ROADS, 3 = END seen
        std::string fields[6];

        while (std::getline(in, line)) {
            lineNo++;
            std::string prefix = "Line " + std::to_string(lineNo) + ": ";
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;          // blank line or comment

            if (line == "LOCATIONS") { section = 1; continue; }
            if (line == "ROADS")     { section = 2; continue; }
            if (line == "END")       { section = 3; break; }       // ignore anything after END

            if (section == 1) {
                // ---- location line: id,name,x,y
                int nf = splitCommas(line, fields, 6);
                if (nf != 4) return prefix + "a location needs 4 values: id,name,x,y";

                int id;
                if (!parseInt(trim(fields[0]), id)) return prefix + "location id must be a whole number";
                if (id != graph.nodeCount()) {
                    return prefix + "location ids must be 0,1,2,... in order (expected "
                           + std::to_string(graph.nodeCount()) + ")";
                }

                std::string name = trim(fields[1]);
                std::string err;
                if (!validateName(name, err)) return prefix + err;
                if (index.find(name) != -1) return prefix + "duplicate location name: " + name;

                double x, y;
                if (!parseDouble(trim(fields[2]), x) || !parseDouble(trim(fields[3]), y)) {
                    return prefix + "x and y must be numbers";
                }
                if (!validCoordinate(x) || !validCoordinate(y)) {
                    return prefix + "x and y must be between 0 and 5000";
                }

                int newId = graph.addNode(name, x, y);
                index.insert(name, newId);
                sorted.insert(name, newId);

            } else if (section == 2) {
                // ---- road line: id1,id2,distKm,timeMin,blocked
                int nf = splitCommas(line, fields, 6);
                if (nf != 5) return prefix + "a road needs 5 values: id1,id2,distance,time,blocked";

                int u, v;
                if (!parseInt(trim(fields[0]), u) || !parseInt(trim(fields[1]), v)) {
                    return prefix + "road location ids must be whole numbers";
                }
                if (!graph.isActive(u) || !graph.isActive(v)) {
                    return prefix + "road refers to a location id that does not exist";
                }
                if (u == v) return prefix + "a road cannot connect a location to itself";

                double dist, time;
                if (!parseDouble(trim(fields[2]), dist) || !parseDouble(trim(fields[3]), time)) {
                    return prefix + "distance and time must be numbers";
                }
                if (!validRoadValue(dist) || !validRoadValue(time)) {
                    return prefix + "distance and time must be positive numbers";
                }

                int blocked;
                if (!parseInt(trim(fields[4]), blocked) || (blocked != 0 && blocked != 1)) {
                    return prefix + "blocked must be 0 or 1";
                }

                if (!graph.addEdge(u, v, dist, time)) {
                    return prefix + "road could not be added (is it a duplicate?)";
                }
                if (blocked == 1) graph.setBlocked(u, v, true);

            } else {
                return prefix + "data found before the LOCATIONS heading";
            }
        }

        if (section != 3) return "Map file is missing the END line";
        return "";
    }

    /*
     * restoreFrom: loads map text SAFELY.
     * Before touching anything it keeps a backup of the current map. If the new text
     * turns out to be broken, it puts the backup back, so a bad file can never
     * leave the program with a half-loaded map.
     * Returns "" on success or the error message from parseInto().
     */
    std::string restoreFrom(const std::string& text) {
        std::string backup = serialize();
        std::string err = parseInto(text);
        if (!err.empty()) parseInto(backup);         // backup is always valid text
        return err;
    }

    // Pushes the current map onto the undo stack. Call this AFTER validation and
    // BEFORE applying the change.
    void pushSnapshot() { undoStack.push(serialize()); }

public:
    // The graph is created by the CommandHandler and passed in by reference.
    explicit MapManager(Graph& g) : graph(g) {}

    // A MapManager is never copied (it refers to one specific graph).
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;

    
    int findId(const std::string& name) {
        return index.find(trim(name));
    }

    
    std::string findLocation(const std::string& name) {
        int id = findId(name);
        if (id == -1) return "{\"ok\":true,\"found\":false}";
        return "{\"ok\":true,\"found\":true,\"name\":" + jsonQuote(graph.getName(id)) +
               ",\"x\":" + numToStr(graph.getX(id)) +
               ",\"y\":" + numToStr(graph.getY(id)) + "}";
    }

    // LIST_SORTED command: all names in alphabetical order (from the AVL tree).
    std::string listSorted() {
        return namesToJson(0, "");
    }

    // SEARCH command: names starting with 'prefix' (from the AVL tree).
    std::string searchPrefix(const std::string& prefix) {
        return namesToJson(1, trim(prefix));
    }

    // GET_MAP command: every location and every road, for drawing the map.
    std::string getMapJson() {
        std::string out = "{\"ok\":true,\"locations\":[";
        bool first = true;
        for (int id = 0; id < graph.nodeCount(); id++) {
            if (!graph.isActive(id)) continue;
            if (!first) out += ",";
            first = false;
            out += "{\"name\":" + jsonQuote(graph.getName(id)) +
                   ",\"x\":" + numToStr(graph.getX(id)) +
                   ",\"y\":" + numToStr(graph.getY(id)) + "}";
        }
        out += "],\"roads\":[";
        first = true;
        for (int u = 0; u < graph.nodeCount(); u++) {
            if (!graph.isActive(u)) continue;
            // Each two-way road appears once (u < other end), in a stable order.
            for (EdgeNode* e = nextRoadFrom(u, u); e != nullptr; e = nextRoadFrom(u, e->to)) {
                if (!first) out += ",";
                first = false;
                out += "{\"a\":" + jsonQuote(graph.getName(u)) +
                       ",\"b\":" + jsonQuote(graph.getName(e->to)) +
                       ",\"dist\":" + numToStr(e->dist) +
                       ",\"time\":" + numToStr(e->time) +
                       ",\"blocked\":" + (e->blocked ? "true" : "false") + "}";
            }
        }
        out += "]}";
        return out;
    }

private:
    // Shared by listSorted() and searchPrefix(): mode 0 = everything, mode 1 = prefix.
    std::string namesToJson(int mode, const std::string& prefix) {
        int capacity = sorted.size();
        std::string* names = new std::string[capacity > 0 ? capacity : 1];
        int* ids = new int[capacity > 0 ? capacity : 1];
        int found = (mode == 0) ? sorted.toArray(names, ids)
                                : sorted.prefixSearch(prefix, names, ids);
        std::string out = "{\"ok\":true,\"names\":[";
        for (int i = 0; i < found; i++) {
            if (i > 0) out += ",";
            out += jsonQuote(names[i]);
        }
        out += "]}";
        delete[] names;
        delete[] ids;
        return out;
    }

public:
    // ============================================================ edit locations
    // ADD_LOCATION: adds a new location.
    std::string addLocation(const std::string& rawName, double x, double y) {
        // ---- 1. VALIDATE (change nothing yet)
        std::string name = trim(rawName);
        std::string err;
        if (!validateName(name, err)) return jsonError(err);
        if (index.find(name) != -1) return jsonError("A location named " + name + " already exists");
        if (!validCoordinate(x) || !validCoordinate(y)) {
            return jsonError("x and y must be between 0 and 5000");
        }
        // ---- 2. SNAPSHOT for undo
        pushSnapshot();
        // ---- 3. APPLY: graph, hash table and AVL tree all get the new location
        int id = graph.addNode(name, x, y);
        index.insert(name, id);
        sorted.insert(name, id);
        return jsonMessage("Location added: " + name);
    }

    // UPDATE_LOCATION: rename and/or move an existing location.
    std::string updateLocation(int id, const std::string& rawNewName, double x, double y) {
        if (!graph.isActive(id)) return jsonError("Unknown location");
        std::string newName = trim(rawNewName);
        std::string err;
        if (!validateName(newName, err)) return jsonError(err);
        std::string oldName = graph.getName(id);

        // Another location may not already use the new name. Changing only the
        // capitalisation of its own name ("saddar" -> "Saddar") is allowed.
        int owner = index.find(newName);
        if (owner != -1 && owner != id) {
            return jsonError("A location named " + newName + " already exists");
        }
        if (!validCoordinate(x) || !validCoordinate(y)) {
            return jsonError("x and y must be between 0 and 5000");
        }

        pushSnapshot();
        // Remove the OLD name from both indexes, change the graph, add the NEW name.
        index.remove(oldName);
        sorted.remove(oldName);
        graph.setName(id, newName);
        graph.setPosition(id, x, y);
        index.insert(newName, id);
        sorted.insert(newName, id);
        return jsonMessage("Location updated: " + newName);
    }

    // REMOVE_LOCATION: deletes a location and every road touching it.
    std::string removeLocation(int id) {
        if (!graph.isActive(id)) return jsonError("Unknown location");
        std::string name = graph.getName(id);

        // Count its roads only to give a helpful message.
        int roads = 0;
        for (EdgeNode* e = graph.getEdges(id); e != nullptr; e = e->next) roads++;

        pushSnapshot();
        index.remove(name);
        sorted.remove(name);
        graph.removeNode(id);                        // also removes its roads
        return jsonMessage("Location removed: " + name + " (and " +
                           std::to_string(roads) + " connected roads)");
    }

    // ================================================================ edit roads
    // ADD_ROAD: adds a two-way road between two locations.
    std::string addRoad(int u, int v, double dist, double time) {
        if (!graph.isActive(u) || !graph.isActive(v)) return jsonError("Unknown location");
        if (u == v) return jsonError("A road must connect two different locations");
        if (!validRoadValue(dist) || !validRoadValue(time)) {
            return jsonError("Distance and time must be positive numbers");
        }
        if (graph.hasEdge(u, v)) {
            return jsonError("A road between " + graph.getName(u) + " and " +
                             graph.getName(v) + " already exists");
        }
        pushSnapshot();
        graph.addEdge(u, v, dist, time);
        return jsonMessage("Road added: " + graph.getName(u) + " - " + graph.getName(v));
    }

    // UPDATE_ROAD: changes the distance and time of an existing road.
    std::string updateRoad(int u, int v, double dist, double time) {
        if (!graph.isActive(u) || !graph.isActive(v)) return jsonError("Unknown location");
        if (!graph.hasEdge(u, v)) {
            return jsonError("There is no road between " + graph.getName(u) + " and " + graph.getName(v));
        }
        if (!validRoadValue(dist) || !validRoadValue(time)) {
            return jsonError("Distance and time must be positive numbers");
        }
        pushSnapshot();
        graph.updateEdge(u, v, dist, time);
        return jsonMessage("Road updated: " + graph.getName(u) + " - " + graph.getName(v));
    }

    // REMOVE_ROAD: deletes a road (the locations stay).
    std::string removeRoad(int u, int v) {
        if (!graph.isActive(u) || !graph.isActive(v)) return jsonError("Unknown location");
        if (!graph.hasEdge(u, v)) {
            return jsonError("There is no road between " + graph.getName(u) + " and " + graph.getName(v));
        }
        pushSnapshot();
        graph.removeEdge(u, v);
        return jsonMessage("Road removed: " + graph.getName(u) + " - " + graph.getName(v));
    }

    // ====================================================================== undo
    // UNDO: restores the map to how it was just before the last edit.
    std::string undo() {
        if (undoStack.isEmpty()) return jsonError("Nothing to undo");
        std::string snapshot = undoStack.pop();      // newest snapshot (LIFO)
        std::string err = restoreFrom(snapshot);
        if (!err.empty()) return jsonError("Undo failed: " + err);   // should never happen
        return jsonMessage("Last edit undone");
    }

    // ================================================================ save / load
    // SAVE: writes the map to a file.
    std::string saveToFile(const std::string& path) {
        std::ofstream file(path.c_str());
        if (!file.is_open()) return jsonError("Cannot write map file: " + path);
        file << serialize();
        file.flush();
        if (file.fail()) return jsonError("Error while writing map file: " + path);
        return jsonMessage("Map saved");
    }

    // LOAD (and the automatic load at program start): reads the map from a file.
    // If the file is missing or broken, the CURRENT map is kept and an error is returned.
    std::string loadFromFile(const std::string& path) {
        std::ifstream file(path.c_str());
        if (!file.is_open()) return jsonError("Cannot open map file: " + path);
        std::stringstream buffer;
        buffer << file.rdbuf();                      // read the whole file into one string

        std::string err = restoreFrom(buffer.str());
        if (!err.empty()) return jsonError(err);

        undoStack.clear();                           // old snapshots belong to the old map
        return jsonMessage("Map loaded: " + std::to_string(graph.activeCount()) +
                           " locations, " + std::to_string(countRoads()) + " roads");
    }
};
