/*
 * =============================================================================
 * File    : Sorting.h
 * Part    : C (Foundations, Interface, Web)
 * Purpose : Two sorting algorithms written from scratch (no std::sort):
 *             mergeSort - Part A uses it to rank the alternate routes
 *             quickSort - Part B uses it to sort roads by distance for Kruskal
 * Course  : Lab 7 - merge sort and quick sort.  Lab 6 - recursion.
 *
 * COMPARATOR
 *   Both take a function pointer  bool lessThan(const T& a, const T& b)
 *   that returns true when a must come BEFORE b. This lets the caller sort the
 *   same data by distance, by time, by number of roads, and so on.
 *
 * MERGE SORT (stable, always O(n log n))
 *   Split the array in two halves, sort each half recursively, then MERGE the
 *   two sorted halves into a temporary array and copy it back.
 *   STABLE = items that compare equal keep their original order. That is why
 *   when merging we take from the left half unless the right item is strictly
 *   smaller. Extra memory: O(n) for the temporary array.
 *
 * QUICK SORT (O(n log n) on average, O(n^2) worst case, no extra array)
 *   Pick a pivot, rearrange so everything smaller is on its left and
 *   everything larger is on its right, then sort the two parts recursively.
 *   - Pivot = MEDIAN OF THREE (first, middle, last value). This avoids the
 *     worst case on already-sorted or reverse-sorted input.
 *   - The partition scans from both ends and swaps out-of-place pairs
 *     (Hoare style). Elements equal to the pivot are swapped too, so arrays
 *     with many equal values still split evenly.
 *   - We recurse into the SMALLER part and loop on the larger one, so the
 *     recursion depth never exceeds about log2(n).
 *   - Quick sort is NOT stable.
 *
 * Both functions accept n <= 1 (nothing to do) and arr == nullptr safely.
 * The element type T needs a default constructor and copy assignment.
 * =============================================================================
 */
#pragma once

// ----------------------------------------------------------------------------
//  MERGE SORT
// ----------------------------------------------------------------------------

// Merges the sorted ranges arr[lo..mid) and arr[mid..hi) using tmp as scratch space.
// Time: O(hi - lo)
template<typename T>
void mergeRanges(T arr[], T tmp[], int lo, int mid, int hi,
                 bool (*lessThan)(const T&, const T&)) {
    int i = lo;      // next unused item of the left half
    int j = mid;     // next unused item of the right half
    int k = lo;      // next free position in tmp
    while (i < mid && j < hi) {
        // Take from the right ONLY if it is strictly smaller: this keeps equal items in order (stable).
        if (lessThan(arr[j], arr[i])) tmp[k++] = arr[j++];
        else                          tmp[k++] = arr[i++];
    }
    while (i < mid) tmp[k++] = arr[i++];       // leftovers of the left half
    while (j < hi)  tmp[k++] = arr[j++];       // leftovers of the right half
    for (int p = lo; p < hi; p++) arr[p] = tmp[p];   // copy the merged result back
}

// Recursively sorts arr[lo..hi) (hi is exclusive).  Time: O(n log n)
template<typename T>
void mergeSortRange(T arr[], T tmp[], int lo, int hi,
                    bool (*lessThan)(const T&, const T&)) {
    if (hi - lo <= 1) return;                  // 0 or 1 item: already sorted
    int mid = lo + (hi - lo) / 2;
    mergeSortRange(arr, tmp, lo, mid, lessThan);
    mergeSortRange(arr, tmp, mid, hi, lessThan);
    mergeRanges(arr, tmp, lo, mid, hi, lessThan);
}

/*
 * mergeSort: sorts arr[0..n-1] so that lessThan order holds. Stable.
 * Time: O(n log n) always.  Extra memory: O(n).
 */
template<typename T>
void mergeSort(T arr[], int n, bool (*lessThan)(const T&, const T&)) {
    if (arr == nullptr || n <= 1) return;
    T* tmp = new T[n];                         // scratch array shared by all merges
    mergeSortRange(arr, tmp, 0, n, lessThan);
    delete[] tmp;
}

// ----------------------------------------------------------------------------
//  QUICK SORT
// ----------------------------------------------------------------------------

// Swaps two items. (Own function: std::swap is not allowed in this project.)
template<typename T>
void sortSwap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

// Returns a COPY of the median (middle value) of a, b and c.  Time: O(1)
template<typename T>
T medianOfThree(const T& a, const T& b, const T& c,
                bool (*lessThan)(const T&, const T&)) {
    if (lessThan(a, b)) {
        if (lessThan(b, c)) return b;          // a < b < c
        if (lessThan(a, c)) return c;          // a < c <= b
        return a;                              // c <= a < b
    }
    if (lessThan(a, c)) return a;              // b <= a < c
    if (lessThan(b, c)) return c;              // b < c <= a
    return b;                                  // c <= b <= a
}

// Sorts arr[lo..hi] (both inclusive).  Time: O(n log n) average.
template<typename T>
void quickSortRange(T arr[], int lo, int hi, bool (*lessThan)(const T&, const T&)) {
    while (lo < hi) {
        // Pivot VALUE (a copy, so swaps cannot move it away from us).
        T pivot = medianOfThree(arr[lo], arr[lo + (hi - lo) / 2], arr[hi], lessThan);

        // Partition: i walks right, j walks left, out-of-place pairs are swapped.
        int i = lo;
        int j = hi;
        while (i <= j) {
            while (lessThan(arr[i], pivot)) i++;      // stops at an item that is NOT smaller than the pivot
            while (lessThan(pivot, arr[j])) j--;      // stops at an item that is NOT bigger than the pivot
            if (i <= j) {
                sortSwap(arr[i], arr[j]);
                i++;
                j--;
            }
        }
        // Now arr[lo..j] <= pivot <= arr[i..hi]. Recurse into the SMALLER part,
        // loop on the larger one: keeps the recursion depth about log2(n).
        if (j - lo < hi - i) {
            quickSortRange(arr, lo, j, lessThan);
            lo = i;
        } else {
            quickSortRange(arr, i, hi, lessThan);
            hi = j;
        }
    }
}

/*
 * quickSort: sorts arr[0..n-1] so that lessThan order holds. Not stable.
 * Time: O(n log n) average, O(n^2) worst case (very unlikely with median of three).
 * Extra memory: O(log n) for the recursion.
 */
template<typename T>
void quickSort(T arr[], int n, bool (*lessThan)(const T&, const T&)) {
    if (arr == nullptr || n <= 1) return;
    quickSortRange(arr, 0, n - 1, lessThan);
}
