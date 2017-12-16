// Solutions to project 3

//#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include "d_matrix.h"

using namespace std;

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

class grid
{
   public:
      grid();
      char getChar(int i, int j);
      int getRows();
      int getCols();
   private:
      matrix<char> charMatrix;
};

grid::grid()
{
   ifstream fin;

   // Read the sample grid from the file.
   string fileName = "../data/input250.txt";

   fin.open(fileName.c_str());
   if (!fin)
   {	
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }

   int numCols, numRows;
   
   // Read the size of the grid and the grid characters
   fin >> numRows;
   fin >> numCols;

   charMatrix.resize(numRows,numCols);
   
   cout << "Reading " << numRows << " rows and " << numCols << " columns" << endl;

   for (int i = 0; i < numRows; i++)
     for (int j = 0; j < numCols; j++)
	fin >> charMatrix[i][j];
/*
   for (int i = 0; i < numCols; i++)
   {
     for (int j = 0; j < numCols; j++)
	cout << charMatrix[i][j] << " ";
     cout << endl;
   }*/
   
   fin.close();
}

char grid::getChar(int i, int j)
{
   return charMatrix[i][j];
}

int grid::getRows()
{
   return charMatrix.rows();
}

int grid::getCols()
{
   return charMatrix.cols();
}

