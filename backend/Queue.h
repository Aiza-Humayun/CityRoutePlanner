/*
 * =============================================================================
 * File    : Queue.h
 * Part    : C (Foundations, Interface, Web)
 * Purpose : A queue (first in, first out) stored in a CIRCULAR ARRAY.
 *           Used by Part A: Queue<int> in the BFS that finds the route with
 *           the fewest roads.
 * Course  : Lab 5 - queues.  Lab 2 - dynamic arrays.  Lab 1 - Rule of Three.
 *
 * HOW IT WORKS (viva summary)
 *   - Items live in an array. 'frontIndex' is where the oldest item is and
 *     'count' says how many items there are.
 *   - enqueue writes at position (frontIndex + count) % capacity,
 *     dequeue reads at frontIndex and then moves frontIndex forward by one
 *     (also with % capacity).
 *   - WHY CIRCULAR? In a plain array, dequeue would either shift every item
 *     one step (O(n)) or leave an ever-growing empty gap at the start. With
 *     modulo, positions that were freed at the front are REUSED at the back,
 *     so both enqueue and dequeue are O(1) and no space is wasted.
 *   - When the array is full it is replaced by one twice as big. The items are
 *     copied IN QUEUE ORDER, so the new array starts at index 0 again.
 *   - Compare with Stack.h: that one is a plain dynamic array.
 *
 * COMPLEXITY
 *   enqueue O(1) amortized, dequeue O(1), front O(1), isEmpty/size O(1), clear O(1)
 *
 * SAFETY: dequeue() / front() on an empty queue must NOT crash. They print a
 * message to std::cerr and return T() (callers should still check isEmpty()).
 * =============================================================================
 */
#pragma once
#include <iostream>

template<typename T>
class Queue {
private:
    T*  data;          // the circular array
    int capacity;      // size of the array
    int frontIndex;    // position of the oldest item
    int count;         // items stored

    static constexpr int INITIAL_CAPACITY = 16;

    // Replaces the array with one twice as big, copying the items in queue
    // order so that the oldest one lands at index 0.  Time: O(n)
    void grow() {
        int newCapacity = capacity * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < count; i++) {
            bigger[i] = data[(frontIndex + i) % capacity];   // walk from the front, wrapping around
        }
        delete[] data;
        data = bigger;
        capacity = newCapacity;
        frontIndex = 0;
    }

public:
    // Creates an empty queue.
    Queue() : data(new T[INITIAL_CAPACITY]), capacity(INITIAL_CAPACITY), frontIndex(0), count(0) {}

    // RULE OF THREE (1/3): copy constructor - own array, items copied in queue order.
    Queue(const Queue& other)
        : data(new T[other.capacity]), capacity(other.capacity), frontIndex(0), count(other.count) {
        for (int i = 0; i < count; i++) {
            data[i] = other.data[(other.frontIndex + i) % other.capacity];
        }
    }

    // RULE OF THREE (2/3): copy assignment.
    Queue& operator=(const Queue& other) {
        if (this != &other) {                       // guard against "a = a"
            T* newData = new T[other.capacity];     // build the copy first
            for (int i = 0; i < other.count; i++) {
                newData[i] = other.data[(other.frontIndex + i) % other.capacity];
            }
            delete[] data;                          // then release the old array
            data = newData;
            capacity = other.capacity;
            frontIndex = 0;
            count = other.count;
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor.
    ~Queue() { delete[] data; }

    // Adds a value at the back.  Time: O(1) amortized
    void enqueue(const T& value) {
        if (count == capacity) grow();
        data[(frontIndex + count) % capacity] = value;   // wraps around the end of the array
        count++;
    }

    // Removes and returns the oldest value.
    // Empty queue: message to cerr, returns T().  Time: O(1)
    T dequeue() {
        if (count == 0) {
            std::cerr << "Queue::dequeue on empty queue\n";
            return T();
        }
        T value = data[frontIndex];
        frontIndex = (frontIndex + 1) % capacity;        // move the front forward, wrapping
        count--;
        return value;
    }

    // Reference to the oldest value (not removed).
    // Empty queue: message to cerr, returns a reference to a blank T().
    // Time: O(1)
    T& front() {
        if (count == 0) {
            std::cerr << "Queue::front on empty queue\n";
            static T blank;          // one shared dummy so we still have something to refer to
            blank = T();
            return blank;
        }
        return data[frontIndex];
    }

    bool isEmpty() const { return count == 0; }   // Time: O(1)
    int  size() const { return count; }           // Time: O(1)

    // Empties the queue (the array is kept for reuse).  Time: O(1)
    void clear() {
        frontIndex = 0;
        count = 0;
    }
};
