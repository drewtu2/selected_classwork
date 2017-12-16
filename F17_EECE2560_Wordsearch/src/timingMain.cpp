#include "wordSearch.h"
#include <iostream>
#include <vector>

void print_timing_1l(timingData* myTime)
{
    std::cout << "Sort Time: " << myTime->sortTime.count() << " ";
    std::cout << "Search Time: " << myTime->searchTime.count() << " ";
    std::cout << "Total Time: " << myTime->totalTime.count() << " ";
}

struct results{
    std::string gridName;
    std::string wordList;
    int sortMechanism;
    timingData timings;
};

int main()
{


    std::vector<std::string> gridFiles = 
        {"data/input15.txt", "data/input30.txt", "data/input250.txt"};
//    std::vector<std::string> wordFiles = 
//        {"data/wordlist.txt", "data/wordlist2.txt"};
    std::vector<std::string> wordFiles = 
        {"data/wordlist2.txt"};
    std::vector<results> myResults;

    for(int ii = 1; ii <= 6; ++ii) // What search function
    {
        if(ii == 4)
        {
            // Don't do this one. (std::sort)
        } 
        else{
            for(auto & grid : gridFiles)
            {
                for(auto & list : wordFiles)
                {
                    WordSearch mySearch(grid, list, ii);
                    
                    std::cout << "Running " << grid << " " << list << " " << ii << std::endl;

                    mySearch.search();
                    results temp;
                    temp.gridName = grid;
                    temp.wordList = list;
                    temp.sortMechanism = ii;
                    temp.timings = mySearch.getTiming();
                    myResults.push_back(temp);
                }
            }
        }
    }

    // Have a vector of timing data...

    for(auto & singleResult : myResults)
    {
        std::cout << "grid: " << singleResult.gridName << "\t";
        std::cout << "wordList: " << singleResult.wordList<< "\t";
        std::cout << "sortMechanism: " << singleResult.sortMechanism<< "\t";
        std::cout << "timingData: ";
        print_timing_1l(&singleResult.timings);
        std::cout << std::endl;

    }
    
    return 0;
}
