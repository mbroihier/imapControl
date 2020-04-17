#ifndef INCLUDE_DUPLICATEKEYS_H_
#define INCLUDE_DUPLICATEKEYS_H_
/*
 *      duplicateKeys.h - header to create duplicate key objects
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>
/* ---------------------------------------------------------------------- */
class duplicateKeys {
 private:
  int numberOfDuplicates;
  int currentKeyToUse;
  int * keys;
 public:
  int getNext(void);
  int insert(int seed);
  duplicateKeys();
  ~duplicateKeys();
};
#endif  // INCLUDE_DUPLICATEKEYS_H_
