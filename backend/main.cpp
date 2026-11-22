#include <iostream>
#include <string>
#include "CommandHandler.h"

int main() {
    CommandHandler handler;                      

    std::cout << jsonMessage("READY") << "\n" << std::flush;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (CommandHandler::isExit(line)) break;  

        std::string reply = handler.handle(line);
        if (reply.empty()) continue;            

        std::cout << reply << "\n" << std::flush;
    }
    return 0;
}
