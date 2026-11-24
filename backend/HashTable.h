
#pragma once
#include <string>

class HashTable {
private:
    // One entry in a bucket's chain (a singly linked list node).
    struct Entry {
        std::string key;     // lowercase name
        int         value;   // location id
        Entry*      next;    // next entry in the same bucket
    };

    Entry** buckets;         // dynamic array of chain heads (nullptr = empty bucket)
    int     capacity;        // number of buckets
    int     count;           // number of entries stored
    int     rehashCount;     // how many times the table has grown (used by tests)

    static constexpr int INITIAL_CAPACITY = 16;

    // Converts A-Z to a-z. Other characters are left unchanged.
    static std::string toLowerKey(const std::string& s) {
        std::string out = s;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] >= 'A' && out[i] <= 'Z') out[i] = (char)(out[i] + ('a' - 'A'));
        }
        return out;
    }

    // The hash function: polynomial rolling hash with multiplier 31.
    // (unsigned long arithmetic wraps around on overflow, which is fine here.)
    unsigned long hashOf(const std::string& lowerKey) const {
        unsigned long h = 0;
        for (size_t i = 0; i < lowerKey.size(); i++) {
            h = h * 31 + (unsigned char)lowerKey[i];
        }
        return h;
    }

    // Bucket index for a key in a table with 'cap' buckets.
    int indexFor(const std::string& lowerKey, int cap) const {
        return (int)(hashOf(lowerKey) % (unsigned long)cap);
    }

    // Builds a bigger table and moves every entry into it (REHASHING).
    void rehash() {
        int newCapacity = capacity * 2 + 1;              // roughly double, kept odd
        Entry** newBuckets = new Entry*[newCapacity]();  // () sets all to nullptr
        for (int b = 0; b < capacity; b++) {
            Entry* e = buckets[b];
            while (e != nullptr) {
                Entry* nextEntry = e->next;              // remember before we relink
                int idx = indexFor(e->key, newCapacity); // new position in bigger table
                e->next = newBuckets[idx];               // put at the front of new chain
                newBuckets[idx] = e;
                e = nextEntry;
            }
        }
        delete[] buckets;                                // old array only (entries moved)
        buckets = newBuckets;
        capacity = newCapacity;
        rehashCount++;
    }

    // Deletes every entry in every chain (does not delete the bucket array).
    void deleteAllEntries() {
        for (int b = 0; b < capacity; b++) {
            Entry* e = buckets[b];
            while (e != nullptr) {
                Entry* nextEntry = e->next;
                delete e;
                e = nextEntry;
            }
            buckets[b] = nullptr;
        }
        count = 0;
    }

    // Deep copy used by the copy constructor and copy assignment.
    void copyFrom(const HashTable& other) {
        capacity = other.capacity;
        count = 0;
        rehashCount = other.rehashCount;
        buckets = new Entry*[capacity]();
        for (int b = 0; b < other.capacity; b++) {
            for (Entry* e = other.buckets[b]; e != nullptr; e = e->next) {
                Entry* copy = new Entry{e->key, e->value, buckets[b]};
                buckets[b] = copy;
                count++;
            }
        }
    }

public:
    // Creates an empty table with 16 buckets.
    HashTable() : buckets(new Entry*[INITIAL_CAPACITY]()), capacity(INITIAL_CAPACITY),
                  count(0), rehashCount(0) {}

    // RULE OF THREE (1/3): copy constructor - makes an independent deep copy.
    HashTable(const HashTable& other) { copyFrom(other); }

    // RULE OF THREE (2/3): copy assignment - frees our data, then deep copies.
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {            // guard against "a = a"
            deleteAllEntries();
            delete[] buckets;
            copyFrom(other);
        }
        return *this;
    }

    // RULE OF THREE (3/3): destructor - releases all memory.
    ~HashTable() {
        deleteAllEntries();
        delete[] buckets;
    }

    /*
     * insert: stores name -> id.
     * Returns true if a NEW entry was added.
     * Returns false if the name already existed (its id is overwritten).
     */
    bool insert(const std::string& name, int id) {
        std::string key = toLowerKey(name);
        int idx = indexFor(key, capacity);

        // Walk the chain: if the key is already there, just update its value.
        for (Entry* e = buckets[idx]; e != nullptr; e = e->next) {
            if (e->key == key) {
                e->value = id;
                return false;
            }
        }
        // Not found: add a new entry at the FRONT of the chain (O(1)).
        buckets[idx] = new Entry{key, id, buckets[idx]};
        count++;

        // Load factor = count / capacity. Rehash when it goes above 0.7.
        // (Written with integers: count/capacity > 7/10  <=>  count*10 > capacity*7)
        if (count * 10 > capacity * 7) rehash();
        return true;
    }

    // find: returns the id stored for this name, or -1 if the name is not present.
    int find(const std::string& name) const {
        std::string key = toLowerKey(name);
        int idx = indexFor(key, capacity);
        for (Entry* e = buckets[idx]; e != nullptr; e = e->next) {
            if (e->key == key) return e->value;
        }
        return -1;
    }

    // remove: deletes the entry for this name. Returns true if something was removed.
    bool remove(const std::string& name) {
        std::string key = toLowerKey(name);
        int idx = indexFor(key, capacity);
        Entry* prev = nullptr;
        for (Entry* e = buckets[idx]; e != nullptr; prev = e, e = e->next) {
            if (e->key == key) {
                if (prev == nullptr) buckets[idx] = e->next;  // removing the first entry
                else                 prev->next = e->next;    // unlink from the middle/end
                delete e;
                count--;
                return true;
            }
        }
        return false;
    }

    // clear: removes all entries (the bucket array keeps its current size).
    void clear() { deleteAllEntries(); }

    int size() const { return count; }                 // number of stored names
    int getCapacity() const { return capacity; }       // number of buckets (for tests)
    int getRehashCount() const { return rehashCount; } // how many times we rehashed (for tests)
};
