#include "wordlist.h"
#include "hashTable.hpp"

#include <iostream>
#include <string>

int main()
{
    wordList *word = new wordList("data/wordlist.txt");
    //wordList *word = new wordList("data/wordlist2.txt");
    //wordList *word = new wordList("data/puzzle10-words.txt");
    wordList *word2 = new wordList("data/wordlist.txt");
    //wordList *word2 = new wordList("data/wordlist2.txt");
    //wordList *word2 = new wordList("data/puzzle10-words.txt");
    hashTable<std::string> *hash = new hashTable<std::string>(*word);
    std::cout << "Begin sorting" << std::endl;
    //word->insertionSort();
    //word->quickSort();
    word->mergeSort();
    word2->stdSort();
    std::cout << "finished sorting" << std::endl;

    std::cout << "minally: " << hash->inList("minally") << std::endl;
    std::cout << "huntress: " << hash->inList("huntress") << std::endl;
    std::cout << "gorgonzola: " << hash->inList("gorgonzola") << std::endl;
    hash->deleteItem("huntress");
    std::cout << "huntress: " << hash->inList("huntress") << std::endl;

    int incorrect = 0;
    for(int i = 0; i < word->getSize(); ++i)
    {
    /*    if(i < 10)
        {
            std::cout<<"Value " << i << std::endl;
            std::cout<<word->getVector().at(i)<<std::endl;
            std::cout<<word2->getVector().at(i)<<std::endl;
        }
     */   if(word->get(i).compare(word2->get(i)) != 0)
        {
            std::cout<<word->getVector().at(i)<<std::endl;
            std::cout<<word2->getVector().at(i)<<std::endl;
            incorrect++;
        }
    }

    std::cout << "apple: " << word->wordExists("apple") << std::endl;
    std::cout << "abode: " << word->wordExists("abode") << std::endl;
    std::cout << "network: " << word->wordExists("network") << std::endl;
    std::cout << "Size: " << word->getSize() << std::endl;;

    std::cout << "Total Incorrect: " << incorrect << std::endl;

    return 0;
}
