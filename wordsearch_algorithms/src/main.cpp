#include "wordSearch.h"
#include <iostream>


int main()
{

  std::cout << "Creating search object" << std::endl;

  WordSearch mySearch;
 
  std::cout << "Executing search" << std::endl;
  
  mySearch.search();

  mySearch.print_results();

  std::cout << "Finished" << std::endl;

  return 0;
}
