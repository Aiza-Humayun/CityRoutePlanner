
#pragma once
#include <iostream>

template<typename T>
class Queue {
private:
    T*  data;          // the circular array
    int capacity;      // size of the array
    int frontIndex;    
    int count;         // items stored

    static constexpr int INITIAL_CAPACITY = 16;

   
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
