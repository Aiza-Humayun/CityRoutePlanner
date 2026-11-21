/*
 * =============================================================================
 * File    : Stack.h
 * Part    : C (Foundations, Interface, Web)
 * Purpose : A stack (last in, first out) stored in a DYNAMIC ARRAY.
 *           Used by Part B: Stack<std::string> for undo snapshots and
 *           Stack<int> for the iterative DFS in the connectivity check.
 * Course  : Lab 5 - stacks.  Lab 2 - dynamic arrays.  Lab 1 - Rule of Three.
 *
 * HOW IT WORKS (viva summary)
 *   - Items live in an array. 'count' says how many are used, so the top item
 *     is data[count - 1].
 *   - push puts the new item at data[count]. If the array is full, we first
 *     allocate an array TWICE as big and copy everything (doubling). Doubling
 *     makes push O(1) on average ("amortized"), because the expensive copy
 *     happens rarely.
 *   - pop just lowers 'count' and returns the item.
 *   - Compare with Queue.h: that one is a CIRCULAR array (a different
 *     strategy for a different access pattern).
 *
 * COMPLEXITY
 *   push O(1) amortized, pop O(1), top O(1), isEmpty/size O(1), clear O(1)
 *
 * SAFETY: pop() / top() on an empty stack must NOT crash. They print a message
 * to std::cerr and return T() (callers should still check isEmpty() first).
 * =============================================================================
 */
#pragma once
#include <iostream>

template<typename T>
class Stack {
private:
    T*  data;        // the array
    int capacity;    // size of the array
    int count;       // items stored

    static constexpr int INITIAL_CAPACITY = 16;

    // Replaces the array with one twice as big.  Time: O(n)
    void grow() {
        int newCapacity = capacity * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    // Creates an empty stack.
    Stack() : data(new T[INITIAL_CAPACITY]), capacity(INITIAL_CAPACITY), count(0) {}

    // RULE OF THREE (1/3): copy constructor - own array, own copies of the items.
    Stack(const Stack& other)
        : data(new T[other.capacity]), capacity(other.capacity), count(other.count) {
        for (int i = 0; i < count; i++) data[i] = other.data[i];
    }

    // RULE OF THREE (2/3): copy assignment.
    Stack& operator=(const Stack& other) {
        if (this != &other) {                       // guard against "a = a"
            T* newData = new T[other.capacity];     // build the copy first
            for (int i = 0; i < other.count; i++) newData[i] = other.data[i];
            delete[] data;                          // then release the old array
            data = newData;
            capacity = other.capacity;
            count = other.count;
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor.
    ~Stack() { delete[] data; }

    // Puts a value on top.  Time: O(1) amortized
    void push(const T& value) {
        if (count == capacity) grow();
        data[count] = value;
        count++;
    }

    // Removes and returns the top value.
    // Empty stack: message to cerr, returns T().  Time: O(1)
    T pop() {
        if (count == 0) {
            std::cerr << "Stack::pop on empty stack\n";
            return T();
        }
        count--;
        return data[count];
    }

    // Reference to the top value (it can be read or changed in place).
    // Empty stack: message to cerr, returns a reference to a blank T().
    // Time: O(1)
    T& top() {
        if (count == 0) {
            std::cerr << "Stack::top on empty stack\n";
            static T blank;          // one shared dummy so we still have something to refer to
            blank = T();
            return blank;
        }
        return data[count - 1];
    }

    bool isEmpty() const { return count == 0; }   // Time: O(1)
    int  size() const { return count; }           // Time: O(1)

    // Empties the stack (the array is kept for reuse).  Time: O(1)
    void clear() { count = 0; }
};
