/*
 *      lock - a virtual lock that requires a key to open it
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include "../include/lock.h"
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      check.cc - check a key to see if it fits a lock
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int lock::check(char * bytes, int length) {
  aRealKey[0] = bytes[0];
  aRealKey[1] = bytes[1];
  aRealKey[2] = bytes[2];
  aRealKey[3] = bytes[3];
  int seed = ((aRealKey[0] << 24) + ((aRealKey[1] << 16) & 0xff0000) +
          ((aRealKey[2] << 8) & 0xff00) + (aRealKey[3] & 0xff));
  seed = seed % modulo;
  int size = seed & KEY_MASK;
  if (size < 7) {
    size = 7;
  }
  int index = 4;
  bool goodKey = true;
  while (index < size) {
    seed = (seed * slope + offset) % modulo;
    aRealKey[index] = seed & 0xff;
    goodKey &= aRealKey[index] == bytes[index];
    index++;
  }
  if (goodKey) {
    goodKey = size == length;  // if we don't match the input size it
                               // can't be a good key
  }
  aRealKeyLength = size;
  return (goodKey ? 1 : 0);
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      getARealKey.cc - return a real key
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int lock::getARealKey(char ** bytes) {
  *bytes = aRealKey;
  return aRealKeyLength;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      seedOf.cc -- get the seed of a key
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int lock::seedOf() {
  int seed = 0;
  if (aRealKeyLength > 0) {
    seed = ((aRealKey[3] << 24) + ((aRealKey[2] << 16) & 0xff0000) +
           ((aRealKey[1] << 8) & 0xff00) + (aRealKey[0] & 0xff));
  }
  return seed;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      nameOf.cc -- get the name of the lock
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
const char * lock::nameOf() {
  return name;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      lock - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
lock::lock(int * lockParameters, const char * name) {
  if (lockParameters) {
    slope = *lockParameters++;
    offset = *lockParameters++;
    modulo = *lockParameters++;
    this->name = name;
    memset(aRealKey, 0, sizeof(aRealKey));
    aRealKeyLength = 0;
  } else {
    fprintf(stderr, "Lock configuration is illegal - no lock defined\n");
    exit(-1);
  }
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      lock - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
lock::~lock() {
}
/* ---------------------------------------------------------------------- */

