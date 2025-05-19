/*
 *      sendMessage - send a message via mail
 *
 *      Copyright (C) 2025
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <getopt.h>
#include <unistd.h>
#include "sendMessageConfig.h"
#include "sendMessage.h"
#include "sendUtil.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] = "\n"
        "Usage: %s <message> \n"
        "  <message>                : text string that is a message being by imapControl processes\n"
        "  -h                       : help\n"
        "  --help                   : help\n";

static struct option longOpts[] = {
  {"help", no_argument, NULL, 1},
  { NULL, 0, NULL, 0 }
};

int main(int argc, char *argv[]) {
  int c;

  fprintf(stdout, "sendMessage VERSION %s\n", STR_VALUE(sendMessage_VERSION_NUMBER));
  if (argc != 2 && argc !=3) {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  while ((c = getopt_long(argc, argv, "h", longOpts, NULL)) >= 0) {
    switch (c) {
      case 'h': {
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      case 1 : {
        fprintf(stderr, USAGE_STR, argv[0]);
      }
      default:
        break;
    }
  }

  sendUtil * sendMessageInstance = new sendUtil("Subject: Message from imapControl processing", argv[1]);
  if (CURLE_OK != sendMessageInstance->send()) {
    fprintf(stderr, "sendMessage Failed\n");
  } else {
    fprintf(stdout, "sendMessage Successful\n");
  }
  delete(sendMessageInstance);
  return 0;
}
/* ---------------------------------------------------------------------- */
