#pragma once
//  MERGE SORT
template<typename T>
void mergeRanges(T arr[], T tmp[], int lo, int mid, int hi,
                 bool (*lessThan)(const T&, const T&)) {
    int i = lo;      
    int j = mid;     
    int k = lo;     
    while (i < mid && j < hi) {
        if (lessThan(arr[j], arr[i])) tmp[k++] = arr[j++];
        else                          tmp[k++] = arr[i++];
    }
    while (i < mid) tmp[k++] = arr[i++];       
    while (j < hi)  tmp[k++] = arr[j++];      
    for (int p = lo; p < hi; p++) arr[p] = tmp[p];   
}

template<typename T>
void mergeSortRange(T arr[], T tmp[], int lo, int hi,
                    bool (*lessThan)(const T&, const T&)) {
    if (hi - lo <= 1) return;                 
    int mid = lo + (hi - lo) / 2;
    mergeSortRange(arr, tmp, lo, mid, lessThan);
    mergeSortRange(arr, tmp, mid, hi, lessThan);
    mergeRanges(arr, tmp, lo, mid, hi, lessThan);
}

template<typename T>
void mergeSort(T arr[], int n, bool (*lessThan)(const T&, const T&)) {
    if (arr == nullptr || n <= 1) return;
    T* tmp = new T[n];                         
    mergeSortRange(arr, tmp, 0, n, lessThan);
    delete[] tmp;
}

//  QUICK SORT

template<typename T>
void sortSwap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

template<typename T>
T medianOfThree(const T& a, const T& b, const T& c,
                bool (*lessThan)(const T&, const T&)) {
    if (lessThan(a, b)) {
        if (lessThan(b, c)) return b;         
        if (lessThan(a, c)) return c;          
        return a;                            
    }
    if (lessThan(a, c)) return a;             
    if (lessThan(b, c)) return c;             
    return b;                                  
}

template<typename T>
void quickSortRange(T arr[], int lo, int hi, bool (*lessThan)(const T&, const T&)) {
    while (lo < hi) {
        T pivot = medianOfThree(arr[lo], arr[lo + (hi - lo) / 2], arr[hi], lessThan);

        int i = lo;
        int j = hi;
        while (i <= j) {
            while (lessThan(arr[i], pivot)) i++;      
            while (lessThan(pivot, arr[j])) j--;     
            if (i <= j) {
                sortSwap(arr[i], arr[j]);
                i++;
                j--;
            }
        }

        if (j - lo < hi - i) {
            quickSortRange(arr, lo, j, lessThan);
            lo = i;
        } else {
            quickSortRange(arr, i, hi, lessThan);
            hi = j;
        }
    }
}

template<typename T>
void quickSort(T arr[], int n, bool (*lessThan)(const T&, const T&)) {
    if (arr == nullptr || n <= 1) return;
    quickSortRange(arr, 0, n - 1, lessThan);
}
