//
//  d_hashtable.h
//  wordsearch
//

#ifndef wordsearch_d_hashtable_h
#define wordsearch_d_hashtable_h

#define hashtableSIZE 80000
#include <list>
#include <functional>

template <typename T>
class hashtable
{
public:
   hashtable(int size);
   hashtable();
   void addItem(T &item);
   void deleteItem(T &item);
   bool inlist(T &item);
   int hash(T &item);
   
private:
   vector<vector<T> > list;
   int size;
};


template <typename T>
hashtable<T>::hashtable()
// Constructor that inserts all the words in the wordlist into a hash table.
{
   T newWord;
   ifstream fin;
   
   size = hashtableSIZE;
   list.resize(size);
   
      // Open the wordlist file.
   string fileName = "../data/wordlist2.txt";
   
   fin.open(fileName.c_str());
   if (!fin)
   {
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }
   
   while (fin)
   {
      fin >> newWord;
      if (fin)
         addItem(newWord);
   }
   
//   for (int i = 0; i < size; i++)
//      cout << (list[i]).size() << endl;
      
   fin.close();
}

template <typename T>
hashtable<T>::hashtable(int size)
// Initializes an empty hashtable with this size.
{
   list.resize(size);
}

template <typename T>
void hashtable<T>::addItem(T &item)
// Add an item to the hashed location in the hashtable.  Add to the end of the list
// at the hashed location.
{
   unsigned int n = hash(item);
   list[n].push_back(item);
}

template <typename T>
int hashtable<T>::hash(T &item)
// Return the hashed value for item, using the built in hash function.  Map to a value in the right
// range for the table.
{
   std::hash<T> h;
   return h(item) % size;
}

template <typename T>
bool hashtable<T>::inlist(T &item)
// Return true iff item is in the hashtable.
{
   int n = hash(item);
   for (int i = 0; i < list[n].size(); i++)
   {
      if (list[n][i] == item)
         return true;
   }
   return false;
}


#endif
