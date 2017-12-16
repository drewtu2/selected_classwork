#ifndef GRID_H
#define GRID_H

#include <string>
#include <fstream>

class grid
{
 public:
  grid();
  grid(std::string fileName);
  ~grid();
  
  char get(int xx, int yy);
  int getSize();

 private:
  int size;
  char** gridA;  
};

#endif
