/*
 *      duplicateKeys - manage a set of keys that match the same lock
 *                      how a match is determined is not done here
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include "duplicateKeys.h"
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/*
 *      getNext.cc -- get the next key to use
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int  duplicateKeys::getNext() {
  int key = keys[currentKeyToUse++];
  if (currentKeyToUse >= numberOfDuplicates) {
    currentKeyToUse = 0;
  }
  return key;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      insert.cc -- insert a new duplicate into the list
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int  duplicateKeys::insert(int seed) {
  if (numberOfDuplicates == 0) {
    keys = reinterpret_cast<int *>(malloc(sizeof(int)));
  } else {
    keys = reinterpret_cast<int *>(realloc(keys, sizeof(int) * (numberOfDuplicates + 1)));
  }
  keys[numberOfDuplicates] = seed;
  numberOfDuplicates++;
  return numberOfDuplicates;;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      duplicateKeys - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

duplicateKeys::duplicateKeys() {
  numberOfDuplicates = 0;
  currentKeyToUse = 0;
  keys = 0;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      duplicateKeys - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

duplicateKeys::~duplicateKeys() {
  if (keys) free(keys);
}
