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


static const char USAGE_STR[] =
        "Usage: %s <text string>\n"
        "  a text string enclosed with quotes\n"
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

  int lockParameters[] = {LOCK_PARAMETERS};
  const char * names[] = {LOCK_NAMES};
  encodeTextUtil textEncoderInstance(lockParameters, names, 0);

  int size = textEncoderInstance.translate(incomingString, &translatedString);

  fprintf(stdout, "{ ");
  for (int index = 0; index < size; index++) {
    if (index == 0) {
      fprintf(stdout, "0x%2.2x", static_cast<unsigned char>(translatedString[index]));
    } else {
      fprintf(stdout, ", 0x%2.2x", static_cast<unsigned char>(translatedString[index]));
    }
  }
  fprintf(stdout, " }\n");

  free(translatedString);

  return 0;
}

