// Heap.h header file
//
// Implements min and max heaps and priority queue

#ifndef Heap_h
#define Heap_h

#define LargeValue 999999999
#define SmallValue -999999999

#include "d_except.h"
#include <vector>

template <typename T>
class heap
{
public:
   heap();
   void initializeMaxHeap(vector<T> list2);
   void initializeMinHeap(vector<T> list2);
   
   void maxHeapify(int i, int heapSize);
   void buildMaxHeap(int heapSize);
   
   void minHeapify(int i, int heapSize);
   void buildMinHeap(int heapSize);
   
   void heapsort(int heapSize);
   
   int parent(int i) {return (i+1)/2-1;}  // Given a node in a heap, return the index of the parent
   // Map vertex indices from 0-based to 1-based and
   int left(int i) {return 2*(i+1)-1;}    // Given a node in a heap, return the left child
   // Map vertex indices from 0-based to 1-based and back
   int right(int i) {return 2*(i+1);}     // Given a node in a heap, return the right child
   
   T &getItem(int i) {return list[i];}      // Return a reference to the ith item in the heap
   int getIndex(T &key);
   int size() {return list.size();}
   
   T getMaxHeapMaximum();
   T getMinHeapMinimum();
   
   T extractMaxHeapMaximum();
   T extractMinHeapMinimum();
   
   void maxHeapIncreaseKey(int i);
   void minHeapDecreaseKey(int i);
   
   void maxHeapInsert(T key);
   void minHeapInsert(T key);
   
private:
   vector<T> list;
};

template <typename T>
int heap<T>::getIndex(T &key)
// Return the index of the item key in the heap.  Assumes each item is unique.
{
   for (int i = 0; i < size(); i++)
      if (getItem(i) == key)
         return i;
   throw rangeError("key not found ing heap::getIndex");
}

template <typename T>
heap<T>::heap()
// Construct an empty heap.
{
}

template <typename T>
void heap<T>::initializeMaxHeap(vector<T> list2)
// Initialize a max heap from values in list2.
{
   for (int i = 0; i < list2.size(); i++)
      list.push_back(list2[i]);
   
   buildMaxHeap(size());
    
    /*
     
     template <typename T>
     void heap<T>::buildMaxHeap(int heapSize)
     // Transforms an unordered list into a heap bottom-up.  The second half of a list
     // represents leaves of the tree, so each element represents a one-element heap.
     // This function builds a heap by running minHeapify on the first non-leaf, and
     // works up to the first element.  Max indices from starting at 0 to 1.
     {
     for (int i = (size()+1)/2; i >= 0; i--)
     maxHeapify(i, heapSize);
     }
     
     */
    
    
}

template <typename T>
void heap<T>::initializeMinHeap(vector<T> list2)
// Initialize a min heap from values in list2.
{
   for (int i = 0; i < list2.size(); i++)
      list.push_back(list2[i]);
   
   buildMinHeap(size());
    

    
}


template <typename T>
void heap<T>::maxHeapify(int i, int heapSize)
// Assumes the binary trees rooted at left(i) and right(i) are max heaps, but node i
// might be smaller than its children, violating the max-heap property.  The function
// lets the value at i float down so the subtree rooted at i obeys the max heap property.
{
   int l = left(i);
   int r = right(i);
   int largest;
   
   // Set largest = the largest of the two children and i (the root)
   
   if (l <= heapSize-1 && list[l] > list[i])
      largest = l;
   else
      largest = i;
   
   if (r <= heapSize-1 && list[r] > list[largest])
      largest = r;
   
   // If the root isn't the largest, swap the root with the largest child and recurse.
   if (largest != i)
   {
      swap(list[i],list[largest]);
      maxHeapify(largest, heapSize);
   }
}

template <typename T>
void heap<T>::minHeapify(int i, int heapSize)
// Assumes the binary trees rooted at left(i) and right(i) are min heaps, but node i
// might be larger than its children, violating the min-heap property.  The function
// lets the value at i float down so the subtree rooted at i obeys the min heap property.
{
   int l = left(i);
   int r = right(i);
   int smallest;
   
   // Set smallest = the smallest of the two children and i (the root)
   
   if (l <= heapSize-1 && list[l] < list[i])
      smallest = l;
   else
      smallest = i;
   
   if (r <= heapSize-1 && list[r] < list[smallest])
      smallest = r;
   
   // If the root isn't the smallest, swap the root with the smallest child and recurse.
   if (smallest != i)
   {
      swap(list[i],list[smallest]);
      minHeapify(smallest, heapSize);
   }
}

template <typename T>
void heap<T>::buildMaxHeap(int heapSize)
// Transforms an unordered list into a heap bottom-up.  The second half of a list
// represents leaves of the tree, so each element represents a one-element heap.
// This function builds a heap by running minHeapify on the first non-leaf, and
// works up to the first element.  Max indices from starting at 0 to 1.
{
   for (int i = (size()+1)/2; i >= 0; i--)
      maxHeapify(i, heapSize);
}

template <typename T>
void heap<T>::buildMinHeap(int heapSize)
// Transforms an unordered list into a heap bottom-up.  The second half of a list
// represents leaves of the tree, so each element represents a one-element heap.
// This function builds a heap by running minHeapify on the first non-leaf, and
// works up to the first element.  Max indices from starting at 0 to 1.
{
   for (int i = (size()+1)/2; i >= 0; i--)
      minHeapify(i, heapSize);
}

template <typename T>
T heap<T>::getMinHeapMinimum()
// Return the smallest item in a min heap.
{
   return list[0];
}

template <typename T>
T heap<T>::getMaxHeapMaximum()
// Return the largest item in a max heap
{
   return list[0];
}

template <typename T>
T heap<T>::extractMinHeapMinimum()
// Return the smallest item in a min heap and remove it.
{
   if (size() < 1)
      throw rangeError("heap too small in extractMinHeapMinimum");
   T min = list[0];
   list[0] = list[size()-1];
   list.resize(size()-1);
   minHeapify(0,size());
   return min;
}

template <typename T>
T heap<T>::extractMaxHeapMaximum()
// Return the largest item in a max heap and remove it.
{
   if (size() < 1)
      throw rangeError("heap too small in extractMaxHeapMaximum");
   T max = list[0];
   list[0] = list[size()-1];
   list.resize(size()-1);
   maxHeapify(0,size());
   return max;
}

template <typename T>
void heap<T>::maxHeapIncreaseKey(int i)
// Assumes the item at position i has had its value increased.  Rebuilds the max-heap by performing
// an insertion sort that finds the new item's correct location in the list.
{
   while (i > 0 && list[parent(i)] < list[i])
   {
      swap(list[i], list[parent(i)]);
      i = parent(i);
   }
}

template <typename T>
void heap<T>::minHeapDecreaseKey(int i)
// Assumes the item at position i has had its value decreased.  Rebuilds the min-heap by performing
// an insertion sort that finds the new item's correct location in the list.
{
   // cout << i << " " << list[i] << " " << list[parent(i)] << endl;
   
   while (i > 0 && list[parent(i)] > list[i])
   {
      swap(list[i], list[parent(i)]);
      i = parent(i);
   }
}

template <typename T>
void heap<T>::maxHeapInsert(T key)
// Adds a new leaf then calls maxHeapIncreaseKey to move the key to the correct location.
{
   list.push_back(key);
   maxHeapIncreaseKey(size()-1);
}

template <typename T>
void heap<T>::minHeapInsert(T key)
// Adds a new leaf then calls minHeapDecreaseKey to move the key to the correct location.
{
   list.push_back(key);
   minHeapDecreaseKey(size()-1);
}


template <typename T>
void heap<T>::heapsort(int heapSize)
// Given an unordered list, first builds a max-heap.  The maximum
// element is now in the first position, and it is swapped into the
// last position.  The heap size is reduced by one, which might cause
// element 1 to violate the max-heap property.  The call to maxHeapify
// restores the max-heap property, and the functions repeats.
{
   buildMaxHeap(heapSize);
   
   for (int i = size()-1; i >= 0; i--)
   {
      // cout << "Position: " << i << endl;
      // cout << "Swap" << endl;
      
      swap(list[0],list[i]);
      heapSize--;
      
      // cout << "maxHeapify" << endl;
      maxHeapify(0,heapSize);
   }
}

template <typename T>
ostream &operator<<(ostream &ostr, heap<T> &h)
// Print all information about the graph.
{
   cout << "------------------------------------------------" << endl;
   for (int i = 0; i < h.size(); i++)
      cout << h.getItem(i);

   cout << endl;
   
   return ostr;
}
#endif
