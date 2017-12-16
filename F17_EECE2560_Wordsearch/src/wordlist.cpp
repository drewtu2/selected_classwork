#include "wordlist.h"
#include <string.h>
#include <assert.h>
#include <omp.h>

wordList::wordList()
{
    std::ifstream file;
    std::string var;
    file.open("data/wordlist.txt");
    assert(!file.fail());

    file>>var;
    while(!file.eof())
    {
        word_list.push_back(var);
        file>>var;
    }
    file.close();
}

wordList::wordList(std::string fileName)
{
    std::ifstream file(fileName.c_str());
    std::string var;
    assert(!file.fail());
    file>>var;
    while(!file.eof())
    {
        word_list.push_back(var);
        file>>var;
    }
    file.close();
}

std::ostream &operator<<(std::ostream &os, const wordList &w)
{
  for(int i =0; i<w.word_list.size(); i++)
  {
  os<<w.word_list.at(i)<<' ';    
  }
  return os;
}

void wordList::insertionSort()
{
  std::string temp;

  int j;
  for (int i=0; i<word_list.size(); i++)
  {
    j=i; 
    while(j >0 && word_list.at(j).compare(word_list.at(j-1)) < 0)
    {
       temp = word_list.at(j);
       word_list.at(j) = word_list.at(j-1);
       word_list.at(j-1) = temp;
      j--;
    }
  }
  this -> word_list = word_list;
}

void wordList::stdSort()
{
    std::sort(word_list.begin(), word_list.end());
}

void wordList::quickSort(int left, int right)
{
    if(right==-1)
    {
        right=word_list.size()-1;
    }
    
    int i=left;
    int j=right;
    std::string pivot=word_list.at((i+j)/2);
    
    while(i<=j)
    {
    
        while(word_list.at(i).compare(pivot)<0)
        {
            i++;
        }
        
        while(word_list.at(j).compare(pivot)>0)
        {
            j--;
        }
        
        if(i<=j)
        {
            std::string temp=word_list.at(j);
            word_list.at(j)=word_list.at(i);
            word_list.at(i)=temp;
            i++;
            j--;
        }
    }
    if(left<j)
    {
        quickSort(left, j);
    }
    
    if(right>i)
    {
        quickSort(i, right);
    }
}

void wordList::merge(int left, int middle, int right)
{
  int n1 = middle - left +1;
  int n2 = right - middle;
  std::vector<std::string> l;
  std::vector<std::string> r;
//  l.resize(n1+1);
//  r.resize(n2+1);

  for(int i=0; i<n1; i++)
    l.push_back( word_list.at(left + i));
  for(int j=0; j<n2; j++)
    r.push_back( word_list.at(middle + j));

  l.push_back( "zzzzzzzzzzzz");
  r.push_back( "zzzzzzz");

  int i=0;
  int j=0;

  for(int k=left; k<right; k++)
  {
    if(l.at(i).compare(r.at(j))<=0)
    {
      word_list.at(k) = l.at(i);
      i++;
    }
    else
    {
      word_list.at(k) = r.at(j);
      j++;
    }
  }
}
void wordList::mergeSort(int left, int right)
{
//std::cout<<"Right: " << right<<std::endl;
  if (right==-1){
 //   std::cout<<"entering main";
  //  truth = false;
    right=word_list.size()-1;
  }

  if(left<right){
    int middle=(left+right)/2;
    mergeSort(left, middle);
    mergeSort(middle+1, right);
    merge(left, middle, right);
  }
}


void wordList::heapSort()
{ //Creates new heap and places wordlist inside it, then sorts using heap sort
  heapV<std::string, std::string> newheap;
  newheap.initializeMaxHeap(word_list);
  newheap.heapsort(word_list.size());
  for(int i=0; i<word_list.size(); i++)
  word_list.at(i)= newheap.getItem(i);

  //setList(newheap.getWords());
  //}
}
int wordList::wordExists(std::string test, int first, int last, int first_round)
{
    int index;
    if(first_round == 0)
    {
        last = word_list.size()-1;
    }
     
    if (first>last)
    {
        index =-1;
    } else
    {
        int mid = (first + last)/2;

        if(test.compare(word_list.at(mid)) == 0 )
          index = mid;
        else if(test.compare(word_list.at(mid)) < 0)
          index = wordExists(test,first, mid-1, 10);
        else 
          index =wordExists(test,mid+1, last, 10);
          
    }

    return index;
}

bool wordList::wordLookup(std::string test, int first, int last, int run)
{
  bool exists=false;
  if (run==0)
  last = word_list.size() - 1;
 

  if (first>last)
  exists = false;
  
  else
  {
    int mid = (first + last)/2;

    if(word_list.at(mid).find(test) != std::string::npos)
    exists = true;
    else if(test.compare(word_list.at(mid)) < 0)
    exists = wordLookup(test,first, mid-1, 10);
    else
    exists = wordLookup(test,mid+1, last, 10);
  }
return exists; 

}
void wordList::setVector(std::vector<std::string> word_list)
{
  this-> word_list = word_list;
}

std::vector<std::string> wordList::getVector()
{
  return word_list;
}

std::string wordList::get(int i)
{
    return word_list[i];
}

int wordList::getSize()
{
    return word_list.size();
}
