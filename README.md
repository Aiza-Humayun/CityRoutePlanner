<div align="center">

# City Route Planner

**Find the best way across the city, built on data structures we wrote ourselves.**

![C++11](https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus&logoColor=white)
![Python](https://img.shields.io/badge/Python-3-3776AB?logo=python&logoColor=white)
![Course](https://img.shields.io/badge/Course-Data%20Structures-2E8B57)

</div>

A C++ application that models a city as a network of locations and roads, and finds the best way to travel between two places. Every data structure is implemented from scratch (no STL containers). A web page sits on top of the C++ program for easy use and demonstration.

**Course:** CL-2001 Data Structures Lab, Fall 2026, FAST-NUCES Karachi (Section BSE-3B)

## Team

| Member | Roll No. | Role |
|--------|----------|------|
| Aiza Humayun | 24K-3116 | Part B: map data, search, undo, save/load, spanning tree |
| Marium Jamshed | 25K-3073 | Part A: graph, shortest paths, alternate routes |
| Ilham Fatima | 25K-3024 | Part C: containers, command interface, web page, integration |

## How it works

```
Web page  <-->  Python bridge  <-->  C++ engine
(browser)       (server.py)          (planner.exe)
```

The page sends one text command (for example `ROUTE_DIST|Saddar|Clifton`). The C++ engine runs the algorithm and prints one line of JSON. The engine also works on its own in a console, so you can type commands by hand.

## Features

- Shortest route by distance or by time
- Route with the fewest roads
- Multi-stop trips and alternate routes
- Block and unblock roads, then re-plan
- Add, update and remove locations and roads, with undo
- Search by name, alphabetical list, trip history
- Connectivity check and minimum spanning tree
- Map saved to and loaded from a file, fully offline

## Data structures used

| Data structure | Used for |
|----------------|----------|
| Graph (adjacency list) | Locations as nodes, roads as weighted edges |
| Min-heap | Picking the next closest node in Dijkstra |
| Queue | Breadth-first search (fewest roads) |
| Stack | Depth-first search, undo |
| Singly linked list | Storing the final route |
| Doubly linked list | Trip history |
| Hash table | Finding a location by name |
| AVL tree | Sorted list and prefix search |
| Disjoint set | Kruskal minimum spanning tree |
| Merge sort and quick sort | Ranking routes, sorting roads |

## Project structure

```
CityRoutePlanner/
  README.md        this file
  CONTRACT.md      agreement between the three parts
  backend/         C++ code (.h files and main.cpp)
  data/            map.txt (sample city map)
  bridge/          server.py (Python bridge)
  web/             index.html, style.css, app.js
  tests/           one test program per part
```

## Setup

Requires g++ (C++11 or newer) and Python 3. Run every command from the project root folder.

If you use Dev-C++, first run this once in the command window so g++ is found:

```
set PATH=C:\Program Files (x86)\Dev-Cpp\MinGW64\bin;%PATH%
```

Then build:

```
g++ -std=c++11 -Wall -o backend/planner.exe backend/main.cpp
```

## Run

Console (type commands by hand, for example `GET_MAP`):

```
backend\planner.exe
```

Web page:

```
python bridge/server.py
```

Then open http://localhost:8000 in a browser.

## Test

```
g++ -std=c++11 -Wall -I backend -o tests/test_mapdata.exe tests/test_mapdata.cpp
tests\test_mapdata.exe
```

## Contributing

Commit under your own GitHub account with clear messages that start with your part letter, for example `[B] add AVL tree`. Edit only your own files. Commit history is part of the contribution record.
