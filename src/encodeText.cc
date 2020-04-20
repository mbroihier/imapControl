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
#include "encodeText.h"
#include "encodeTextConfig.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] =
        "Usage: %s <text string>\n"
        "  a text string enclosed with quotes\n"
        "  -h                       : help\n"
        "  --help                   : help\n";

static struct option longOpts[] = {
  {"help", no_argument, NULL, 1},
  { NULL, 0, NULL, 0 }
};

/* ---------------------------------------------------------------------- */
/*
 *      translate.cc -- translate the text into an encoded version
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int  encodeText::translate(const char * incomingText, char ** translatedText) {
  char * workingPointer = 0;
  int size = strlen(incomingText);
  int index = 0;
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
 *      encodeText - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

encodeText::encodeText() {
  char * key;
  int lockParameters[] = {1807, 45289, 214326, 0};
  const char * names[] = {"encoder", 0};
  lockObject = new lock(lockParameters, *names);
  char c;
  int size;
  bool done = false;
  unsigned int rand_seed = 31416;
  struct timespec seedTime;
  clock_gettime(CLOCK_BOOTTIME, &seedTime);
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
      std::cerr << "Shouldn't have gotten a key this small: " << size <<std::endl;
    }
  } while (!done);
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      encodeText - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

encodeText::~encodeText() {
  delete lockObject;
}
/* ---------------------------------------------------------------------- */
#ifdef ENCODE_BUILD_MAIN
int main(int argc, char *argv[]) {
  int c;

  if (argc != 2) {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  int doneProcessing = 0;
  while ((c = getopt_long(argc, argv, "h", longOpts, NULL)) >= 0) {
    switch (c) {
      case 'h': {
        fprintf(stderr, "Text Encode VERSION %s\n", STR_VALUE(encodeText_VERSION_NUMBER));
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      case 1: {
        fprintf(stderr, "Text Encode VERSION %s\n", STR_VALUE(encodeText_VERSION_NUMBER));
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      default:
        break;
    }
  }

  const char * incomingString = argv[1];

  char * translatedString = 0;

  encodeText textEncoderInstance;

  int size = textEncoderInstance.translate(incomingString, &translatedString);

  fprintf(stdout, "{ ");
  for (int index = 0; index < size; index++) {
    if (index == 0) {
      fprintf(stdout, "0x%2.2x", translatedString[index]);
    } else {
      fprintf(stdout, ", 0x%2.2x", translatedString[index]);
    }
  }
  fprintf(stdout, " }\n");

  free(translatedString);

  return 0;
}
#endif
/* ---------------------------------------------------------------------- */
