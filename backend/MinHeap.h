#pragma once

 

#include <iostream>
using namespace std;

// One item in the heap: a location id and its cost so far.
struct HeapEntry {
    int    node;
    double priority;
};

class MinHeap {
private:
    HeapEntry* data;    // the array
    int capacity;       // size of the array
    int count;          // items stored

    // Swaps two items.  Time: O(1)
    void swapItems(int i, int j) {
        HeapEntry temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    // Makes the array twice as big.  Time: O(n)
    void grow() {
        int newCapacity = capacity * 2;
        HeapEntry* bigger = new HeapEntry[newCapacity];
        for (int i = 0; i < count; i++) {
            bigger[i] = data[i];
        }
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

    // Moves item i UP while it is smaller than its parent.  Time: O(log n)
    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (data[i].priority >= data[parent].priority) {
                break;                      // parent is smaller: correct place
            }
            swapItems(i, parent);
            i = parent;
        }
    }

    // Moves item i DOWN while a child is smaller than it.  Time: O(log n)
    void siftDown(int i) {
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < count && data[left].priority < data[smallest].priority) {
                smallest = left;
            }
            if (right < count && data[right].priority < data[smallest].priority) {
                smallest = right;
            }
            if (smallest == i) {
                break;                      // both children are bigger: done
            }
            swapItems(i, smallest);
            i = smallest;
        }
    }

public:
    // Creates an empty heap with room for 16 items.
    MinHeap() {
        capacity = 16;
        data = new HeapEntry[capacity];
        count = 0;
    }

    // Rule of Three: destructor
    ~MinHeap() {
        delete[] data;
    }

    // Rule of Three: copy constructor (makes its own array)
    MinHeap(const MinHeap& other) {
        capacity = other.capacity;
        count = other.count;
        data = new HeapEntry[capacity];
        for (int i = 0; i < count; i++) {
            data[i] = other.data[i];
        }
    }

    // Rule of Three: copy assignment
    MinHeap& operator=(const MinHeap& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = new HeapEntry[capacity];
            for (int i = 0; i < count; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // Adds an item: put it at the end, then move it up.  Time: O(log n)
    void push(int node, double priority) {
        if (count == capacity) {
            grow();
        }
        data[count].node = node;
        data[count].priority = priority;
        siftUp(count);
        count++;
    }

    // Removes and returns the smallest item.
    // The last item is moved to the top and then moved down.
    // If the heap is empty: prints a message to cerr and returns node -1.
    // Time: O(log n)
    HeapEntry pop() {
        if (count == 0) {
            cerr << "MinHeap::pop on empty heap\n";
            HeapEntry empty;
            empty.node = -1;
            empty.priority = 0;
            return empty;
        }
        HeapEntry smallest = data[0];
        count--;
        if (count > 0) {
            data[0] = data[count];
            siftDown(0);
        }
        return smallest;
    }

    // True if there are no items.  Time: O(1)
    bool isEmpty() const {
        return count == 0;
    }

    // Number of items.  Time: O(1)
    int size() const {
        return count;
    }
};
