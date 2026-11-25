#pragma once
#include <string>

class AVLTree {
private:
    // ISME NODE BANAY HAIII 
    struct Node {
        std::string name;     
        std::string key;     
        int         id;      
        int         height;   
        Node*       left;     
        Node*       right;    

        Node(const std::string& n, const std::string& k, int i)
            : name(n), key(k), id(i), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root;    // top of the tree (nullptr when empty)
    int   count;   // number of names stored


    static std::string toLowerKey(const std::string& s) {
        std::string out = s;
        for (size_t i = 0; i < out.size(); i++) {

            
            
            if (out[i] >= 'A' && out[i] <= 'Z') out[i] = (char)(out[i] + ('a' - 'A'));
        }
        return out;
    }

    static int heightOf(Node* n) { return n ? n->height : 0; }   
    static int maxOf(int a, int b) { return a > b ? a : b; }

    static void updateHeight(Node* n) {
        n->height = 1 + maxOf(heightOf(n->left), heightOf(n->right));
    }

    // balance factor = left height - right height  (positive = left heavy)
    static int balanceOf(Node* n) {
        return n ? heightOf(n->left) - heightOf(n->right) : 0;
    }

    
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* b = x->right;
        x->right = y;          // y becomes the right child of x
        y->left = b;   

        
        updateHeight(y);       // y is lower now, so update it FIRST
        updateHeight(x);
        return x;
    }

    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* b = y->left;
        y->left = x;
        x->right = b;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    static Node* rebalance(Node* n) {
        updateHeight(n);
        int balance = balanceOf(n);

        if (balance > 1) {                           // LEFT side too tall
            if (balanceOf(n->left) < 0) {            // case LR: left child leans right
                n->left = rotateLeft(n->left);       //   first turn it into case LL
            }
            return rotateRight(n);                   // case LL (or fixed LR)
        }
        if (balance < -1) {                          // RIGHT side too tall
            if (balanceOf(n->right) > 0) {   

                
                n->right = rotateRight(n->right);    //   first turn it into case RR
            }
            return rotateLeft(n);                    // case RR (or fixed RL)
        }
        return n;                                    // already balanced
    }

    //insert kra hai
    static Node* insertNode(Node* n, const std::string& name, const std::string& key,
                            int id, bool& inserted) {
        if (n == nullptr) {                          // found the empty spot
            inserted = true;
            return new Node(name, key, id);
        }
        if (key < n->key)      n->left  = insertNode(n->left,  name, key, id, inserted);
        else if (key > n->key) n->right = insertNode(n->right, name, key, id, inserted);
        else {                                       // same key: duplicate, do nothing
            inserted = false;
            return n;
        }
        return rebalance(n);
    }

    //  remove
    static Node* removeNode(Node* n, const std::string& key, bool& removed) {
        if (n == nullptr) return nullptr;            // key not in the tree

        if (key < n->key) {
            n->left = removeNode(n->left, key, removed);

            
        } else if (key > n->key) {
            n->right = removeNode(n->right, key, removed);
        } else {
            removed = true;
            // zero or one child
            if (n->left == nullptr || n->right == nullptr) {
                Node* child = n->left ? n->left : n->right;
                delete n;
                return child;                        // (nullptr if it was a leaf)
            }
    
            Node* successor = n->right;
            while (successor->left != nullptr) successor = successor->left;
            n->name = successor->name;
            n->key  = successor->key;

            
            n->id   = successor->id;
            std::string successorKey = successor->key;   // copy: the node is about to be freed
            bool ignored = false;
            n->right = removeNode(n->right, successorKey, ignored);
        }
        return rebalance(n);
    }

    // traversals kiya hai thora mushkil hai
    static void fillInOrder(Node* n, std::string* names, int* ids, int& pos) {
        if (n == nullptr) return;
        fillInOrder(n->left, names, ids, pos);
        names[pos] = n->name;

        
        ids[pos] = n->id;
        pos++;
        fillInOrder(n->right, names, ids, pos);
    }

    static bool startsWith(const std::string& text, const std::string& prefix) {
        return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
    }

    static void fillPrefix(Node* n, const std::string& prefix,
                           std::string* names, int* ids, int& pos) {

        
        if (n == nullptr) return;
        bool matches = startsWith(n->key, prefix);
        if (n->key >= prefix) fillPrefix(n->left, prefix, names, ids, pos);
        if (matches) {
            names[pos] = n->name;
            ids[pos] = n->id;
            pos++;
        }
        if (n->key < prefix || matches) fillPrefix(n->right, prefix, names, ids, pos);
    }

    
    static bool checkBalanced(Node* n, int& realHeight) {
        if (n == nullptr) { realHeight = 0; return true; }
        int lh = 0, rh = 0;
        if (!checkBalanced(n->left, lh))  return false;
        if (!checkBalanced(n->right, rh)) return false;


        
        realHeight = 1 + maxOf(lh, rh);
        int diff = lh - rh;
        if (diff < -1 || diff > 1) return false;       // balance factor out of range
        if (n->height != realHeight) return false;     // stored height is wrong
        return true;
    }

    
    static Node* cloneTree(Node* n) {
        if (n == nullptr) return nullptr;
        Node* copy = new Node(n->name, n->key, n->id);

        
        copy->height = n->height;
        copy->left = cloneTree(n->left);
        copy->right = cloneTree(n->right);
        
        return copy;
    }
    static void destroyTree(Node* n) {
        if (n == nullptr) return;
        destroyTree(n->left);
        destroyTree(n->right);
        delete n;
    }

public:
    AVLTree() : root(nullptr), count(0) {}

    AVLTree(const AVLTree& other) : root(cloneTree(other.root)), count(other.count) {}

    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {
            destroyTree(root);

            
            root = cloneTree(other.root);

            
            count = other.count;
        }
        return *this;
    }

    ~AVLTree() { destroyTree(root); }

    // insert: adds name -> id. Returns false if the name (ignoring case) already exists.
    bool insert(const std::string& name, int id) {
        bool inserted = false;
                root = insertNode(root, name, toLowerKey(name), id, inserted);
        
        
        if (inserted) count++;
        return inserted;

        
    }

    // remove: deletes a name. Returns false if it was not in the tree.
    bool remove(const std::string& name) {
        bool removed = false;
        root = removeNode(root, toLowerKey(name), removed);
        if (removed) count--;
        return removed;
    }

    // find: returns the id stored for this name, or -1 if not present.
    int find(const std::string& name) const {
        std::string key = toLowerKey(name);
        Node* n = root;
        while (n != nullptr) {                      
            if (key < n->key)      n = n->left;
            else if (key > n->key) n = n->right;
            else                   return n->id;
        }
        return -1;


        
    }

    int toArray(std::string* names, int* ids) const {
        int pos = 0;
        fillInOrder(root, names, ids, pos);
        return pos;


        
    }

    int prefixSearch(const std::string& prefix, std::string* names, int* ids) const {
        int pos = 0;
        fillPrefix(root, toLowerKey(prefix), names, ids, pos);
        return pos;
    }

    int size() const { return count; }
    int height() const { return heightOf(root); }

    bool isBalanced() const {
        int h = 0;
        return checkBalanced(root, h);
    }

    void clear() {
        destroyTree(root);
        root = nullptr;
        count = 0;
    }
};
