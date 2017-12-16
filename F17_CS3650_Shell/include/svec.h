/*
 * This code is based on the svec code provided by Nat Tuck during lecture
 */

#ifndef SVEC_H
#define SVEC_H

typedef struct svec {

  int size;       // How many elements we have in the vector
  int capacity;   // How many elements we CAN store in the vector
  char** data;    // A pointer to whatever data we are storing...

} svec;

// Initialize
svec* svec_make(int init_cap);
svec* svec_make_from_file(FILE* myFile);
svec* svec_split(svec* sv, int index);
void svec_free(svec* sv);

// Accessors
char* svec_get(svec* sv, int index);

// Returns an array starting with the given index to the end
void svec_toArray(svec* sv, char* array, int index);
void svec_put(svec* sv, int index, char* item);
svec* svec_trunc(svec* sv, int start_index, int end_index);

void svec_push_back(svec* sv, char* item);
void svec_print(svec* sv);
void svec_fprint(svec* sv, FILE* file);
void svec_concat(svec* sv, svec* add);


#endif
