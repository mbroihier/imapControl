/*
 *      locks - Create a list of locks that will check arrays of bytes
 *              to see if they form a key to one of the locks in the
 *              array.  History will be kept so that the same keys will
 *              not easily be reused.
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include "../include/locks.h"
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      checkList.cc -- check a list of previous keys and disallow its
 *                      reuse
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int locks::checkList(int seed) {
  bool found = false;
  int returnCode = 1;  // it was good coming in by default
  if (numberOfKeys > 0) {
    for (int index = 0; index < numberOfKeys; index++) {
      if (seed == keyList[index]) {
        found = true;  // this is a recent repeat
        std::cout << "found " << seed << std::endl;
        break;
      }
    }
    if (!found) {  // put this key in the list
      std::cout << "Putting seed into list " << seed << std::endl;
      keyList[keyIndex] = seed;
      keyIndex++;
      if (keyIndex == OLD_KEY_LIST_SIZE) {
        keyIndex = 0;
      }
      if (numberOfKeys < OLD_KEY_LIST_SIZE) {
        numberOfKeys++;
      }
    } else {  // we have seen this before
      returnCode = 2;
    }
  } else {
    std::cout << "First time forced failure with seed " << seed << std::endl;
    returnCode = 2;  // always fail the first time
    keyList[0] = seed;
    keyIndex = 1;
    numberOfKeys = 1;
  }
  return returnCode;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      check.cc -- check a key to see if it fits a lock - if it does
 *                  make sure it hasn't been recently used
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int locks::check(std::string digitsString) {
  char buffer[MAX_BYTES];
  std::regex digitPattern(" [0-9a-fA-f]{2}");
  bool none = false;
  int index = 0;

  std::string incomingRemainderString = digitsString;
  std::cmatch digits;
  do {
    const char * incomingRemainder = incomingRemainderString.c_str();
    std::cout << "now searching str: " << incomingRemainderString << std::endl;
    std::cout << "now searching c_str: " << incomingRemainder << std::endl;
    if (std::regex_search(incomingRemainder, digits, digitPattern)) {
      buffer[index] = std::stoi(digits[0], 0, 16);
      std::cout << "new entry: " << std::hex << static_cast<int>(buffer[index]) << " " << std::endl;;
      incomingRemainderString = digits[0].second;
      index++;
    } else {
      none = true;
    }
  } while (!none);
  int candidateKeySize = index;
  std::cout << std::endl;
  bool ok = false;
  char * key = 0;
  int keySize = 0;
  int status = 0;
  int seed = 0;
  for (int index = 0; index < numberOfLocks; index++) {
    status = lockList[index]->check(buffer, candidateKeySize);
    keySize = lockList[index]->getARealKey(&key);
    seed = lockList[index]->seedOf();
    std::cout << "A valid key for this lock and seed would be:" << std::endl;
    for (int i = 0; i < keySize; i++) {
      std::cout << std::hex << static_cast<int>(key[i]) << " ";
    }
    std::cout << std::endl;
    if (status == 1) {  // we got a match, can stop
      std::cout << "This key seems to be valid" << std::endl;
      ok = true;
      break;
    }
  }
  int returnCode = ok ? 1 : 0;  // at this point we know if the key was valid or not
  if (ok) {  // if it was valid, the next question is whether we've seen it before
    returnCode = checkList(seed);
  } else {  // lets try to translate this into a command message
    std::cout << "See if this is a more complex encoded message" << std::endl;
    char * message = 0;
    for (int index = 0; index < numberOfLocks; index++) {
      int size = decoderList[index]->translate(buffer, &message, candidateKeySize);
      if (strncmp(message, lockList[index]->nameOf(), size) == 0) {
        returnCode = checkList(seed);
        break;
      }
    }
  }
  std::cout << "check with locks returns status of " << returnCode << std::endl;
  return returnCode;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      locks - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
locks::locks(const int * lockParameters, const char ** names) {
  numberOfLocks = 0;
  numberOfKeys = 0;
  int configurationParameters[3];
  if (lockParameters) {
    while (*lockParameters != 0) {
      configurationParameters[0] = *lockParameters++;  // slope
      configurationParameters[1] = *lockParameters++;  // offset
      configurationParameters[2] = *lockParameters++;  // modulo
      lockList[numberOfLocks] = new lock(configurationParameters, *names);
      decoderList[numberOfLocks] = new decodeTextUtil(configurationParameters, *names);
      numberOfLocks++;
      names++;
      if (numberOfLocks >= MAX_NUMBER_OF_LOCKS) {
        fprintf(stderr, "Lock configuration is illegal - exceeded configured limit\n");
        exit(-1);
      }
    }
  } else {
    fprintf(stderr, "Lock configuration is illegal - no locks defined\n");
    exit(-1);
  }
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
 *      locks - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
locks::~locks() {
  for (int index = 0; index < numberOfLocks; index++) {
    delete lockList[index];
    delete decoderList[index];
  }
}
/* ---------------------------------------------------------------------- */

