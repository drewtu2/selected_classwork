#include "grid.h"


grid::grid()
{
  

}
grid::grid(std::string fileName)
{

  std::ifstream inputFile(fileName.c_str());
  inputFile>>size>>size;
  gridA = new char*[size];
  for(int i=0; i<size; i++)
  {
    gridA[i] = new char[size];
  }
  for(int i =0; i<size; i++)
  {
  for(int j = 0; j<size; j++)
    {
    
      inputFile>>gridA[i][j]; 
    }
  }

}

char grid::get(int xx, int yy)
{
  return gridA[yy][xx];
}
int grid::getSize()
{
  return size;
}

grid::~grid()
{


}
