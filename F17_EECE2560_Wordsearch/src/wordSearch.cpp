
#include <string>
#include <iostream>
#include <chrono>
#include <omp.h>

#include "wordSearch.h"
#include "wordlist.h"
#include "constants.h"

using std::chrono::high_resolution_clock;
using std::chrono::steady_clock;

int debug = 1;
void debugPrint(std::string);

void print_timing(timingData* myTime)
{
  std::cout << "Sort Time: " << myTime->sortTime.count() << std::endl;
  std::cout << "Search Time: " << myTime->searchTime.count() << std::endl;
  std::cout << "Total Time: " << myTime->totalTime.count() << std::endl;
}

WordSearch::WordSearch():
    givenWords(NULL),
    givenGrid(NULL),
    myHash(NULL),
    wordsFound(0)
{
  init();
}

WordSearch::WordSearch(std::string gridFile, std::string wordFile, int sort):
    givenWords(NULL),
    givenGrid(NULL),
    myHash(NULL),
    wordsFound(0)
{
    givenGrid = new grid(gridFile);
    givenWords = new wordList(wordFile);
    myHash = new hashTable<std::string>(*givenWords);
    sortMechanism = sort;
}

WordSearch::~WordSearch()
{
    delete givenGrid;
    delete givenWords;
    delete myHash;
}

void WordSearch::init()
{
  std::string fileName;
  std::cout << "Enter name of gridfile: ";
  std::getline(std::cin, fileName);
  //fileName = "data/input15.txt";
  //fileName = "data/input30.txt";
  //fileName = "data/puzzle10.txt";
  //fileName = "data/input250.txt";
  std::cout << "Initializing with file " << fileName << std::endl << std::endl;
  
  givenGrid = new grid(fileName);

  std::string wordListFile;
  std::cout << "Enter name of wordListFile: ";
  std::getline(std::cin, wordListFile);
  //wordListFile = "data/puzzle10-words.txt";
  //wordListFile = "data/wordlist.txt";
  //wordListFile = "data/wordlist2.txt";
  std::cout << "Initializing with file " << wordListFile << std::endl << std::endl;

  givenWords = new wordList(wordListFile);

  myHash = new hashTable<std::string>(*givenWords);

  std::cout << "Enter sort type: "; 
  std::cin >> sortMechanism;
  std::cout << std::endl;
}

// Assume we've received a grid and a sorted list of words...
// Print out any words that are found in the given grid that exist in the
// wordlist
void WordSearch::findMatches()
{
    int gridDim = givenGrid->getSize();
    //omp_set_num_threads(1);
    std::cout << "Running with: " << omp_get_max_threads() << " threads" << std::endl;
#pragma omp parallel
{
    // These variables are private for thread....
    std::vector<std::string> temp; 
    std::vector<std::string> tc;

    #pragma omp for 
    for (int ii = 0; ii < gridDim; ++ii)
    {
        for (int jj = 0; jj < gridDim; ++jj)
        { 
          temp = checkWord8Dim(ii, jj);
          tc.insert(tc.end(), temp.begin(), temp.end());
        }
    }
    // Needs to be critical so each thread can add their own tc to the shared
    // wordsFound vector
    #pragma omp critical
    {
      wordsFound.insert(wordsFound.end(), tc.begin(), tc.end());
    }
}

    std::sort(wordsFound.begin(), wordsFound.end());
    wordsFound.erase(unique(wordsFound.begin(), wordsFound.end()), wordsFound.end());
}


// Receives a coordinate to check in the grid. Check any words that start from
// the given point. Return a vector of words that can be made from this point. 
std::vector<std::string> WordSearch::checkWord8Dim(int ii, int jj)
{
    if(sortMechanism == USE_HASH)
    {
        return checkWord8DimHash(ii, jj);
    } else
    {
        return checkWord8DimSort(ii, jj);
    }
}

std::vector<std::string> WordSearch::checkWord8DimSort(int ii, int jj)
{

    std::vector<std::string> runningList;
    std::vector<std::string> odl;

    std::string firstLetter = std::string(1, givenGrid->get(ii, jj));
    bool canContinue = givenWords->wordLookup(firstLetter);

    if(canContinue)
    {
      for(int dir = 0; dir < 8; ++dir)
      {
        odl = checkWord1DimSort(ii, jj, dir);
        runningList.insert(runningList.end(), odl.begin(), odl.end());
      }
    }
    return runningList;
}

// Returns all the words found in a specified direction of the table as a vector
// 0: Right
// 1: Left
// 2: Up
// 3: Down
// 4: UR
// 5: DR
// 6: UL
// 7: DL
std::vector<std::string> WordSearch::checkWord1DimSort(int ii, int jj, int direction)
{
    char tempChar;

    std::vector<std::string> runningList;
    array_coord tempCoord;

    // Check word (increment j)
    std::string tempWord = "";
    tempWord += givenGrid->get(ii, jj);

    for(int offset = 1; offset < givenGrid->getSize(); ++offset)
    {
        tempCoord = findCoord(ii, jj, direction, offset); 
        tempChar = givenGrid->get(tempCoord.xx, tempCoord.yy);
       
        // Create our new string to check 
        tempWord += tempChar;

        // If our current stem doesn't exist anyhere in our list of words, 
        // we can break immediately...
        if(!givenWords->wordLookup(tempWord))
        {
            break;
        }

        // If the last character we added creates a string in our list, keep
        // going. Otherwise delete it and check if the resulting string is a full
        // word in the list.
        if(offset+1 >= MIN_WORD_LENGTH && givenWords->wordExists(tempWord) >= 0) 
        {
          runningList.push_back(tempWord);
        } 

    }
    return runningList;
}

// Receives a coordinate to check in the grid. Check any words that start from
// the given point. Return a vector of words that can be made from this point. 
std::vector<std::string> WordSearch::checkWord8DimHash(int ii, int jj)
{

    std::vector<std::string> runningList;
    std::vector<std::string> odl;

    for(int dir = 0; dir < 8; ++dir)
    {
      odl = checkWord1DimHash(ii, jj, dir);
      runningList.insert(runningList.end(), odl.begin(), odl.end());
    }

    return runningList;
}

std::vector<std::string> WordSearch::checkWord1DimHash(int ii, int jj, int direction)
{
    char tempChar;

    std::vector<std::string> runningList;
    array_coord tempCoord;

    // Check word (increment j)
    std::string tempWord = "";
    tempWord += givenGrid->get(ii, jj);

    for(int offset = 1; offset < givenGrid->getSize(); ++offset)
    {
        tempCoord = findCoord(ii, jj, direction, offset); 
        tempChar = givenGrid->get(tempCoord.xx, tempCoord.yy);
       
        // Create our new string to check 
        tempWord += tempChar;

        // If the last character we added creates a string in our list, keep
        // going. Otherwise delete it and check if the resulting string is a full
        // word in the list.
        if((offset+1 >= MIN_WORD_LENGTH) && (myHash->inList(tempWord)))
        {
          runningList.push_back(tempWord);
        } 

    }
    return runningList;
}


// Calculates an x, y coordinate on the grid given the direction and offset from
// point (ii, jj)
array_coord WordSearch::findCoord(int ii, int jj, int direction, int offset)
{
    array_coord nextPoint;

    int gridDim = givenGrid->getSize();

    switch(direction)
    {
      case 0: // Right
        nextPoint.yy = jj; 
        nextPoint.xx = (ii + offset) % gridDim;
        break;
      case 1: // Left
        nextPoint.yy = jj; 
        nextPoint.xx = (ii - offset) % gridDim;
        break;
      case 2: // Up
        nextPoint.yy = (jj - offset) % gridDim;
        nextPoint.xx = ii;
        break;
      case 3: // Down
        nextPoint.yy = (jj + offset) % gridDim;
        nextPoint.xx = ii;
        break;
      case 4: // UR
        nextPoint.yy = (jj - offset) % gridDim;
        nextPoint.xx = (ii + offset) % gridDim;
        break;
      case 5: // DR
        nextPoint.yy = (jj + offset) % gridDim;
        nextPoint.xx = (ii + offset) % gridDim;
        break;
      case 6: // UL
        nextPoint.yy = (jj - offset) % gridDim;
        nextPoint.xx = (ii - offset) % gridDim;
        break;
      case 7: // DL
        nextPoint.yy = (jj + offset) % gridDim;
        nextPoint.xx = (ii - offset) % gridDim;
        break;

    }

    while(nextPoint.yy < 0)
    {
      nextPoint.yy += gridDim;
    } 
    while(nextPoint.xx < 0)
    {
      nextPoint.xx += gridDim;
    } 

    return nextPoint;
}

void WordSearch::search()
{
  // Sort given words alphabetically
  auto start = high_resolution_clock::now();
  std::cout<<"sortMechanism: " << sortMechanism << std::endl;
  switch(sortMechanism)
  {
    case USE_INSERTION:
      givenWords->insertionSort();
      std::cout<< "Performed Insertion Sort" <<std::endl;
      break;
    case USE_QUICKSORT:
      givenWords->quickSort();
      std::cout<< "Performed Quick Sort" <<std::endl;
      break;
    case USE_MERGESORT:
      givenWords->mergeSort();
      std::cout<< "Performed Merge Sort" <<std::endl;
      break;
    case USE_STDSORT:
      givenWords->stdSort();
      std::cout<< "Performed std Sort" <<std::endl;
      break;
    case USE_HASH:
      std::cout<< "Using Hash Table" <<std::endl;
      break;
    case USE_HEAP:
      std::cout<< "Using Heap" <<std::endl;
      givenWords->heapSort();
      break;
    default:
      std::cout<< "Performing default: Merge Sort" << std::endl;
      givenWords->mergeSort();
      break;
  }
  auto end = high_resolution_clock::now();


  runTiming.sortTime = end-start;

  // Find and print matches
  start = high_resolution_clock::now();
  findMatches();
  end = high_resolution_clock::now();
  
  runTiming.searchTime = end-start;

  runTiming.totalTime = runTiming.sortTime + runTiming.searchTime;

}


void WordSearch::print_results()
{
    std::cout << "Num words found: " << wordsFound.size() << std::endl;    
    for(std::vector<std::string>::const_iterator it = wordsFound.begin();
        it != wordsFound.end(); ++it)
    {
      std::cout << *it << std::endl;
    }

    print_timing(&runTiming);
}

timingData WordSearch::getTiming()
{
    return runTiming;
}

void debugPrint(std::string msg)
{
  if(debug)
  {
    std::cout << msg << std::endl;
  }

}
