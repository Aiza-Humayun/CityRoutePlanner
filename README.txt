CITY ROUTE PLANNER
Data Structures (CL-2001), BSE-3B, NUCES Karachi

WHAT IT IS
A city map stored as a graph. You can find the shortest, fastest or fewest-roads
route, plan trips with several stops, see alternative routes, block roads, edit the
map, undo edits, search locations and see a history of your searches.

It has three layers:
  1. C++ engine   (backend/)  - holds the map and runs ALL algorithms. Our own data
                                structures, no STL containers.
  2. Python bridge (bridge/)  - starts the engine once and passes requests to it.
  3. Web page      (web/)     - draws the map and shows the results. It never
                                calculates anything itself.
The engine also works alone in a console (see "Console only" below).

WHAT YOU NEED
  - g++ with C++17 support (for example MinGW-w64 on Windows)
  - Python 3 (standard library only, nothing to install with pip)
  - a web browser

QUICK START (Windows)
  1. Double-click  build.bat   (compiles the engine, only needed once or after code changes)
  2. Double-click  run.bat     (starts the bridge and opens http://localhost:8000)
  3. Stop the program with Ctrl+C in the black window.

QUICK START (by hand, any system) - always from the project root folder
(the folder that contains backend, data and web):
  g++ -std=c++17 -Wall -o backend/planner.exe backend/main.cpp
  python bridge/server.py
  then open http://localhost:8000 in your browser.

CONSOLE ONLY (no web page)
  backend\planner.exe
  The program prints {"ok":true,"message":"READY"}. Now type commands, one per line:
    GET_MAP
    ROUTE_DIST|Saddar|Clifton
    ROUTE_TIME|Saddar|Clifton
    ROUTE_FEWEST|Keamari|Clifton
    MULTI|DIST|Saddar|Clifton|DHA|Korangi
    ALTERNATES|Saddar|Clifton|TIME
    BLOCK|Saddar|Clifton        (and UNBLOCK|Saddar|Clifton)
    ADD_LOCATION|Test Town|100|100
    ADD_ROAD|Test Town|Saddar|3.5|9
    UNDO
    SEARCH|Sa       LIST_SORTED       FIND|saddar
    CONNECTIVITY    MST               HISTORY
    SAVE            LOAD              EXIT
  Every command gets exactly one line of JSON back.

TESTS (from the project root)
  g++ -std=c++17 -Wall -I backend -o tests/test_containers.exe tests/test_containers.cpp
  g++ -std=c++17 -Wall -I backend -o tests/test_route.exe      tests/test_route.cpp
  g++ -std=c++17 -Wall -I backend -o tests/test_mapdata.exe    tests/test_mapdata.cpp
  g++ -std=c++17 -Wall -I backend -o tests/test_commands.exe   tests/test_commands.cpp
  Then run each .exe. They print PASS or FAIL per check and a summary at the end.

FOLDERS
  backend/  all C++ code (main.cpp is the only file that is compiled; it includes the headers)
  bridge/   server.py
  web/      index.html, style.css, app.js
  data/     map.txt (the sample map of Karachi)
  tests/    test programs

TROUBLESHOOTING
  "backend/planner.exe not found"   Run build.bat first.
  "cannot use port 8000"            The server is already running, or another program
                                    uses port 8000. Close it, or start with another port:
                                    python bridge/server.py 9000
  Page says "Cannot reach the server"  server.py is not running. Start run.bat again.
  "Cannot open map file"            Start from the project root, so data/map.txt is found.

TEAM
  Ilham Fatima (25K-3024), Marium Jamshed (25K-3073), Aiza Humayun (24K-3116)
