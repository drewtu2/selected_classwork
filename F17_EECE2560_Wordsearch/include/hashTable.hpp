#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <string>
#include <algorithm>
#include <functional>

#include "wordlist.h"

template<class T>
class hashTable {

 public:

  hashTable(wordList &input);
  ~hashTable();

  // Adds an item to the hash table
  void addItem(T item);

  // Removes an item from the hash table
  void deleteItem(T item);

  // Returns a boolean saying whether or not the given item was in the hash
  // table
  bool inList(T item);

  size_t hash(T key);

 private:
  // Table[key][chain index]
  std::vector<std::vector<T> > table;
  float target_alpha; // target for numKeys/numBins
  int num_bins;

};

template<class T> hashTable<T>::hashTable(wordList &init_list)
{
    num_bins = init_list.getSize();
    // Bins should be twice the size of the initial list
    table.resize(num_bins);

    for(int ii = 0; ii < init_list.getSize(); ++ii)
    {
        this->addItem(init_list.get(ii));
    }

}

template<class T> hashTable<T>::~hashTable()
{
}

template<class T> void hashTable<T>::addItem(T item)
{
    int index = hash(item);
    table[index].push_back(item);
}

template<class T> void hashTable<T>::deleteItem(T item)
{
    std::vector<T>* chain = &table[hash(item)];
    chain->erase(std::remove(chain->begin(), chain->end(), item), chain->end());
}

template<class T> bool hashTable<T>::inList(T item)
{
    std::vector<T> *chain = &table[hash(item)];
    if(std::find(chain->begin(), chain->end(), item) != chain->end())
    {
      return true;
    }
    return false;
}

template<class T> size_t hashTable<T>::hash(T key)
{
    return std::hash<T>{}(key)%num_bins;
}



#endif
