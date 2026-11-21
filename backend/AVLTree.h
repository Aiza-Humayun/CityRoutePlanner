/*
 * =============================================================================
 * File    : AVLTree.h
 * Part    : B (Map Data and Analysis)
 * Purpose : Keeps all location NAMES sorted so we can show an alphabetical list
 *           and do prefix search ("sa" finds Saddar, Sea View, ...).
 *           Each node stores the name and the location's id.
 * Course  : Lab 8  - Binary search trees and their operations.
 *           Lab 9  - AVL trees (balance factor, rotations, insert and remove).
 *           Lab 6  - Recursion (every operation here is recursive).
 *
 * WHY AVL AND NOT A PLAIN BST?
 *   A plain BST can become a long "stick" if names are inserted in sorted order
 *   (height n, so searching costs O(n)). An AVL tree rebalances itself with
 *   rotations after each insert/remove so its height is always O(log n).
 *
 * KEY IDEAS
 *   - height of a node  = 1 + max(height of left, height of right); empty = 0
 *   - balance factor    = height(left) - height(right); must stay -1, 0 or +1
 *   - If a node becomes unbalanced there are 4 cases, fixed by rotations:
 *        LL (left-left)   -> one right rotation
 *        RR (right-right) -> one left rotation
 *        LR (left-right)  -> left rotation on the left child, then right rotation
 *        RL (right-left)  -> right rotation on the right child, then left rotation
 *
 * COMPLEXITY
 *   insert / remove / find : O(log n)
 *   in-order listing       : O(n)
 *   prefix search          : O(log n + k)   (k = number of matches)
 *
 * Names are compared case-insensitively (we compare lowercase "keys"), but the
 * original spelling is kept in the node for display.
 * =============================================================================
 */
#pragma once
#include <string>

class AVLTree {
private:
    // One tree node.
    struct Node {
        std::string name;     // original spelling, e.g. "Gulshan-e-Iqbal"
        std::string key;      // lowercase version, used for ordering
        int         id;       // location id
        int         height;   // height of the subtree rooted here (leaf = 1)
        Node*       left;     // names that come BEFORE this one
        Node*       right;    // names that come AFTER this one

        Node(const std::string& n, const std::string& k, int i)
            : name(n), key(k), id(i), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root;    // top of the tree (nullptr when empty)
    int   count;   // number of names stored

    // ---------------------------------------------------------------- helpers
    static std::string toLowerKey(const std::string& s) {
        std::string out = s;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] >= 'A' && out[i] <= 'Z') out[i] = (char)(out[i] + ('a' - 'A'));
        }
        return out;
    }

    static int heightOf(Node* n) { return n ? n->height : 0; }   // empty subtree = 0
    static int maxOf(int a, int b) { return a > b ? a : b; }

    // Recomputes a node's height from its children (call after changing children).
    static void updateHeight(Node* n) {
        n->height = 1 + maxOf(heightOf(n->left), heightOf(n->right));
    }

    // balance factor = left height - right height  (positive = left heavy)
    static int balanceOf(Node* n) {
        return n ? heightOf(n->left) - heightOf(n->right) : 0;
    }

    // ------------------------------------------------------------- rotations
    /*
     * Right rotation around y (used when the LEFT side is too tall):
     *
     *         y                x
     *        / \              / \
     *       x   C    ==>     A   y
     *      / \                  / \
     *     A   B                B   C
     *
     * Order is preserved: A < x < B < y < C. Returns the new top (x).
     */
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* b = x->right;
        x->right = y;          // y becomes the right child of x
        y->left = b;           // x's old right subtree moves under y
        updateHeight(y);       // y is lower now, so update it FIRST
        updateHeight(x);
        return x;
    }

    /*
     * Left rotation around x (used when the RIGHT side is too tall): mirror image.
     *
     *       x                    y
     *      / \                  / \
     *     A   y      ==>       x   C
     *        / \              / \
     *       B   C            A   B
     */
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* b = y->left;
        y->left = x;
        x->right = b;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Checks one node and fixes it if unbalanced. Returns the (possibly new) top of
    // this subtree. Called on the way back up after every insert/remove.
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
            if (balanceOf(n->right) > 0) {           // case RL: right child leans left
                n->right = rotateRight(n->right);    //   first turn it into case RR
            }
            return rotateLeft(n);                    // case RR (or fixed RL)
        }
        return n;                                    // already balanced
    }

    // ---------------------------------------------------------------- insert
    // Recursive BST insert, then rebalance on the way back up.
    // 'inserted' becomes true only if a new node was really added.
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

    // ---------------------------------------------------------------- remove
    // Recursive BST delete (3 cases), then rebalance on the way back up.
    static Node* removeNode(Node* n, const std::string& key, bool& removed) {
        if (n == nullptr) return nullptr;            // key not in the tree

        if (key < n->key) {
            n->left = removeNode(n->left, key, removed);
        } else if (key > n->key) {
            n->right = removeNode(n->right, key, removed);
        } else {
            removed = true;
            // Case 1 and 2: zero or one child -> replace the node by its child.
            if (n->left == nullptr || n->right == nullptr) {
                Node* child = n->left ? n->left : n->right;
                delete n;
                return child;                        // (nullptr if it was a leaf)
            }
            // Case 3: two children -> copy the IN-ORDER SUCCESSOR (smallest node
            // of the right subtree) into this node, then delete that successor.
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

    // ------------------------------------------------------------- traversals
    // In-order = left, node, right = alphabetical order.
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

    // Prefix search. All keys that start with 'prefix' form one continuous block in
    // sorted order, so we only walk into the subtrees that can contain that block:
    //   - go LEFT  only if this key >= prefix (smaller keys could still match)
    //   - go RIGHT only if this key < prefix, or it starts with the prefix
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

    // Used by isBalanced(): recomputes the real height and checks every node.
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

    // Deep copy / destroy (recursive).
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

    // RULE OF THREE (1/3): copy constructor - deep copy of the whole tree.
    AVLTree(const AVLTree& other) : root(cloneTree(other.root)), count(other.count) {}

    // RULE OF THREE (2/3): copy assignment.
    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {
            destroyTree(root);
            root = cloneTree(other.root);
            count = other.count;
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor.
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
        while (n != nullptr) {                       // ordinary BST search (iterative)
            if (key < n->key)      n = n->left;
            else if (key > n->key) n = n->right;
            else                   return n->id;
        }
        return -1;
    }

    /*
     * toArray: fills 'names' and 'ids' with every entry in ALPHABETICAL order.
     * The caller must allocate both arrays with at least size() elements.
     * Returns how many entries were written.
     */
    int toArray(std::string* names, int* ids) const {
        int pos = 0;
        fillInOrder(root, names, ids, pos);
        return pos;
    }

    /*
     * prefixSearch: fills the arrays with every name that starts with 'prefix'
     * (case-insensitive), in alphabetical order. An empty prefix returns everything.
     * The caller must allocate both arrays with at least size() elements.
     * Returns how many matches were written.
     */
    int prefixSearch(const std::string& prefix, std::string* names, int* ids) const {
        int pos = 0;
        fillPrefix(root, toLowerKey(prefix), names, ids, pos);
        return pos;
    }

    int size() const { return count; }
    int height() const { return heightOf(root); }

    // For tests: true if every node has balance factor -1, 0 or +1 and correct height.
    bool isBalanced() const {
        int h = 0;
        return checkBalanced(root, h);
    }

    // clear: removes everything.
    void clear() {
        destroyTree(root);
        root = nullptr;
        count = 0;
    }
};
