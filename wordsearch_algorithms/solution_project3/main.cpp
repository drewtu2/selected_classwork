// Solutions to project 3

#include <iostream>
using namespace std;

#include <string>
#include <vector>

int binarySearch(string &target, vector<string> &list)
// Perform a binary search for target within the sorted list.  Returns
// the index of the target if found, and returns -1 otherwise.
{
   int left = 0;
   int right = list.size()-1;
   int mid = 0;
   
   while (left <= right)
   {
      mid = (int) (left+right)/2;

      if (target == list[mid])
	 return mid;
      else
	 if (target < list[mid])
	    right = mid-1;
         else
	    left = mid+1;
   }

   return -1;
}

#include <fstream>
#include <list>
#include "d_matrix.h"
#include "wordlist.h"
#include "grid.h"
#include "hashtable.h"
#include <time.h>

#define MaxString 15

// Constants that are used to represent search directions

#define Right 0
#define UpperRight 1
#define Up 2
#define UpperLeft 3
#define Left 4
#define LowerLeft 5
#define Down 6
#define LowerRight 7

void scramble(vector<string> &wordlist)
// Randomly scrambles the words in wordlist.  Selects two random words
// interchanges them, and repeats a large number of times.
{
   string temp;
   int j, k;

   for (int i = 0; i < wordlist.size(); i++)
   {
      j = random() % wordlist.size();
      k = random() % wordlist.size();
      temp = wordlist[j];
      wordlist[j] = wordlist[k];
      wordlist[k] = temp;
   }
}

template <typename T>
void match (T &d, grid &g, int startI, int startJ, int num, int D)
// If the characters in the matrix starting at position (startI, startJ) 
// and continuing for num characters in direction D match a word in the 
// wordlist, print that word out.
{
   string s(num,' ');
   int updateI, updateJ;
   
   // Depending on the value of D, set updateI and updateJ to the
   // appropriate values to search the character array.
   switch(D)
   {
      case(Right):
	 updateI = 0; updateJ = +1;
	 break;
      case(UpperRight):
	 updateI = +1; updateJ = +1;
	 break;
      case(Up):
	 updateI = +1; updateJ = 0;
	 break;
      case(UpperLeft):
	 updateI = +1; updateJ = -1;
	 break;
      case(Left):
	 updateI = 0; updateJ = -1;
	 break;
      case(LowerLeft):
	 updateI = -1; updateJ = -1;
	 break;
      case(Down):
	 updateI = -1; updateJ = 0;
	 break;
      case(LowerRight):
	 updateI = -1; updateJ = +1;
	 break;
   }

   int i = startI;
   int j = startJ;
   
   for (int k = 0; k < num; k++)
   {
      // Build up a string s containing consecutive characters from
      // the matrix in some direction.
      
      s[k] = g.getChar(i,j);

      // Update the i and j indices in the appropriate way, depending
      // on the direction.
      
      i = (i + updateI);
      j = (j + updateJ); 

      // Wrap around at the edge of the matrix.
      
      if (i == -1)
	 i = g.getRows()-1;
      else
	 if (i == g.getRows())
	    i = 0;

      if (j == -1)
	 j = g.getCols()-1;
      else
	 if (j == g.getCols())
	    j = 0;
}

   // If the string s is in the wordlist, print it out.
   
   //if (d.inlist(s) && s.size() > 4)
     // cout << "found: " << s << endl;
}

template <typename T>
void findMatches(T &d, grid &g)
// Print all strings in grid g that are in the wordlist d.
{
   // Search for words of length len starting at each position
   // in the character array.
   
   for (int i = 0; i < g.getRows(); i++)
   {
      for (int j = 0; j < g.getCols(); j++)
      {
         for (int len = 1; len < MaxString; len++)
         {
            match<T>(d, g, i, j, len, Right);
            match<T>(d, g, i, j, len, UpperRight);
            match<T>(d, g, i, j, len, Up);
            match<T>(d, g, i, j, len, UpperLeft);
            match<T>(d, g, i, j, len, Left);
            match<T>(d, g, i, j, len, LowerLeft);
            match<T>(d, g, i, j, len, Down);
            match<T>(d, g, i, j, len, LowerRight);
         }
      }
   }
}

int main()
{
   clock_t start,end;
   start = clock();
  
  /* 
   hashtable<string> d;
   grid g;
   findMatches(d,g);
*/


   wordlist d;
   d.sort();
   //d.print();
   grid g;
   findMatches(d,g);

   end=clock();
   cout << "CPU time (CPU seconds)" << ((double)(end-start))/CLOCKS_PER_SEC << endl;

}
