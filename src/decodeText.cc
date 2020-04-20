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
#include "decodeTextConfig.h"
#include "decodeTextUtil.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] =
        "Usage: %s <byte string>\n"
        "  a quoted text string containing hex bytes that are encoded messages\n"
        "  -h                       : help\n"
        "  --help                   : help\n";

static struct option longOpts[] = {
  {"help", no_argument, NULL, 1},
  { NULL, 0, NULL, 0 }
};

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
        fprintf(stderr, "Text Decode VERSION %s\n", STR_VALUE(decodeText_VERSION_NUMBER));
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      case 1: {
        fprintf(stderr, "Text Decode VERSION %s\n", STR_VALUE(decodeText_VERSION_NUMBER));
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      default:
        break;
    }
  }

  const char * incomingString = argv[1];
  char * incomingBytes = reinterpret_cast<char *>(malloc(2));

  char * translatedString = 0;

  int lockParameters[] = {1807, 45289, 214326, 0};
  const char * names[] = {"encoder", 0};

  decodeTextUtil textDecoderUtilInstance(lockParameters, *names);

  std::regex digitPattern(" 0*x*[0-9a-fA-F]{2}");
  std::string remainingIncomingString = incomingString;
  std::cmatch digits;
  int index = 0;
  bool none = false;
  do {
    const char * remainingIncoming = remainingIncomingString.c_str();
    if (std::regex_search(remainingIncoming, digits, digitPattern)) {
      incomingBytes[index] = std::stoi(digits[0], 0, 16);
      incomingBytes[index+1] = 0;
      remainingIncomingString = digits[0].second;
      index++;
      incomingBytes = reinterpret_cast<char *>(realloc(incomingBytes, 2 + index));
    } else {
      none = true;
    }
  } while (!none);
  int size = index;
  size = textDecoderUtilInstance.translate(incomingBytes, &translatedString, size);

  fprintf(stdout, "%s\n", translatedString);

  free(translatedString);
  free(incomingBytes);

  return 0;
}
/* ---------------------------------------------------------------------- */
