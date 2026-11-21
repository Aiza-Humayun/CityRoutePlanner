/*
 * =============================================================================
 * File    : main.cpp
 * Part    : C (Foundations, Interface, Web)
 * Purpose : Entry point of the C++ engine. It reads one command per line from
 *           the keyboard (or from the Python bridge's pipe) and prints exactly
 *           ONE line of JSON per command.
 *
 * PROTOCOL (what the Python bridge relies on)
 *   1. When startup is finished, print   {"ok":true,"message":"READY"}
 *   2. Loop: read a line; if it is EXIT stop; otherwise print the reply and a
 *      newline, and FLUSH so the bridge gets it immediately.
 *   3. Blank lines are ignored (no reply). The bridge must never send one,
 *      or it would wait for an answer that never comes.
 *
 * RULES
 *   - This is the ONLY file that writes to std::cout. Warnings go to std::cerr.
 *   - Run it from the project root so the path data/map.txt works.
 *
 * Build : g++ -std=c++17 -Wall -o backend/planner.exe backend/main.cpp
 * Run   : backend\planner.exe     (then type commands by hand, e.g. GET_MAP)
 * =============================================================================
 */
#include <iostream>
#include <string>
#include "CommandHandler.h"

int main() {
    CommandHandler handler;                       // builds everything and loads data/map.txt

    std::cout << jsonMessage("READY") << "\n" << std::flush;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (CommandHandler::isExit(line)) break;  // EXIT ends the program

        std::string reply = handler.handle(line);
        if (reply.empty()) continue;              // blank line: ignored

        std::cout << reply << "\n" << std::flush;
    }
    return 0;
}
