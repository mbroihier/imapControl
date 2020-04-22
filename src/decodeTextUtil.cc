/*
 *      decodeText - use lock object to decode bytes
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <getopt.h>
#include <unistd.h>
#include "decodeTextUtil.h"
#include "decodeTextConfig.h"
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      translate.cc -- translate the text into an decoded version
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int  decodeTextUtil::translate(char * incomingText, char ** translatedText, int size) {
  char * workingPointer = 0;
  int messageSize = 0;
  int charSize;
  char * key;
  for (int index = 0; index < size;) {
    overlay.bytes[0] = incomingText[index];
    overlay.bytes[1] = incomingText[index+1];
    overlay.bytes[2] = incomingText[index+2];
    overlay.bytes[3] = incomingText[index+3];
    lockObject->check(overlay.bytes, 4);
    charSize = lockObject->getARealKey(&key);
    if (charSize > 6) {
      charSize = charSize - 1;
    } else {
      fprintf(stderr, "Error in translation - this should not happen\n");
      exit(-1);
    }
    if (index == 0) {
      workingPointer = reinterpret_cast<char *>(malloc(2 + messageSize));
    } else {
      workingPointer = reinterpret_cast<char *>(realloc(workingPointer, 2 + messageSize));
    }
    workingPointer[messageSize] = key[charSize];
    messageSize += 1;
    workingPointer[messageSize] = 0;
    index += charSize;
  }
  *translatedText = workingPointer;
  return messageSize;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      decodeTextUtil - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

decodeTextUtil::decodeTextUtil(int * parameters, const char * name) {
  lockObject = new lock(parameters, name);
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      decodeTextUtil - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

decodeTextUtil::~decodeTextUtil() {
  delete lockObject;
}
/* ---------------------------------------------------------------------- */
