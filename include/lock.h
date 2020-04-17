#ifndef INCLUDE_LOCK_H_
#define INCLUDE_LOCK_H_
/*
 *      lock.h - header for lock object
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <iostream>
/* ---------------------------------------------------------------------- */
class lock {
 public:
  static const int MAX_BYTES = 32;   // space + 1 for a simple key
  static const int KEY_MASK = 0x1f;  // only allow of keys of 31 bytes
 private:
  int slope;
  int offset;
  int modulo;
  const char * name;
  char aRealKey[MAX_BYTES];
  int aRealKeyLength;

 public:
  int check(char * bytes, int length);
  int getARealKey(char ** bytes);
  int seedOf();
  const char * nameOf();
  lock(int * lockParameters, const char * name);
  ~lock();
};

#endif  // INCLUDE_LOCK_H_
