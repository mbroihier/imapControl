/*
 *      encodeText - use lock object to encode text
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include "encodeTextUtil.h"
#include "encodeTextConfig.h"
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/*
 *      translate.cc -- translate the text into an encoded version
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int  encodeTextUtil::translate(const char * incomingText, char ** translatedText) {
  char * workingPointer = 0;
  int size = strlen(incomingText);
  int messageSize = 0;
  int charSize;
  char * key;
  for (int index = 0; index < size; index++) {
    overlay.seed = characterToKeyObject[incomingText[index]]->getNext();
    lockObject->check(overlay.bytes, 4);
    charSize = lockObject->getARealKey(&key);
    if (charSize > 6) {
      charSize = charSize - 1;
    } else {
      fprintf(stderr, "Error in translation - this should not happen\n");
      exit(-1);
    }
    if (index == 0) {
      workingPointer = reinterpret_cast<char *>(malloc(charSize));
    } else {
      workingPointer = reinterpret_cast<char *>(realloc(workingPointer, charSize + messageSize));
    }
    memcpy(workingPointer + messageSize, key, charSize);
    messageSize += charSize;
  }
  *translatedText = workingPointer;
  return messageSize;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      encodeTextUtil - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

encodeTextUtil::encodeTextUtil(int * lockParameters, const char ** names, int lockIndex) {
  char * key;
  lockObject = new lock(lockParameters+3*lockIndex, *(names+lockIndex));
  int size;
  bool done = false;
  unsigned int rand_seed = 31416;
  struct timespec seedTime;
  clock_gettime(CLOCK_MONOTONIC, &seedTime);
  rand_seed = seedTime.tv_nsec & 0xffffffff;

  std::map<char, int>  duplicates;
  do {
    overlay.seed = rand_r(&rand_seed);
    lockObject->check(overlay.bytes, 4);
    size = lockObject->getARealKey(&key);
    if (size > 6) {
      int c = key[size - 1];
      characterToKeyObjectIterator = characterToKeyObject.find(c);
      if (characterToKeyObjectIterator != characterToKeyObject.end()) {
        duplicates[c] = characterToKeyObject[c]->insert(lockObject->seedOf());
        if (duplicates.size() >= 256) {
          done = true;
          for (std::map<char, int>::iterator it = duplicates.begin(); it != duplicates.end(); it++) {
            done &= (it->second > 3);
          }
        }
      } else {
        characterToKeyObject[c] = new duplicateKeys();
        duplicates[c] = characterToKeyObject[c]->insert(lockObject->seedOf());
      }
    } else {
      fprintf(stderr, "Shouldn't have gotten a key this small: %d", size);
    }
  } while (!done);
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      encodeTextUtil - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

encodeTextUtil::~encodeTextUtil() {
  delete lockObject;
}
/* ---------------------------------------------------------------------- */
