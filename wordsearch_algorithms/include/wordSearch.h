#ifndef WORDSEARCH_H
#define WORDSEARCH_H

#include <chrono>
#include <vector>
#include <string>

#include "wordlist.h"
#include "grid.h"
#include "hashTable.hpp"


struct timingData{
  std::chrono::duration<double> sortTime;
  std::chrono::duration<double> searchTime;
  std::chrono::duration<double> totalTime;

};

struct array_coord{
  int xx;
  int yy;
};

void print_timing(timingData* myTime);

class WordSearch
{

 public:
  WordSearch();
  WordSearch(std::string gridFile, std::string wordFile, int sort);
  ~WordSearch();

  void init();
  void findMatches();
  void search();
  void print_results();
  timingData getTiming();
  


 private:

  timingData runTiming;
  int sortMechanism;

  std::vector<std::string> wordsFound;
  wordList* givenWords;
  grid* givenGrid;
  hashTable<std::string>* myHash;


  std::vector<std::string> checkWord8Dim(int ii, int jj);
  std::vector<std::string> checkWord8DimSort(int ii, int jj);
  std::vector<std::string> checkWord1DimSort(int ii, int jj, int direction);
  std::vector<std::string> checkWord8DimHash(int ii, int jj);
  std::vector<std::string> checkWord1DimHash(int ii, int jj, int direction);
  array_coord findCoord(int ii, int jj, int direction, int offset); 
  


};




#endif
