#ifndef TOKENS_H
#define TOKENS_H

#include "svec.h"

char* firstWord(char* input);
void tokenize(svec* tokens, char* input);
void cleanFront(char*input);
int deliminator(char* input);

// Operators
int first_op(svec* tokens);
int has_pipe(svec* tokens);
int has_redirect_to(svec* tokens);
int has_redirect_from(svec* tokens);
int has_and(svec* tokens);
int has_or(svec* tokens);
int has_bg(svec* tokens);
int has_semi(svec* tokens);

int has_symbol(svec* tokens, char* symbol);

char** usedArgs(svec* tokens, int size);
void freeCharArray(char** ptr, int size);



#endif
