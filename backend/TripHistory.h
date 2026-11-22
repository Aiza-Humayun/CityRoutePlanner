#pragma once
#include <string>
#include <fstream>
#include "Json.h"
#include "LinkedList.h"

class TripHistory {
private:
    DoublyLinkedList<std::string> entries;   
    static constexpr int MAX_ENTRIES = 50;   // keep only the 50 most recent trips

public:
    void add(const std::string& text) {
        std::string clean = text;
        for (size_t i = 0; i < clean.size(); i++) {
            if (clean[i] == '\n' || clean[i] == '\r') clean[i] = ' ';
        }
        entries.pushBack(clean);
        while (entries.size() > MAX_ENTRIES) entries.popFront();
    }

    int size() const { return entries.size(); }

    std::string toJson() const {
        std::string out = "{\"ok\":true,\"history\":[";
        bool first = true;
        // Start at the tail (newest) and walk backwards using 'prev'.
        for (DNode<std::string>* n = entries.getTail(); n != nullptr; n = n->prev) {
            if (!first) out += ",";
            first = false;
            out += jsonQuote(n->data);
        }
        out += "]}";
        return out;
    }

    // Returns true if the file was written.
    bool saveToFile(const std::string& path) const {
        std::ofstream file(path.c_str());
        if (!file.is_open()) return false;
        for (DNode<std::string>* n = entries.getHead(); n != nullptr; n = n->next) {
            file << n->data << "\n";
        }
        return !file.fail();
    }

    bool loadFromFile(const std::string& path) {
        std::ifstream file(path.c_str());
        if (!file.is_open()) return false;
        entries.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) add(line);
        }
        return true;
    }
};
