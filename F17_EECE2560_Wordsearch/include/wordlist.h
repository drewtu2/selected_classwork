#ifndef WORDLIST_H
#define WORDLIST_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "heap.hpp"

class wordList
{
// string boolean yes or no if the part of the word binary search for this 
  private:
    std::vector<std::string> word_list;
    std::vector<std::string> sorted_word_list;   

    const static int lower_bound =0;
    const static int upper_bound =-1;
    const static int first_run =0;
  public: 
   wordList();
   wordList(std::string fileName);
   void insertionSort();
   void merge(int start, int middle, int end);
   void heapSort();
   void quickSort(int left = lower_bound, int right = upper_bound);
   void stdSort();
   void mergeSort(int l = lower_bound, int r= upper_bound);
   int wordExists(std::string test, 
                   int first=lower_bound, 
                   int last=upper_bound, 
                   int first_round = first_run);
   bool wordLookup(std::string test, 
                   int first=lower_bound, 
                   int last=upper_bound, 
                   int first_round = first_run);
   std::vector<std::string> getVector();
   void setVector(std::vector<std::string> word_list);  

   int getSize();
   std::string get(int i);
   friend std::ostream &operator << (std::ostream &os, const wordList &w);

};

#endif
