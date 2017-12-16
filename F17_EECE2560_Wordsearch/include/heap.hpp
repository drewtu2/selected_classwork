#ifndef Heap_h
#define Heap_h

//#define LargeValue 999999999
//#define SmallValue -999999999
#include <iostream>
#include "d_except.h"
#include <vector>
#include <cstdlib>
#include <algorithm>

template <typename T, typename U>
class heapV
{
public:
   heapV();
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
   int size() {return (int) list.size();}
   
   T getMaxHeapMaximum();
   T getMinHeapMinimum();
   
   T extractMaxHeapMaximum( );
   T extractMinHeapMinimum( );
   
   void maxHeapIncreaseKey(int i);
   void minHeapDecreaseKey(int i);
   
   void maxHeapInsert(T key);
   void minHeapInsert(T key);
   
private:
   vector<T> list;
};

template <typename T, typename U>
int heapV<T,U>::getIndex(T &key)
// Return the index of the item key in the heap.  Assumes each item is unique.
{
   for (int i = 0; i < size(); i++)
      if (getItem(i) == key)
         return i;
   throw rangeError("key not found in heap::getIndex");
}

template <typename T, typename U>
heapV<T,U>::heapV()
// Construct an empty heap.
{
}

template <typename T, typename U>
void heapV<T,U>::initializeMaxHeap(vector<T> list2)
// Initialize a max heap from values in list2.
{
   for (int i = 0; i < list2.size(); i++)
      list.push_back(list2[i]);
   
   buildMaxHeap(size());
}

template <typename T, typename U>
void heapV<T,U>::initializeMinHeap(vector<T> list2)
// Initialize a min heap from values in list2.
{
   for (int i = 0; i < list2.size(); i++)
      list.push_back(list2[i]);
   
   buildMinHeap(size());
}


template <typename T, typename U>
void heapV<T,U>::maxHeapify(int i, int heapSize)
// Assumes the binary trees rooted at left(i) and right(i) are max heaps, but node i
// might be smaller than its children, violating the max-heap property.  The function
// lets the value at i float down so the subtree rooted at i obeys the max heap property.
{
   int l = left(i);
   int r = right(i);
   int largest;
   
   // Set largest = the largest of the two children and i (the root)
   
   if (l <= heapSize-1 && list[l].compare( list[i])>0)
      largest = l;
   else
      largest = i;
   
   if (r <= heapSize-1 && list[r].compare( list[largest])>0)
      largest = r;
   
   // If the root isn't the largest, swap the root with the largest child and recurse.
   if (largest != i)
   {
      swap(list[i],list[largest]);
      maxHeapify(largest, heapSize);
   }
}

template <typename T, typename U>
void heapV<T,U>::minHeapify(int i, int heapSize)
// Assumes the binary trees rooted at left(i) and right(i) are min heaps, but node i
// might be larger than its children, violating the min-heap property.  The function
// lets the value at i float down so the subtree rooted at i obeys the min heap property.
{
   int l = left(i);
   int r = right(i);
   int smallest;
   
   // Set smallest = the smallest of the two children and i (the root)
   
   if (l <= heapSize-1 && list[l].weight < list[i].weight)
      smallest = l;
   else
      smallest = i;
   
   if (r <= heapSize-1 && list[r].weight < list[smallest].weight)
      smallest = r;
   
   // If the root isn't the smallest, swap the root with the smallest child and recurse.
   if (smallest != i)
   {
      //cout << "Swapping " << list[i] << " and " << list[smallest] << endl;
      swap(list[i],list[smallest]);
      minHeapify(smallest, heapSize);
   }
}

template <typename T, typename U>
void heapV<T,U>::buildMaxHeap(int heapSize)
// Transforms an unordered list into a heap bottom-up.  The second half of a list
// represents leaves of the tree, so each element represents a one-element heap.
// This function builds a heap by running minHeapify on the first non-leaf, and
// works up to the first element.  Max indices from starting at 0 to 1.
{
   for (int i = (size()+1)/2; i >= 0; i--)
      maxHeapify(i, heapSize );
}

template <typename T, typename U>
void heapV<T,U>::buildMinHeap(int heapSize)
// Transforms an unordered list into a heap bottom-up.  The second half of a list
// represents leaves of the tree, so each element represents a one-element heap.
// This function builds a heap by running minHeapify on the first non-leaf, and
// works up to the first element.  Max indices from starting at 0 to 1.
{
   for (int i = (size()+1)/2; i >= 0; i--)
      minHeapify(i, heapSize);
}

template <typename T, typename U>
T heapV<T,U>::getMinHeapMinimum()
// Return the smallest item in a min heap.
{
   return list[0];
}

template <typename T, typename U>
T heapV<T,U>::getMaxHeapMaximum()
// Return the largest item in a max heap
{
   return list[0];
}

template <typename T, typename U>
T heapV<T,U>::extractMinHeapMinimum()
// Return the smallest item in a min heap and remove it.
{
   if (size() < 1)
      throw rangeError("heap too small in extractMinHeapMinimum");
   T min = list[0];
   list[0] = list[size()-1];
   //cout << "Swapping " << list[0] << " to the top" << endl;
   list.resize(size()-1);
   minHeapify(0,size());
   return min;
}

template <typename T, typename U>
T heapV<T,U>::extractMaxHeapMaximum()
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

template <typename T, typename U>
void heapV<T,U>::maxHeapIncreaseKey(int i)
// Assumes the item at position i has had its value increased.  Rebuilds the max-heap by performing
// an insertion sort that finds the new item's correct location in the list.
{
   while (i > 0 && list[parent(i)].compare( list[i])>0)
   {
      swap(list[i], list[parent(i)]);
      i = parent(i);
   }
}

template <typename T, typename U>
void heapV<T,U>::minHeapDecreaseKey(int i)
// Assumes the item at position i has had its value decreased.  Rebuilds the min-heap by performing
// an insertion sort that finds the new item's correct location in the list.
{
   // cout << i << " " << list[i] << " " << list[parent(i)] << endl;

   while (i > 0 && list[parent(i)].compare( list[i])<0)
{   
      swap(list[i], list[parent(i)]);
      i = parent(i);
   }
}

template <typename T, typename U>
void heapV<T,U>::maxHeapInsert(T key)
// Adds a new leaf then calls maxHeapIncreaseKey to move the key to the correct location.
{
   list.push_back(key);
   maxHeapIncreaseKey(size()-1);
}

template <typename T, typename U>
void heapV<T,U>::minHeapInsert(T key)
// Adds a new leaf then calls minHeapDecreaseKey to move the key to the correct location.
{
   list.push_back(key);
   minHeapDecreaseKey(size()-1);
}


template <typename T, typename U>
   void heapV<T,U>::heapsort(int heapSize)
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

template <typename T, typename U>
ostream &operator<<(ostream &ostr, heapV<T,U> &h)
// Print all information about the graph.
{
   std::cout << "------------------------------------------------" << endl;
   for (int i = 0; i < h.size(); i++)
      std::cout << h.getItem(i);

   std::cout << endl;
   
   return ostr;
}
#endif



/*#ifndef HEAP_HPP
#define HEAP_HPP

#include <string>
#include <algorithm>

template<class T>
class heap{

 public:

  heap();
  heap(str::string fileName);
  ~heap();

  int parent(int index);
  int left(int index);
  int right(int index);
  void initializeMaxHeap();
  void maxHeapify(i);
  void buildMaxHeap();

  void initializeMinHeap();
  void minHeapify();
  void buildMinHeap();
  int size(){return elements.size()};
  void heapSort(int heapSize);
  
  std::Vector<T> getContents();
  
 private:
  T getItem(int index);
//  int parent(int index);
//  int left(int index);
//  int right(int index);
  std::vector<T> contents;

}
heap::heap(T &elements)
{
  for(int i=0; i<word_list.size(); i++)
  contents.push_back(elements[i]);

  buildMaxHeap(words, size());
}
void heap::initializeMaxHeap(vector<T> list)
{
  for(int i=0; i<list.size(); i++)
  contents.push_back(list.at(i)):
  buildMaxHeap(size());
}
std::vector<T> getContents()
{
  return element;
}

void heap::buildMaxHeap(int heapSize)
{
  for(int i = contents.size()/2; i > =0; i--)
  {
    maxHeapify(i, heapSize);
  }

}

void heap::maxHeapify(int i, int heapSize)
{
  T = temp;
  int largest;
  int l = left(i);
  int r = right(i);

  if(l <= contents.size-1 && (contents[l]> contents[i]) )
  {
    largest = l;
  }
  else
  largest = i;
  if( r<= contents.size-1 && contents[r]> contents[i] )
      largest =r;
  if(largest !=i)
  {
      temp = contents[i];
      contents[i] = contents[largest];
      contents[largest] = temp;
      maxHeapify(largest, heapSize);
  }
}

// Returns the item at a given index
T getItem(int index)
{
  return content[index];
}

// Returns the item of the parent index
int heap::parent(int index);
{
  return getItem(index/2);
}

// Returns the item of the left child of a given index
int heap::left(int index);
{
  return getItem(2*index);
}

// Returns the item of the right child of a given index
int heap::right(int index);
{
  return getItem(2*index + 1);
}


#endif*/

