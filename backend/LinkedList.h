/*
 * =============================================================================
 * File    : LinkedList.h
 * Part    : C (Foundations, Interface, Web)
 * Purpose : Two linked list templates used by the other parts:
 *             SinglyLinkedList<T> - Part A stores the final route (source to
 *                                   destination) in one.
 *             DoublyLinkedList<T> - Part B's TripHistory (oldest at the head,
 *                                   newest at the tail).
 * Course  : Lab 4 - linked lists.   Lab 1 - Rule of Three.
 *
 * HOW THEY WORK (viva summary)
 *   - A list is a chain of small separate blocks (nodes). Each node holds one
 *     value and a pointer to the next node (doubly: also to the previous one).
 *   - We keep a head pointer AND a tail pointer, so adding at either end does
 *     not need to walk the list.
 *   - RULE OF THREE: the list owns memory (its nodes). Without our own
 *     destructor the nodes would leak; without our own copy constructor and
 *     copy assignment, a copy would share the same nodes and both lists would
 *     delete them (crash). So we write all three and make real deep copies.
 *
 * COMPLEXITY
 *                         Singly          Doubly
 *   pushFront / pushBack  O(1)            O(1)
 *   popFront              O(1)            O(1)
 *   popBack               (not provided)  O(1)   <- thanks to the 'prev' pointer
 *   get(index)            O(n)            (not provided)
 *   reverse               O(n)            (not provided)
 *   clear / copy          O(n)            O(n)
 *
 * SAFETY: popFront / popBack / get on an empty list or a bad index must NOT
 * crash. They print a message to std::cerr and return T() (never to cout).
 * =============================================================================
 */
#pragma once
#include <iostream>

// ============================================================================
//  SINGLY LINKED LIST
// ============================================================================

// One node of a singly linked list.
template<typename T>
struct ListNode {
    T          data;   // the stored value
    ListNode*  next;   // next node (nullptr at the end)
    ListNode(const T& d = T(), ListNode* n = nullptr) : data(d), next(n) {}
};

template<typename T>
class SinglyLinkedList {
private:
    ListNode<T>* head;    // first node (nullptr when empty)
    ListNode<T>* tail;    // last node  (nullptr when empty)
    int          count;   // number of nodes

    // Appends a copy of every value of 'other' (used by copy constructor/assignment).
    // Time: O(n)
    void copyFrom(const SinglyLinkedList& other) {
        for (ListNode<T>* n = other.head; n != nullptr; n = n->next) {
            pushBack(n->data);
        }
    }

public:
    // Creates an empty list.
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // RULE OF THREE (1/3): copy constructor - builds brand new nodes.
    SinglyLinkedList(const SinglyLinkedList& other) : head(nullptr), tail(nullptr), count(0) {
        copyFrom(other);
    }

    // RULE OF THREE (2/3): copy assignment - frees our nodes, then deep copies.
    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
        if (this != &other) {          // guard against "a = a"
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor - frees every node.
    ~SinglyLinkedList() { clear(); }

    // Adds a value at the front.  Time: O(1)
    void pushFront(const T& value) {
        ListNode<T>* node = new ListNode<T>(value, head);   // new node points to old head
        head = node;
        if (tail == nullptr) tail = node;                   // list was empty
        count++;
    }

    // Adds a value at the back.  Time: O(1) (thanks to the tail pointer)
    void pushBack(const T& value) {
        ListNode<T>* node = new ListNode<T>(value, nullptr);
        if (tail == nullptr) head = node;                   // list was empty
        else                 tail->next = node;
        tail = node;
        count++;
    }

    // Removes the first value and returns it.
    // Empty list: message to cerr, returns T().  Time: O(1)
    T popFront() {
        if (head == nullptr) {
            std::cerr << "SinglyLinkedList::popFront on empty list\n";
            return T();
        }
        ListNode<T>* node = head;
        T value = node->data;
        head = head->next;
        if (head == nullptr) tail = nullptr;                // list became empty
        delete node;
        count--;
        return value;
    }

    bool isEmpty() const { return count == 0; }   // Time: O(1)
    int  size() const { return count; }           // Time: O(1)

    // Deletes every node.  Time: O(n)
    void clear() {
        ListNode<T>* n = head;
        while (n != nullptr) {
            ListNode<T>* nextNode = n->next;     // remember before deleting
            delete n;
            n = nextNode;
        }
        head = nullptr;
        tail = nullptr;
        count = 0;
    }

    // First node, so callers can walk:  for (n = list.getHead(); n; n = n->next)
    ListNode<T>* getHead() const { return head; }

    // Value at position 'index' (0 = first). Bad index: message to cerr, returns T().
    // Time: O(index)
    T get(int index) const {
        if (index < 0 || index >= count) {
            std::cerr << "SinglyLinkedList::get index " << index << " out of range\n";
            return T();
        }
        ListNode<T>* n = head;
        for (int i = 0; i < index; i++) n = n->next;
        return n->data;
    }

    // Reverses the list in place by turning every 'next' pointer around.
    // Time: O(n), no extra memory.
    void reverse() {
        ListNode<T>* previous = nullptr;
        ListNode<T>* current = head;
        tail = head;                              // the old first node becomes the last
        while (current != nullptr) {
            ListNode<T>* nextNode = current->next;
            current->next = previous;             // turn the pointer around
            previous = current;
            current = nextNode;
        }
        head = previous;
    }
};

// ============================================================================
//  DOUBLY LINKED LIST
// ============================================================================

// One node of a doubly linked list.
template<typename T>
struct DNode {
    T      data;   // the stored value
    DNode* prev;   // node before this one (nullptr at the front)
    DNode* next;   // node after this one  (nullptr at the back)
    DNode(const T& d = T(), DNode* p = nullptr, DNode* n = nullptr)
        : data(d), prev(p), next(n) {}
};

template<typename T>
class DoublyLinkedList {
private:
    DNode<T>* head;    // first node (oldest in TripHistory)
    DNode<T>* tail;    // last node  (newest in TripHistory)
    int       count;

    // Appends a copy of every value of 'other'.  Time: O(n)
    void copyFrom(const DoublyLinkedList& other) {
        for (DNode<T>* n = other.head; n != nullptr; n = n->next) {
            pushBack(n->data);
        }
    }

public:
    // Creates an empty list.
    DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // RULE OF THREE (1/3): copy constructor.
    DoublyLinkedList(const DoublyLinkedList& other) : head(nullptr), tail(nullptr), count(0) {
        copyFrom(other);
    }

    // RULE OF THREE (2/3): copy assignment.
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor.
    ~DoublyLinkedList() { clear(); }

    // Adds a value at the front.  Time: O(1)
    void pushFront(const T& value) {
        DNode<T>* node = new DNode<T>(value, nullptr, head);
        if (head != nullptr) head->prev = node;    // old first node now has a node before it
        else                 tail = node;          // list was empty
        head = node;
        count++;
    }

    // Adds a value at the back.  Time: O(1)
    void pushBack(const T& value) {
        DNode<T>* node = new DNode<T>(value, tail, nullptr);
        if (tail != nullptr) tail->next = node;
        else                 head = node;          // list was empty
        tail = node;
        count++;
    }

    // Removes the first value and returns it. Empty: message to cerr, returns T().
    // Time: O(1)
    T popFront() {
        if (head == nullptr) {
            std::cerr << "DoublyLinkedList::popFront on empty list\n";
            return T();
        }
        DNode<T>* node = head;
        T value = node->data;
        head = head->next;
        if (head != nullptr) head->prev = nullptr;
        else                 tail = nullptr;       // list became empty
        delete node;
        count--;
        return value;
    }

    // Removes the last value and returns it. Empty: message to cerr, returns T().
    // Time: O(1) - the 'prev' pointer lets us step back without walking.
    T popBack() {
        if (tail == nullptr) {
            std::cerr << "DoublyLinkedList::popBack on empty list\n";
            return T();
        }
        DNode<T>* node = tail;
        T value = node->data;
        tail = tail->prev;
        if (tail != nullptr) tail->next = nullptr;
        else                 head = nullptr;       // list became empty
        delete node;
        count--;
        return value;
    }

    bool isEmpty() const { return count == 0; }
    int  size() const { return count; }

    // Deletes every node.  Time: O(n)
    void clear() {
        DNode<T>* n = head;
        while (n != nullptr) {
            DNode<T>* nextNode = n->next;
            delete n;
            n = nextNode;
        }
        head = nullptr;
        tail = nullptr;
        count = 0;
    }

    DNode<T>* getHead() const { return head; }   // oldest (walk forward with 'next')
    DNode<T>* getTail() const { return tail; }   // newest (walk backward with 'prev')
};
