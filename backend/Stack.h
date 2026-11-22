#pragma once
#include <iostream>

template<typename T>
class Stack {
private:
    T*  data;        
    int capacity;   
    int count;       

    static constexpr int INITIAL_CAPACITY = 16;

    void grow() {
        int newCapacity = capacity * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    Stack() : data(new T[INITIAL_CAPACITY]), capacity(INITIAL_CAPACITY), count(0) {}

    Stack(const Stack& other)
        : data(new T[other.capacity]), capacity(other.capacity), count(other.count) {
        for (int i = 0; i < count; i++) data[i] = other.data[i];
    }

    Stack& operator=(const Stack& other) {
        if (this != &other) {                       
            T* newData = new T[other.capacity];    
            for (int i = 0; i < other.count; i++) newData[i] = other.data[i];
            delete[] data;                        
            data = newData;
            capacity = other.capacity;
            count = other.count;
        }
        return *this;
    }

    ~Stack() { delete[] data; }

    void push(const T& value) {
        if (count == capacity) grow();
        data[count] = value;
        count++;
    }

    T pop() {
        if (count == 0) {
            std::cerr << "Stack::pop on empty stack\n";
            return T();
        }
        count--;
        return data[count];
    }

    T& top() {
        if (count == 0) {
            std::cerr << "Stack::top on empty stack\n";
            static T blank;          
            blank = T();
            return blank;
        }
        return data[count - 1];
    }

    bool isEmpty() const { return count == 0; }  
    int  size() const { return count; }           

    void clear() { count = 0; }
};
