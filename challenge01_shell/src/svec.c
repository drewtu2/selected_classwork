/*
 * This code is based on the svec code provided by Nat Tuck during lecture
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "svec.h"
#include "tokens.h"
#include "constants.h"


// Creates a svec with a given intial capacity.
svec* svec_make(int init_cap)
{
    svec* sv = malloc(sizeof(svec));
    sv->size = 0;
    sv->capacity = init_cap;
    sv->data = malloc(init_cap * sizeof(char*));
   
    return sv; 
}

// from a given file descriptor
svec* svec_make_from_file(FILE* myFile)
{
    svec* sv = svec_make(MAX_LENGTH);
    svec* svec_temp;
    char tmp[MAX_LENGTH];
   
    while(fgets(tmp, MAX_LENGTH, myFile) != NULL)
    {
        svec_temp = svec_make(MAX_LENGTH);
        // Get an entire string and tokenize;
        tokenize(svec_temp, tmp);
        
        // Add that tokenized string to our new obj..
        svec_concat(sv, svec_temp);

        // Clean up some memory
        svec_free(svec_temp);
    }


    return sv; 
}

void svec_concat(svec* sv, svec* add)
{
    
    char* temp;
    for(int ii = 0; ii < add->size; ++ii)
    {
        temp = svec_get(add, ii);
        svec_push_back(sv, temp);
    }

    //memcpy(sv->data[sv->size], add->data, add->size);

}

// Frees all memory allocated to a given svec. An svec owns all of the data
// pointed to by data (frees data as well)
void svec_free(svec* sv)
{
    // Free memory pointed to in data
    for(int ii = 0; ii < sv->size; ++ii)
    {
        free(sv->data[ii]);
    }
    // Free data pointer 
    free(sv->data);
    // Free entire obj
    free(sv);
}

char* svec_get(svec* sv, int index)
{
    assert(index >= 0 && index < sv->size);
    return sv->data[index];
}

void svec_put(svec* sv, int index, char* item)
{
    assert(index >= 0 && index < sv->size);
    // Copy value into vector (want to own data)
    //printf("strlen(item): %u\n", strlen(item));
    /*
    char *temp = malloc(strlen(item) * sizeof(char));
    strcpy(temp, item);
    sv->data[index] = temp; 
    */
    sv->data[index] = strdup(item); 
}

void svec_push_back(svec* sv, char* item)
{
    if(sv->size >= sv->capacity)
    {
        printf("Resized\n");
        sv->capacity *= 2;
        sv->data = (char**) realloc(sv->data, sv->capacity * sizeof(char*));
    }
   
    // Stick the given item into the end of the vecotr 
    sv->size++;
    svec_put(sv, sv->size - 1, item);
}

void svec_fprint(svec* sv, FILE* file)
{
    for(int ii = 0; ii < sv->size; ++ii)
    {
        //printf("[%s]\n", svec_get(sv, ii));
        fprintf(file, "%s\n", svec_get(sv, ii));
    }
    char* a = NULL;
    fwrite(a, 1, 1, file);
}

void svec_print(svec* sv)
{
    for(int ii = 0; ii < sv->size; ++ii)
    {
        //printf("[%s]\n", svec_get(sv, ii));
        printf("%s\n", svec_get(sv, ii));
    }

}

// Splits a given svec at a given index. The left hand side of the split is left 
// in sv. The right hand side of the split is returned. The index used to split is 
// dropped. The user is responsible for freeing the given svec by calling svec_free(sv)
svec* svec_split(svec* sv, int index)
{
    svec* right = svec_make(MAX_LENGTH);
   
    // Add elements from [index+1, sv->size) to the new svec.  
    char* temp;
    for(int ii = index + 1; ii < sv->size; ++ii)
    {
        temp = svec_get(sv, ii);
        svec_push_back(right, temp);
    }

    // Truncate SV.
    sv->data[index] = 0;
    sv->size = index;
     
    return right; 

}

// Truncates a given svec to [start_index, end_index)
svec* svec_trunc(svec* sv, int start_index, int end_index)
{
    svec* new_svec = svec_make(MAX_LENGTH);
    char* word;

    // Add from [start_index, end_index) to the new svec.
    for(int ii = start_index; ii < end_index; ++ii)
    {
        word = svec_get(sv, ii);
        svec_push_back(new_svec, word);
    }    

    svec_free(sv);
    return new_svec;
}







