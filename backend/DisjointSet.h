/*
 * =============================================================================
 * File    : DisjointSet.h
 * Part    : B (Map Data and Analysis)
 * Purpose : "Union-Find". Keeps track of which locations are already connected
 *           to each other. Kruskal's algorithm (minimum spanning tree) uses it
 *           to ask "would this road create a loop?" in almost constant time.
 * Course  : Lab 11 - Minimum spanning trees (Kruskal needs a disjoint set).
 *           Lab 1  - Rule of Three.
 *
 * HOW IT WORKS (viva summary)
 *   - Every element starts in its own set. parent[i] points to the element's
 *     "parent"; a set is a tree and its root is the set's representative.
 *   - find(x)     : follow parents up to the root.
 *                   PATH COMPRESSION: afterwards, every element visited points
 *                   directly at the root, so later finds are faster.
 *   - unite(a, b) : join the two sets. UNION BY RANK: the shorter tree is
 *                   attached under the taller one, so trees stay shallow.
 *   - With both tricks, each operation is nearly O(1) (inverse Ackermann).
 * =============================================================================
 */
#pragma once

class DisjointSet {
private:
    int* parent;    // parent[i] = parent of i (a root is its own parent)
    int* rnk;       // rnk[i]    = upper bound of the height of the tree rooted at i
    int  n;         // number of elements (0 .. n-1)
    int  sets;      // current number of separate sets

    // Allocates and initialises: every element alone in its own set.
    void init(int size) {
        n = size;
        sets = size;
        parent = new int[n > 0 ? n : 1];     // (never allocate 0 elements)
        rnk    = new int[n > 0 ? n : 1];
        for (int i = 0; i < n; i++) {
            parent[i] = i;
            rnk[i] = 0;
        }
    }

    // Deep copy used by the copy constructor and copy assignment.
    void copyFrom(const DisjointSet& other) {
        n = other.n;
        sets = other.sets;
        parent = new int[n > 0 ? n : 1];
        rnk    = new int[n > 0 ? n : 1];
        for (int i = 0; i < n; i++) {
            parent[i] = other.parent[i];
            rnk[i] = other.rnk[i];
        }
    }

public:
    // Creates 'size' elements, numbered 0 .. size-1, each in its own set.
    explicit DisjointSet(int size = 0) { init(size); }

    // RULE OF THREE (1/3): copy constructor.
    DisjointSet(const DisjointSet& other) { copyFrom(other); }

    // RULE OF THREE (2/3): copy assignment.
    DisjointSet& operator=(const DisjointSet& other) {
        if (this != &other) {
            delete[] parent;
            delete[] rnk;
            copyFrom(other);
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor.
    ~DisjointSet() {
        delete[] parent;
        delete[] rnk;
    }

    // find: returns the root (representative) of x's set, or -1 if x is out of range.
    int find(int x) {
        if (x < 0 || x >= n) return -1;
        // Pass 1: walk up to the root.
        int root = x;
        while (parent[root] != root) root = parent[root];
        // Pass 2: PATH COMPRESSION - point every node on the path straight at the root.
        while (parent[x] != root) {
            int next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    // unite: merges the sets containing a and b.
    // Returns true if they were different sets (a merge happened),
    // false if they were already in the same set (adding this road would make a loop)
    // or if a or b is out of range.
    bool unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == -1 || rb == -1 || ra == rb) return false;

        // UNION BY RANK: attach the lower-rank tree under the higher-rank tree.
        if (rnk[ra] < rnk[rb]) {
            parent[ra] = rb;
        } else if (rnk[ra] > rnk[rb]) {
            parent[rb] = ra;
        } else {                       // equal ranks: pick one, its rank grows by 1
            parent[rb] = ra;
            rnk[ra]++;
        }
        sets--;
        return true;
    }

    // connected: true if a and b are in the same set.
    bool connected(int a, int b) {
        int ra = find(a);
        return ra != -1 && ra == find(b);
    }

    int setCount() const { return sets; }   // how many separate sets exist now
    int size() const { return n; }          // how many elements
};
