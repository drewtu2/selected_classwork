// Wordlist declarations

//#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include "d_matrix.h"
#include "heap.h"

using namespace std;

#define MaxString 15

// Constants that are used to represent search directions

#define Right 0
#define UpperRight 1
#define Up 2
#define UpperLeft 3
#define Left 4
#define LowerLeft 5
#define Down 6p
#define LowerRight 7

#define SENTINEL_STRING "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
class wordlist
// Class that stores a wordlist of words in a binary search tree.
{
public:
   wordlist();
   ~wordlist();
   bool inlist(string word);
   bool inlist2(string word);
   void sort();
   void print();
   int size() {return words.size();}

   private:
   vector<string> words;

   void selectionsort();
   
   void quicksort2(int left, int right);
   void quicksort();
   
   int partition(int left, int right);
   
   void mergesort();
   void merge(int p, int q, int r);
   void mergesort2(int p, int r);
   void heapsort();
};

wordlist::wordlist()
// Constructor that inserts all the words in wordlist into the
// wordlist binary search tree.
{
   string newWord;
   ifstream fin;
   
/*   words.push_back("8");
   words.push_back("6");
   words.push_back("1");
   words.push_back("2");
   words.push_back("5");
   words.push_back("3");
   words.push_back("7");
   words.push_back("4");
   words.push_back("0");
   words.push_back("9");
   
   return;
*/
   
   // Open the wordlist file.
   string fileName = "../data/wordlist2.txt";

   fin.open(fileName.c_str());
   if (!fin)
   {	
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }

   while (fin)
   {
      fin >> newWord;
      if (fin)
         words.push_back(newWord);
   }

   fin.close();
}

wordlist::~wordlist()
{
}

bool wordlist::inlist2(string word)
// Returns true if word is in the wordlist.
{
   for (int i = 0; i < words.size(); i++)
      if (words[i] == word)
	 return true;

   return false;
}

bool wordlist::inlist(string target)
// Returns true if target is in the wordlist.  Assumes word list is
// sorted, and uses binary search.
{
   if (binarySearch(target, words) != -1)
      return true;
   else
      return false;
}

void wordlist::print()
{
   for (int i = 0; i < words.size(); i++)
      cout << words[i] << endl;

   cout << "-----------" << endl;
}

int wordlist::partition(int left, int right)
{
   string p = words[left];
   int i = left;
   int j = right+1;

   do
   {
      do
      {
	 i = i+1;
      }
      while (words[i] < p && i < right);

      do
      {
	 j = j-1;
      }
      while (words[j] > p && j > left);

      swap(words[i],words[j]);
   } while (i < j);

   swap(words[i],words[j]);
   swap(words[left],words[j]);

   return j;
}

void wordlist::selectionsort()
{
   int min;
   
   for (int i = 0; i < words.size()-1; i++)
   {
      cout << i << endl;
      min = i;
      for (int j = i+1; j < words.size(); j++)
      {
         if (words[j] < words[min])
            min = j;
      }
      
      swap(words[i], words[min]);
   }
}

void wordlist::quicksort2(int left, int right)
{
   int s;
   
   if (left < right)
   {
      s = partition(left,right);
      quicksort2(left,s-1);
      quicksort2(s+1,right);
   }
}

void wordlist::quicksort()
{
   quicksort2(0,size()-1);
}
              
void wordlist::merge(int p, int q, int r)
// Assumes the two sublists [p,q] and [q+1,r] are sorted.  Merges them into a
// single sorted list.
{
   int n1 = q - p + 1;
   int n2 = r - q;
   
   // cout << "Merging " << "[" << p << "," << q << "] and [" << q+1 << "," << r << "]" << endl;
   
   vector<string> L;
   vector<string> R;
   
   for (int i = 0; i < n1; i++)
      L.push_back(words[p+i]);
   
   for (int j = 0; j < n2; j++)
      R.push_back(words[q+j+1]);
   
   L.push_back(SENTINEL_STRING);
   R.push_back(SENTINEL_STRING);
   
   int i = 0;
   int j = 0;
   
   for (int k = p; k <= r; k++)
   {
      if (L[i] <= R[j])
      {
         // cout << "Adding " << L[i] << " " << i << endl;
         words[k] = L[i];
         i = i + 1;
      }
      else
      {
         // cout << "Adding " << R[j] << " " << j << endl;
         words[k] = R[j];
         j = j + 1;
      }
   }
}

void wordlist::mergesort2(int p, int r)
// High level merge function, from CLRS.
{
   int q;
   
   // cout << "Sorting " << p << " to " << r << endl;
   // print();
   
   if (p < r)
   {
      q = (int) (p + r)/2;   // rounded down
      mergesort2(p,q);
      mergesort2(q+1,r);
      merge(p,q,r);
   }
}

void wordlist::mergesort()
{
   mergesort2(0,size()-1);
}

void wordlist::heapsort()
// Sorts items in list by copying the items into a heap, appling a heap sort, and then
// copying the sorted items back into list.
{
   heap<string> h;
   
   h.initializeMaxHeap(words);
   h.heapsort(words.size());
   for (int i = 0; i < words.size(); i++)
      words[i] = h.getItem(i);
}

void wordlist::sort()
{
//   mergesort();
//   quicksort();
//   selectionsort();
   heapsort();

}


