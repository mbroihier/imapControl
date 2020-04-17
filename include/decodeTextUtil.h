#ifndef INCLUDE_DECODETEXTUTIL_H_
#define INCLUDE_DECODETEXTUTIL_H_
/*
 *      decodeTextUtil.h - header for text decoder
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
#include "lock.h"
/* ---------------------------------------------------------------------- */
class decodeTextUtil {
 private:
  union { char bytes[32];
           int  seed;
  } overlay;
  lock * lockObject;
 public:
  int translate(char * incomingText, char ** translatedText, int size);
  decodeTextUtil(int * parameters, const char * name);
  ~decodeTextUtil();
};

#endif  // INCLUDE_DECODETEXTUTIL_H_
