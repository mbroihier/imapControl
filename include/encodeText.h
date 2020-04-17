#ifndef INCLUDE_ENCODETEXT_H_
#define INCLUDE_ENCODETEXT_H_
/*
 *      encodeText.h - header for text encoder
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
class encodeText {
 private:
  union { char bytes[32];
           int  seed;
  } overlay;
  lock * lockObject;
  std::map<char, duplicateKeys *> characterToKeyObject;
  std::map<char, duplicateKeys *>::iterator characterToKeyObjectIterator;
 public:
  int translate(const char * incomingText, char ** translatedText);
  encodeText();
  ~encodeText();
};

#endif  // INCLUDE_ENCODETEXT_H_
