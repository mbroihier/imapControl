#ifndef INCLUDE_LOCKS_H_
#define INCLUDE_LOCKS_H_
/*
 *      locks.h - header for locks object
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <regex>
#include <string>
#include "decodeTextUtil.h"
#include "lock.h"
/* ---------------------------------------------------------------------- */
class locks {
 public:
  static int const MAX_BYTES = 1024;
  static int const MAX_NUMBER_OF_LOCKS = 10;
  static int const OLD_KEY_LIST_SIZE = 1000;
  static bool const debug = DEBUG;
 private:
  int keyList[OLD_KEY_LIST_SIZE];
  lock * lockList[MAX_NUMBER_OF_LOCKS];
  decodeTextUtil * decoderList[MAX_NUMBER_OF_LOCKS];
  int numberOfLocks;
  int keyIndex;
  int numberOfKeys;
  int lastLockMatchedIndex;
  int checkList(int seed);

 public:
  int getLastLockMatchedIndex();
  int check(std::string digits);
  locks(const int * lockParameters, const char ** names);
  ~locks();
};

#endif  // INCLUDE_LOCKS_H_
