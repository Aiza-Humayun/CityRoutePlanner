/*
 * File    : test_containers.cpp
 * Part    : C (Foundations, Interface, Web)
 * Purpose : Tests Json.h, LinkedList.h, Stack.h, Queue.h and Sorting.h on their
 *           own. Prints one PASS/FAIL line per check, then a summary.
 *           Includes the Rule of Three copy tests and sorting 1000 random numbers.
 *
 * Build (from the project root):
 *   g++ -std=c++17 -Wall -I backend -o tests/test_containers.exe tests/test_containers.cpp
 * Run:
 *   tests\test_containers.exe
 *
 * NOTE: a few tests pop from an EMPTY container on purpose. The containers print
 * a short message to cerr (stderr) in that case, so lines starting with
 * "Stack::", "Queue::" or "...LinkedList::" in the output are expected.
 */

#include <iostream>
#include <string>
#include "Json.h"
#include "LinkedList.h"
#include "Stack.h"
#include "Queue.h"
#include "Sorting.h"
using namespace std;

int passed = 0, failed = 0;

// Prints PASS or FAIL for one check and counts it.
void check(const string& name, bool ok, const string& got = "") {
    if (ok) {
        cout << "PASS  " << name << "\n";
        passed++;
    } else {
        cout << "FAIL  " << name << "\n      got: " << got << "\n";
        failed++;
    }
}

// ---------------------------------------------------------------- helpers
// Simple pseudo-random generator (so we do not need <cstdlib>). Same sequence every run.
unsigned long seedValue = 12345;
int nextRandom() {
    seedValue = seedValue * 1103515245UL + 12345UL;
    return (int)((seedValue / 65536UL) % 32768UL);
}

bool lessInt(const int& a, const int& b) { return a < b; }
bool greaterInt(const int& a, const int& b) { return a > b; }

// For the stability test: sort by key only; 'order' remembers the original position.
struct Item { int key; int order; };
bool lessKey(const Item& a, const Item& b) { return a.key < b.key; }

bool isSortedAscending(const int arr[], int n) {
    for (int i = 1; i < n; i++) if (arr[i - 1] > arr[i]) return false;
    return true;
}

// Sum of all values: a cheap check that sorting did not lose or invent items.
long long sumOf(const int arr[], int n) {
    long long s = 0;
    for (int i = 0; i < n; i++) s += arr[i];
    return s;
}

// Lists a singly linked list as "1,2,3" so we can compare it with one check.
string listToText(const SinglyLinkedList<int>& list) {
    string out;
    for (ListNode<int>* n = list.getHead(); n != nullptr; n = n->next) {
        if (!out.empty()) out += ",";
        out += to_string(n->data);
    }
    return out;
}

// Same for a doubly linked list, walking forward (head to tail) ...
string dlistForward(const DoublyLinkedList<int>& list) {
    string out;
    for (DNode<int>* n = list.getHead(); n != nullptr; n = n->next) {
        if (!out.empty()) out += ",";
        out += to_string(n->data);
    }
    return out;
}
// ... and backward (tail to head), which proves the 'prev' pointers are right.
string dlistBackward(const DoublyLinkedList<int>& list) {
    string out;
    for (DNode<int>* n = list.getTail(); n != nullptr; n = n->prev) {
        if (!out.empty()) out += ",";
        out += to_string(n->data);
    }
    return out;
}

// =================================================================== JSON
void testJson() {
    cout << "\n--- Json.h ---\n";
    check("jsonQuote plain text", jsonQuote("abc") == "\"abc\"", jsonQuote("abc"));
    check("jsonQuote escapes quote and backslash",
          jsonQuote("a\"b\\c") == "\"a\\\"b\\\\c\"", jsonQuote("a\"b\\c"));
    check("jsonQuote escapes line break (reply must stay on one line)",
          jsonQuote("x\ny") == "\"x\\ny\"", jsonQuote("x\ny"));
    check("jsonQuote empty string", jsonQuote("") == "\"\"", jsonQuote(""));
    check("jsonQuote keeps normal punctuation (Gulshan-e-Iqbal)",
          jsonQuote("Gulshan-e-Iqbal") == "\"Gulshan-e-Iqbal\"");

    check("jsonError format",
          jsonError("Unknown location: Sadar") == "{\"ok\":false,\"error\":\"Unknown location: Sadar\"}",
          jsonError("Unknown location: Sadar"));
    check("jsonMessage format",
          jsonMessage("Location added") == "{\"ok\":true,\"message\":\"Location added\"}",
          jsonMessage("Location added"));
    check("jsonError escapes a quote inside the message",
          jsonError("bad \"x\"") == "{\"ok\":false,\"error\":\"bad \\\"x\\\"\"}", jsonError("bad \"x\""));

    check("numToStr 12.5 -> 12.5", numToStr(12.5) == "12.5", numToStr(12.5));
    check("numToStr 3.0 -> 3", numToStr(3.0) == "3", numToStr(3.0));
    check("numToStr 0 -> 0", numToStr(0) == "0", numToStr(0));
    check("numToStr 3.456 -> 3.46 (2 decimals)", numToStr(3.456) == "3.46", numToStr(3.456));
    check("numToStr 3.001 -> 3 (no trailing zeros)", numToStr(3.001) == "3", numToStr(3.001));
    check("numToStr 0.05 -> 0.05", numToStr(0.05) == "0.05", numToStr(0.05));
    check("numToStr 99.999 -> 100", numToStr(99.999) == "100", numToStr(99.999));
    check("numToStr 41.2 -> 41.2", numToStr(41.2) == "41.2", numToStr(41.2));
    check("numToStr -2.5 -> -2.5", numToStr(-2.5) == "-2.5", numToStr(-2.5));
    check("numToStr -0.001 -> 0 (no negative zero)", numToStr(-0.001) == "0", numToStr(-0.001));
    check("numToStr 120 -> 120 (trailing zeros of whole numbers stay)", numToStr(120) == "120", numToStr(120));
}

// ======================================================= SINGLY LINKED LIST
void testSinglyList() {
    cout << "\n--- SinglyLinkedList ---\n";
    SinglyLinkedList<int> list;
    check("new list is empty", list.isEmpty() && list.size() == 0 && list.getHead() == nullptr);

    list.pushBack(2);
    list.pushBack(3);
    list.pushFront(1);
    check("pushFront / pushBack order is 1,2,3", listToText(list) == "1,2,3", listToText(list));
    check("size is 3", list.size() == 3);
    check("get(0), get(1), get(2)", list.get(0) == 1 && list.get(1) == 2 && list.get(2) == 3);
    check("get with a bad index returns T() without crashing", list.get(7) == 0 && list.get(-1) == 0);

    list.reverse();
    check("reverse gives 3,2,1", listToText(list) == "3,2,1", listToText(list));
    list.pushBack(0);
    check("pushBack still works after reverse (tail was updated)",
          listToText(list) == "3,2,1,0", listToText(list));

    check("popFront returns the first value", list.popFront() == 3);
    check("list after popFront is 2,1,0", listToText(list) == "2,1,0", listToText(list));

    // Rule of Three: copy constructor must make independent nodes.
    SinglyLinkedList<int> copy(list);
    copy.pushBack(99);
    copy.popFront();
    check("copy constructor: original unchanged after changing the copy",
          listToText(list) == "2,1,0" && listToText(copy) == "1,0,99",
          listToText(list) + " | " + listToText(copy));

    // Rule of Three: copy assignment.
    SinglyLinkedList<int> assigned;
    assigned.pushBack(7);
    assigned = list;
    assigned.pushFront(-5);
    check("copy assignment: original unchanged, old contents replaced",
          listToText(list) == "2,1,0" && listToText(assigned) == "-5,2,1,0",
          listToText(list) + " | " + listToText(assigned));
    assigned = assigned;                       // self-assignment must be harmless
    check("self-assignment keeps the data", listToText(assigned) == "-5,2,1,0", listToText(assigned));

    // Draining the list, then popping an empty list.
    while (!list.isEmpty()) list.popFront();
    check("list is empty after popping everything", list.isEmpty() && list.getHead() == nullptr);
    check("popFront on empty list returns T() without crashing", list.popFront() == 0);
    list.pushBack(5);
    check("empty list works again after being drained", listToText(list) == "5", listToText(list));

    list.clear();
    check("clear empties the list", list.isEmpty() && list.size() == 0);

    // Works with strings too (Part A / Part B use other types).
    SinglyLinkedList<string> names;
    names.pushBack("Saddar");
    names.pushBack("Clifton");
    check("list of strings", names.get(1) == "Clifton" && names.size() == 2);
}

// ======================================================= DOUBLY LINKED LIST
void testDoublyList() {
    cout << "\n--- DoublyLinkedList ---\n";
    DoublyLinkedList<int> list;
    check("new list is empty", list.isEmpty() && list.getHead() == nullptr && list.getTail() == nullptr);

    list.pushBack(2);
    list.pushBack(3);
    list.pushFront(1);
    check("forward order is 1,2,3", dlistForward(list) == "1,2,3", dlistForward(list));
    check("backward order is 3,2,1 (prev pointers correct)", dlistBackward(list) == "3,2,1", dlistBackward(list));
    check("head and tail are 1 and 3", list.getHead()->data == 1 && list.getTail()->data == 3);

    check("popFront returns 1", list.popFront() == 1);
    check("popBack returns 3", list.popBack() == 3);
    check("one item left: 2, head == tail",
          dlistForward(list) == "2" && list.getHead() == list.getTail() && list.size() == 1);
    check("popBack of last item returns 2 and empties the list",
          list.popBack() == 2 && list.isEmpty() && list.getHead() == nullptr && list.getTail() == nullptr);
    check("popFront / popBack on empty list return T()", list.popFront() == 0 && list.popBack() == 0);

    // Rule of Three.
    for (int i = 1; i <= 4; i++) list.pushBack(i);
    DoublyLinkedList<int> copy(list);
    copy.pushBack(50);
    copy.popFront();
    check("copy constructor: original unchanged after changing the copy",
          dlistForward(list) == "1,2,3,4" && dlistForward(copy) == "2,3,4,50",
          dlistForward(list) + " | " + dlistForward(copy));
    check("copy has correct prev pointers", dlistBackward(copy) == "50,4,3,2", dlistBackward(copy));

    DoublyLinkedList<int> assigned;
    assigned.pushBack(9);
    assigned = list;
    assigned.popBack();
    check("copy assignment: original unchanged",
          dlistForward(list) == "1,2,3,4" && dlistForward(assigned) == "1,2,3",
          dlistForward(list) + " | " + dlistForward(assigned));
    assigned = assigned;
    check("self-assignment keeps the data", dlistForward(assigned) == "1,2,3", dlistForward(assigned));

    // TripHistory pattern: keep at most N entries, drop oldest from the front.
    DoublyLinkedList<string> history;
    for (int i = 1; i <= 60; i++) {
        history.pushBack("trip " + to_string(i));
        while (history.size() > 50) history.popFront();
    }
    check("history pattern: 50 entries, oldest is trip 11, newest is trip 60",
          history.size() == 50 && history.getHead()->data == "trip 11" && history.getTail()->data == "trip 60");

    list.clear();
    check("clear empties the list", list.isEmpty() && list.getTail() == nullptr);
}

// ================================================================== STACK
void testStack() {
    cout << "\n--- Stack ---\n";
    Stack<int> s;
    check("new stack is empty", s.isEmpty() && s.size() == 0);
    s.push(1);
    s.push(2);
    s.push(3);
    check("top is 3, size is 3", s.top() == 3 && s.size() == 3);
    s.top() = 30;                              // top() returns a reference
    check("top() reference can modify the top item", s.top() == 30);
    check("pop order is last in, first out", s.pop() == 30 && s.pop() == 2 && s.pop() == 1);
    check("stack is empty again", s.isEmpty());
    check("pop on empty stack returns T() without crashing", s.pop() == 0);
    check("top on empty stack returns T() without crashing", s.top() == 0);

    // Growth: far more than the starting capacity.
    for (int i = 0; i < 1000; i++) s.push(i);
    bool order = true;
    for (int i = 999; i >= 0; i--) if (s.pop() != i) order = false;
    check("1000 pushes (array grows) then 1000 pops in reverse order", order && s.isEmpty());

    // Rule of Three.
    Stack<int> a;
    for (int i = 1; i <= 40; i++) a.push(i);   // 40 > 16, so the copy must handle a grown array
    Stack<int> b(a);
    b.pop();
    b.push(777);
    check("copy constructor: original unchanged after changing the copy",
          a.size() == 40 && a.top() == 40 && b.top() == 777);
    Stack<int> c;
    c.push(5);
    c = a;
    c.pop();
    check("copy assignment: original unchanged", a.size() == 40 && a.top() == 40 && c.size() == 39);
    c = c;
    check("self-assignment keeps the data", c.size() == 39 && c.top() == 39);

    s.push(8);
    s.clear();
    check("clear empties the stack", s.isEmpty() && s.size() == 0);

    // Strings: the undo stack of Part B stores whole map snapshots.
    Stack<string> snaps;
    snaps.push("map v1");
    snaps.push("map v2");
    check("Stack<string> works", snaps.pop() == "map v2" && snaps.pop() == "map v1" && snaps.isEmpty());
}

// ================================================================== QUEUE
void testQueue() {
    cout << "\n--- Queue ---\n";
    Queue<int> q;
    check("new queue is empty", q.isEmpty() && q.size() == 0);
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    check("front is 1, size is 3", q.front() == 1 && q.size() == 3);
    check("dequeue order is first in, first out", q.dequeue() == 1 && q.dequeue() == 2 && q.dequeue() == 3);
    check("queue is empty again", q.isEmpty());
    check("dequeue on empty queue returns T() without crashing", q.dequeue() == 0);
    check("front on empty queue returns T() without crashing", q.front() == 0);

    // Wrap-around: the front keeps moving forward past the end of the 16-slot array many times.
    q.clear();
    for (int i = 0; i < 200; i++) {
        q.enqueue(i);
        if (i % 2 == 1) { q.dequeue(); }       // front keeps moving forward, wrapping around
    }
    // 200 enqueues, 100 dequeues: the items 100..199 must remain, in order.
    bool remainOk = (q.size() == 100);
    for (int i = 100; i < 200 && remainOk; i++) {
        if (q.dequeue() != i) remainOk = false;
    }
    check("wrap-around: 200 enqueues with interleaved dequeues keep FIFO order", remainOk && q.isEmpty());

    // Growth while the data is wrapped (front is in the middle of the array).
    Queue<int> w;
    for (int i = 0; i < 12; i++) w.enqueue(i);
    for (int i = 0; i < 10; i++) w.dequeue();           // front is now at index 10
    for (int i = 12; i < 60; i++) w.enqueue(i);          // wraps, then must grow while wrapped
    bool growOk = (w.size() == 50);
    for (int i = 10; i < 60 && growOk; i++) if (w.dequeue() != i) growOk = false;
    check("growing a wrapped queue keeps the order", growOk && w.isEmpty());

    // Rule of Three, including a copy made while the data is wrapped.
    Queue<int> a;
    for (int i = 0; i < 14; i++) a.enqueue(i);
    for (int i = 0; i < 12; i++) a.dequeue();            // front at index 12
    for (int i = 14; i < 24; i++) a.enqueue(i);          // data wraps around the end
    Queue<int> b(a);
    b.dequeue();
    b.enqueue(555);
    check("copy constructor: original unchanged after changing the copy",
          a.size() == 12 && a.front() == 12 && b.front() == 13 && b.size() == 12);
    Queue<int> c;
    c.enqueue(5);
    c = a;
    c.dequeue();
    check("copy assignment: original unchanged", a.size() == 12 && a.front() == 12 && c.size() == 11);
    bool copyOrder = true;
    for (int i = 12; i < 24; i++) if (a.dequeue() != i) copyOrder = false;
    check("original still dequeues 12..23 in order after copying", copyOrder);
    c = c;
    check("self-assignment keeps the data", c.size() == 11 && c.front() == 13);

    // Strings.
    Queue<string> names;
    names.enqueue("Saddar");
    names.enqueue("Clifton");
    check("Queue<string> works", names.dequeue() == "Saddar" && names.front() == "Clifton");
}

// ================================================================ SORTING
void testSorting() {
    cout << "\n--- Sorting ---\n";
    const int N = 1000;
    int original[N];
    for (int i = 0; i < N; i++) original[i] = nextRandom();

    int a[N], b[N];
    for (int i = 0; i < N; i++) { a[i] = original[i]; b[i] = original[i]; }

    mergeSort(a, N, lessInt);
    check("mergeSort sorts 1000 random numbers", isSortedAscending(a, N));
    check("mergeSort keeps every value (same sum)", sumOf(a, N) == sumOf(original, N));

    quickSort(b, N, lessInt);
    check("quickSort sorts 1000 random numbers", isSortedAscending(b, N));
    check("quickSort keeps every value (same sum)", sumOf(b, N) == sumOf(original, N));

    bool same = true;
    for (int i = 0; i < N; i++) if (a[i] != b[i]) same = false;
    check("mergeSort and quickSort give the same result", same);

    // Descending order through the comparator.
    for (int i = 0; i < N; i++) { a[i] = original[i]; b[i] = original[i]; }
    mergeSort(a, N, greaterInt);
    quickSort(b, N, greaterInt);
    bool desc = true;
    for (int i = 1; i < N; i++) if (a[i - 1] < a[i] || b[i - 1] < b[i]) desc = false;
    check("both sorts can sort descending via the comparator", desc);

    // Special inputs.
    int empty[1];
    mergeSort(empty, 0, lessInt);
    quickSort(empty, 0, lessInt);
    check("n = 0 does nothing and does not crash", true);
    int single[1] = {42};
    mergeSort(single, 1, lessInt);
    quickSort(single, 1, lessInt);
    check("n = 1 stays unchanged", single[0] == 42);

    int two[2] = {9, 3};
    quickSort(two, 2, lessInt);
    check("quickSort on 2 items", two[0] == 3 && two[1] == 9);

    int sortedIn[500], reverseIn[500], equalIn[500], sortedIn2[500], reverseIn2[500];
    for (int i = 0; i < 500; i++) {
        sortedIn[i] = sortedIn2[i] = i;
        reverseIn[i] = reverseIn2[i] = 499 - i;
        equalIn[i] = 7;
    }
    quickSort(sortedIn, 500, lessInt);
    quickSort(reverseIn, 500, lessInt);
    quickSort(equalIn, 500, lessInt);
    mergeSort(sortedIn2, 500, lessInt);
    mergeSort(reverseIn2, 500, lessInt);
    bool equalOk = true;
    for (int i = 0; i < 500; i++) if (equalIn[i] != 7) equalOk = false;
    check("quickSort on already sorted input", isSortedAscending(sortedIn, 500));
    check("quickSort on reverse sorted input", isSortedAscending(reverseIn, 500) && reverseIn[0] == 0);
    check("quickSort on all equal values", equalOk);
    check("mergeSort on sorted and reverse sorted input",
          isSortedAscending(sortedIn2, 500) && isSortedAscending(reverseIn2, 500));

    // Many duplicates (like roads with the same distance).
    int dup[N];
    for (int i = 0; i < N; i++) dup[i] = nextRandom() % 5;
    long long dupSum = sumOf(dup, N);
    quickSort(dup, N, lessInt);
    check("quickSort with many duplicate values", isSortedAscending(dup, N) && sumOf(dup, N) == dupSum);

    // Sorting a struct array, like Part A's routes and Part B's roads.
    Item items[N];
    for (int i = 0; i < N; i++) { items[i].key = nextRandom() % 10; items[i].order = i; }
    mergeSort(items, N, lessKey);
    bool keysOk = true, stable = true;
    for (int i = 1; i < N; i++) {
        if (items[i - 1].key > items[i].key) keysOk = false;
        if (items[i - 1].key == items[i].key && items[i - 1].order > items[i].order) stable = false;
    }
    check("mergeSort on structs sorts by key", keysOk);
    check("mergeSort is stable (equal keys keep their original order)", stable);

    // Strings with a comparator that is a plain lambda-free function.
    string names[4] = {"Saddar", "Clifton", "DHA", "Airport"};
    mergeSort(names, 4, static_cast<bool (*)(const string&, const string&)>(
        [](const string& x, const string& y) { return x < y; }));
    check("mergeSort on strings", names[0] == "Airport" && names[1] == "Clifton" &&
                                  names[2] == "DHA" && names[3] == "Saddar");
}

int main() {
    testJson();
    testSinglyList();
    testDoublyList();
    testStack();
    testQueue();
    testSorting();

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    if (failed == 0) {
        return 0;
    }
    return 1;
}
