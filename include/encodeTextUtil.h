#ifndef INCLUDE_ENCODETEXTUTIL_H_
#define INCLUDE_ENCODETEXTUTIL_H_
/*
 *      encodeTextUtil.h - header for text encoder
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include "duplicateKeys.h"
#include "lock.h"
/* ---------------------------------------------------------------------- */
class encodeTextUtil {
 private:
  union { char bytes[32];
           int  seed;
  } overlay;
  lock * lockObject;
  std::map<char, duplicateKeys *> characterToKeyObject;
  std::map<char, duplicateKeys *>::iterator characterToKeyObjectIterator;
 public:
  int translate(const char * incomingText, char ** translatedText);
  encodeTextUtil(int * lockParameters, const char ** names, int lockIndex);
  ~encodeTextUtil();
};

#endif  // INCLUDE_ENCODETEXTUTIL_H_
