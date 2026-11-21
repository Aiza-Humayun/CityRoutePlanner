# Shared contract (the agreement between the three parts)

If everyone follows this exactly, the parts connect on the first try.
Do not change anything here without telling the whole team.

## 1. Names and ids

- The **web page only uses location names**. Inside C++ every location has an integer **id** (0, 1, 2, ...).
- Part C's command handler converts names to ids using `MapManager::findId(name)` (hash table). It returns -1 if the name does not exist.
- Names: 1 to 30 characters, no `|` and no `,`. Names are **case-insensitive** (`saddar` = `Saddar`).
- Roads are **two-way**. Each road has a distance (km) and a travel time (minutes), both greater than 0.
- "Hops" = number of roads in a route (Saddar to Clifton directly = 1).

## 2. Map file format (`data/map.txt`)

```
# lines starting with # are comments
LOCATIONS
0,Saddar,330,430            <- id,name,x,y   (x,y = pixel position for drawing, 0..5000)
1,Clifton,410,525
ROADS
0,1,6,18,0                  <- id1,id2,distanceKm,timeMin,blocked(0 or 1)
END
```

Ids in the file are always 0, 1, 2, ... in order.

## 3. Commands

One command per line. Parts separated by `|`. The command name is in capitals.

| Command | Arguments | Written by |
|---------|-----------|-----------|
| `GET_MAP` | none | B |
| `FIND` | name | B |
| `SEARCH` | prefix | B |
| `LIST_SORTED` | none | B |
| `ADD_LOCATION` | name \| x \| y | B |
| `UPDATE_LOCATION` | oldName \| newName \| x \| y | B |
| `REMOVE_LOCATION` | name | B |
| `ADD_ROAD` | nameA \| nameB \| distKm \| timeMin | B |
| `UPDATE_ROAD` | nameA \| nameB \| distKm \| timeMin | B |
| `REMOVE_ROAD` | nameA \| nameB | B |
| `UNDO` | none | B |
| `SAVE` / `LOAD` | none (file is `data/map.txt`) | B |
| `CONNECTIVITY` | none | B |
| `MST` | none | B |
| `HISTORY` | none | B |
| `ROUTE_DIST` | source \| destination | A |
| `ROUTE_TIME` | source \| destination | A |
| `ROUTE_FEWEST` | source \| destination | A |
| `MULTI` | mode (DIST or TIME) \| source \| stop1 \| stop2 \| ... (2 to 6 stops) | A |
| `ALTERNATES` | source \| destination \| sortKey (DIST, TIME or HOPS) | A |
| `BLOCK` / `UNBLOCK` | nameA \| nameB | A |
| `EXIT` | none | C |

Example: `ADD_ROAD|Saddar|Clifton|6|18`

## 4. Replies (what the C++ program prints)

Every command gets **exactly one line** of JSON. Success has `"ok":true`. Failure has `"ok":false` and an `"error"` message.

```
Simple success : {"ok":true,"message":"Location added: Test Town"}
Any failure    : {"ok":false,"error":"Unknown location: Sadar"}
Route          : {"ok":true,"path":["Saddar","Clifton","DHA"],"totalDist":13,"totalTime":32,"hops":2}
Multi-stop     : same as Route plus  "order":["Clifton","DHA"]   (stops in visit order)
Alternates     : {"ok":true,"routes":[{"path":[...],"totalDist":..,"totalTime":..,"hops":..}, ...]}
Map            : {"ok":true,"locations":[{"name":"Saddar","x":330,"y":430}, ...],
                  "roads":[{"a":"Saddar","b":"Clifton","dist":6,"time":18,"blocked":false}, ...]}
Name list      : {"ok":true,"names":["Clifton","DHA","Saddar"]}
Find           : {"ok":true,"found":true,"name":"Saddar","x":330,"y":430}   (found:false if missing)
Connectivity   : {"ok":true,"connected":false,"components":[["A","B"],["C"]]}
MST            : {"ok":true,"connected":true,"totalDist":41.2,
                  "edges":[{"a":"Saddar","b":"Clifton","dist":6}, ...]}
History        : {"ok":true,"history":["Shortest by distance: Saddar to Clifton", ...]}
```

(Lines above are wrapped for reading. Real replies are one single line.)

Route replies always use **names**, never ids.

## 5. Graph interface (Part A writes `Graph.h`; A and B use it)

```cpp
struct EdgeNode {            // one road in a location's adjacency list
    int       to;            // id of the location at the other end
    double    dist;          // km
    double    time;          // minutes
    bool      blocked;       // true = road closed
    EdgeNode* next;
};

class Graph {
public:
    Graph();  ~Graph();                         // copying is disabled
    void        clear();
    int         addNode(const std::string& name, double x, double y);   // returns new id
    bool        removeNode(int id);             // marks inactive and removes its roads
    bool        isActive(int id) const;         // false for removed or invalid ids
    int         nodeCount() const;              // total ids ever used: loop 0..nodeCount()-1
    int         activeCount() const;            // locations that still exist
    std::string getName(int id) const;
    double      getX(int id) const;
    double      getY(int id) const;
    void        setName(int id, const std::string& name);
    void        setPosition(int id, double x, double y);
    bool        addEdge(int u, int v, double dist, double time);   // two-way road
    bool        removeEdge(int u, int v);
    bool        updateEdge(int u, int v, double dist, double time); // must keep the blocked flag
    bool        hasEdge(int u, int v) const;
    bool        setBlocked(int u, int v, bool blocked);
    EdgeNode*   getEdges(int u) const;          // head of u's road list (nullptr if none)
};
```

- **Ids are never reused.** A removed location keeps its id but becomes inactive. Every loop over locations must skip inactive ones: `if (!isActive(id)) continue;`
- `addEdge` returns false if: same location twice, an id is inactive, dist or time is 0 or less, or the road already exists. It stores the road in **both** directions with identical values. `removeEdge`, `updateEdge` and `setBlocked` must also change both directions.
- All route algorithms must ignore **blocked** roads.

## 6. Helper and container interfaces (Part C writes; everyone uses)

```cpp
// ---- Json.h
std::string jsonQuote(const std::string& s);      // "abc" with quotes; escapes " and \
std::string jsonError(const std::string& msg);    // {"ok":false,"error":"msg"}
std::string jsonMessage(const std::string& msg);  // {"ok":true,"message":"msg"}
std::string numToStr(double v);                   // 12.5 -> "12.5", 3.0 -> "3" (max 2 decimals)

// ---- LinkedList.h
template<typename T> struct ListNode { T data; ListNode* next; };
template<typename T> class SinglyLinkedList {
  public: SinglyLinkedList(); SinglyLinkedList(const SinglyLinkedList&);
          SinglyLinkedList& operator=(const SinglyLinkedList&); ~SinglyLinkedList();
          void pushFront(const T&);  void pushBack(const T&);  T popFront();
          bool isEmpty() const;  int size() const;  void clear();
          ListNode<T>* getHead() const;  T get(int index) const;  void reverse(); };
template<typename T> struct DNode { T data; DNode* prev; DNode* next; };
template<typename T> class DoublyLinkedList {      // Rule of Three members too, plus:
  public: void pushFront(const T&);  void pushBack(const T&);
          T popFront();  T popBack();  bool isEmpty() const;  int size() const;
          void clear();  DNode<T>* getHead() const;  DNode<T>* getTail() const; };

// ---- Stack.h and Queue.h (both with Rule of Three members)
template<typename T> class Stack { public: void push(const T&); T pop(); T& top();
          bool isEmpty() const; int size() const; void clear(); };
template<typename T> class Queue { public: void enqueue(const T&); T dequeue(); T& front();
          bool isEmpty() const; int size() const; void clear(); };
// pop/top/dequeue/front on an empty container must NOT crash: print to std::cerr and return T().

// ---- Sorting.h   (lessThan(a,b) is true when a must come before b)
template<typename T> void mergeSort(T arr[], int n, bool (*lessThan)(const T&, const T&));
template<typename T> void quickSort(T arr[], int n, bool (*lessThan)(const T&, const T&));
```

## 7. Functions each part must provide

Every function **returns a JSON string** (never prints).

### Part A - `RouteEngine.h`

```cpp
class RouteEngine {
public:
    RouteEngine(Graph& graph);
    std::string shortestByDistance(int src, int dst);                         // ROUTE_DIST
    std::string shortestByTime(int src, int dst);                             // ROUTE_TIME
    std::string fewestStops(int src, int dst);                                // ROUTE_FEWEST
    std::string multiStop(bool byTime, int src, int stops[], int stopCount);  // MULTI
    std::string alternateRoutes(int src, int dst, int sortKey);               // 0=DIST 1=TIME 2=HOPS
    std::string blockRoad(int u, int v);                                      // BLOCK
    std::string unblockRoad(int u, int v);                                    // UNBLOCK
};
```

### Part B - `MapManager.h`, `NetworkAnalysis.h`, `TripHistory.h`

```cpp
class MapManager {
public:
    explicit MapManager(Graph& graph);
    int         findId(const std::string& name);                      // -1 if not found
    std::string findLocation(const std::string& name);                // FIND
    std::string addLocation(const std::string& name, double x, double y);
    std::string updateLocation(int id, const std::string& newName, double x, double y);
    std::string removeLocation(int id);
    std::string addRoad(int u, int v, double dist, double time);
    std::string updateRoad(int u, int v, double dist, double time);
    std::string removeRoad(int u, int v);
    std::string listSorted();                                         // LIST_SORTED
    std::string searchPrefix(const std::string& prefix);              // SEARCH
    std::string getMapJson();                                         // GET_MAP
    std::string undo();                                               // UNDO
    std::string saveToFile(const std::string& path);                  // SAVE
    std::string loadFromFile(const std::string& path);                // LOAD, and at startup
};
class NetworkAnalysis {
public:
    explicit NetworkAnalysis(Graph& graph);
    std::string checkConnectivity();                                  // CONNECTIVITY
    std::string minimumSpanningTree();                                // MST
};
class TripHistory {
public:
    void        add(const std::string& text);                         // called by C after a route command
    std::string toJson() const;                                       // HISTORY
};
```

Rules for B's functions: coordinates must be between 0 and 5000, names follow section 1. After load or undo, ids are renumbered 0, 1, 2, ... (safe, because ids are never kept between commands).

### Part C - `CommandHandler.h` and `main.cpp`

```cpp
class CommandHandler {
public:
    CommandHandler();                             // creates all objects and loads data/map.txt
    std::string handle(const std::string& line);  // ONE command in -> ONE json line out
};
```

Private members must be declared **in this order** (the others need the graph first):
`Graph graph; MapManager map; RouteEngine engine; NetworkAnalysis net; TripHistory history;` and initialised with `map(graph), engine(graph), net(graph)`.

Steps inside `handle`: trim the line, split on `|`, check the command name and the number of arguments, convert names to ids with `map.findId` (error `Unknown location: X` if -1), convert numbers (error if not a number), call the owner's function, return its string. After a successful `ROUTE_DIST`, `ROUTE_TIME`, `ROUTE_FEWEST`, `MULTI` or `ALTERNATES`, call `history.add("Shortest by distance: Saddar to Clifton")` (a readable line).

`main.cpp`: create a `CommandHandler`, print `{"ok":true,"message":"READY"}` once, then loop: read a line with `getline`; if it is `EXIT` stop; otherwise print `handler.handle(line)` + newline + flush. **Nothing else may be printed to `cout`.**
