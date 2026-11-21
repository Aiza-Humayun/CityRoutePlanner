/*
 * =============================================================================
 * File    : TripHistory.h
 * Part    : B (Map Data and Analysis)
 * Purpose : Remembers the most recent route searches (e.g. "Shortest by distance:
 *           Saddar to Clifton"). Part C's command handler calls add() after each
 *           successful route command, and HISTORY shows the list.
 * Course  : Lab 4 - Doubly linked list (we add at the back, drop from the front,
 *           and read from the back so the newest entry comes first).
 *
 * WHY A DOUBLY LINKED LIST?
 *   - add new entry at the BACK                : O(1) with a tail pointer
 *   - drop the oldest entry from the FRONT     : O(1)
 *   - show newest first by walking BACKWARDS   : possible because of 'prev' pointers
 * =============================================================================
 */
#pragma once
#include <string>
#include <fstream>
#include "Json.h"
#include "LinkedList.h"

class TripHistory {
private:
    DoublyLinkedList<std::string> entries;   // oldest at the head, newest at the tail
    static constexpr int MAX_ENTRIES = 50;   // keep only the 50 most recent trips

public:
    // add: stores one history line. If there are more than 50, the oldest are dropped.
    void add(const std::string& text) {
        // A line break inside an entry would break the history file, so replace it.
        std::string clean = text;
        for (size_t i = 0; i < clean.size(); i++) {
            if (clean[i] == '\n' || clean[i] == '\r') clean[i] = ' ';
        }
        entries.pushBack(clean);
        while (entries.size() > MAX_ENTRIES) entries.popFront();
    }

    int size() const { return entries.size(); }

    // toJson: {"ok":true,"history":["newest ...","older ...", ...]}
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

    // ------------------------------------------------------------------ BONUS
    // Optional: save/load the history to a text file (one entry per line, oldest
    // first). NOT used unless Part C calls them (for example loadFromFile in the
    // CommandHandler constructor and saveToFile after each add).

    // Returns true if the file was written.
    bool saveToFile(const std::string& path) const {
        std::ofstream file(path.c_str());
        if (!file.is_open()) return false;
        for (DNode<std::string>* n = entries.getHead(); n != nullptr; n = n->next) {
            file << n->data << "\n";
        }
        return !file.fail();
    }

    // Returns true if the file was read. A missing file is not an error for the
    // program, so the caller can ignore a false result at first start.
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
