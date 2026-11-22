#pragma once

class DisjointSet {
private:
    int* parent;    
    int* rnk;       
    int  n;         
    int  sets;     

    void init(int size) {
        n = size;
        sets = size;
        parent = new int[n > 0 ? n : 1];    
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
    explicit DisjointSet(int size = 0) { init(size); }

    DisjointSet(const DisjointSet& other) { copyFrom(other); }

    DisjointSet& operator=(const DisjointSet& other) {
        if (this != &other) {
            delete[] parent;
            delete[] rnk;
            copyFrom(other);
        }
        return *this;
    }

    ~DisjointSet() {
        delete[] parent;
        delete[] rnk;
    }

    int find(int x) {
        if (x < 0 || x >= n) return -1;
        int root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != root) {
            int next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    bool unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == -1 || rb == -1 || ra == rb) return false;

        if (rnk[ra] < rnk[rb]) {
            parent[ra] = rb;
        } else if (rnk[ra] > rnk[rb]) {
            parent[rb] = ra;
        } else {                      
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

    int setCount() const { return sets; }   
    int size() const { return n; }          
};
