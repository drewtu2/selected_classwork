// You'll want to compile this to a binary called "tokens".

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "constants.h"
#include "tokens.h"
#include "svec.h"



void tokenize(svec* tokens, char *input)
{
    char* fw;
    while(strlen(input) > 0)
    {
        fw = firstWord(input);
        if(fw != NULL)
        {
            svec_push_back(tokens, fw);
        }
        free(fw);
    } 
}

// Return the first word of an input string, input string is deliminated by
// spaces
char* firstWord(char* input)
{
    char* outputLoc = malloc(sizeof(char) * MAX_LENGTH);

    int delimState = 0;
    int charOffset = 0;
    int inputLength = strlen(input);
    // Iterate through the entire input string
    for(int ii = 0; ii < inputLength; ++ii) 
    {
        // Remove any white space from the front of the list
        // If we move the front of our sentence, adjust end appropriately
        cleanFront(input);
        inputLength = strlen(input);
       
        // If the lenght is now 0, the entire input was white space.  
        if(inputLength == 0)
        {
            break;
        }

        delimState = deliminator(&input[ii]);

        if(delimState != 0)
        {
            // Can only get triggered on operators at front of list
            if(ii == 0) 
            {
                int charOffset = delimState%2;
                memcpy(outputLoc, input, 1 + charOffset);
                outputLoc[ii + 1 + charOffset] = '\0';
                
                // Copy the input back into itself, remove the 1st + charOffset
                // characters 
                memmove(input, input + 1 + charOffset, strlen(input) - (1 + charOffset));
                input[strlen(input) - (1 + charOffset)] = '\0';
            } else 
            {
                // Copy into the the location of tokens, 
                // the value from input[0] to input[ii-1]
                memcpy(outputLoc, input, ii);
                outputLoc[ii] = '\0';

                // Copy input back into itself, removing the word we already 
                // moved to tokens. If this is the last word, 
                // return empty string
                if(input[ii] == '\0')
                {
                    strcpy(input, "\0");
                } else
                {
                    memmove(input, &input[ii], strlen(input) - ii + 1);
                }
            }
            return outputLoc;
        }
    }

    // For whatever reason, if we hit here, we don't have a string to return
    free(outputLoc);
    return NULL;
}

// Returns int if a given character is a deliminator based on the following 
// criteria
//  1: ' ' or '\0'
//  2: '<', '>', ';', '|', '&'
//  3: '||', '&&'
//  0: None of the Above
int deliminator(char* input)
{

    // Generate a list of symbols for each case. Pad arrays with the first symbol 
    // in the list if need be. 
    char case2[3] = {'<', '>', ';'}; 
    char case3[3] = {'|', '&', '|'}; 

    if(isspace(*input) || *input == '\0')
    {
        return 1;
    } else
    {
        for(int ii = 0; ii < 3; ++ii)
        {
            if(*input == case2[ii])
            {
                return 2;
            } else if(*input == case3[ii])
            {
                // Need to check for double symbol in case 3. 
                if(input[1] == case3[ii])
                {
                    return 3;
                } else
                {
                    return 2;
                }
            }
        }
    }

    // Not a deliminator
    return 0;
}

// Cleans out any white spaces at front of list by advancing pointer
void cleanFront(char* input)
{
    int count = 0;
    char temp = *input;
    while(count < strlen(input) && isspace(temp))
    {
        count++;
        temp = input[count];
    }
    if(count == strlen(input))
    {
        strcpy(input, ""); 
    } else if(count != 0)
    {
        memmove(input, &input[count], strlen(input) - count);
        input[strlen(input)-count] = '\0';
    }

}

// Returns the index of the first operator encountered. If no operators were 
// encountered, returns -1. 
int first_op(svec* tokens)
{
    int num_ops = 7;
    char* ops[] = {"|", ";", "||", "&&", "&", "<", ">"};

    for(int ii = tokens->size - 1; ii >=0; --ii)
    {
        for(int jj = 0; jj < num_ops; ++jj)
        {
            if(strcmp(svec_get(tokens, ii), ops[jj]) == 0)
            {
                return ii;
            }

        }
    }

    return -1;

}

int has_pipe(svec* tokens)
{
    return has_symbol(tokens, "|");
}

int has_redirect_to(svec* tokens)
{
    return has_symbol(tokens, ">");
}

int has_redirect_from(svec* tokens)
{
    return has_symbol(tokens, "<");
}

int has_and(svec* tokens)
{
    return has_symbol(tokens, "&&");
}

int has_or(svec* tokens)
{
    return has_symbol(tokens, "||");
}

int has_bg(svec* tokens)
{
    return has_symbol(tokens, "&");
}

int has_semi(svec* tokens)
{
    return has_symbol(tokens, ";");
}

// Returns an integer indicating the first index at which a pipe character is found.
// If the pipe character is not found, a value of -1 is returned. 
int has_symbol(svec* tokens, char* symbol)
{
    for(int ii = 0; ii < tokens->size; ++ii)
    {
        // If they're the same, then the compare equals zero
        if(strcmp(svec_get(tokens, ii), symbol) == 0)
        {
            return ii;
        }
    }

    return -1;
}

char** usedArgs(svec* tokens, int size)
{
    char** used_args = malloc(sizeof(char*) * (size + 1));

    for(int ii = 0; ii < size; ++ii)
    {
        used_args[ii] = svec_get(tokens, ii);
    }
    used_args[size] = 0;
    return used_args;
}

void freeCharArray(char** ptr, int num_elements)
{
    for(int ii = 0; ii < num_elements; ++ii)
    {
        free(ptr[ii]);
    }

    free(ptr);
}
