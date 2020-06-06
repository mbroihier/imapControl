/*
 *      sendCommand - send command via mail
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <getopt.h>
#include <unistd.h>
#include "sendCommandConfig.h"
#include "sendCommand.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] = "\n"
        "Usage: %s <message> [lock index]\n"
        "  <message>                : text string that is the message being sent to imapControl\n"
        "  lock index               : optional lock index that indicate which lock to use to encode the message\n"
        "  -h                       : help\n"
        "  --help                   : help\n";

static struct option longOpts[] = {
  {"help", no_argument, NULL, 1},
  { NULL, 0, NULL, 0 }
};

static char payload[1024*8];
static int lineNumber;

static const char * message[] = {
                           "Subject: CI\r\n",
                           "\r\n",
                           "payload",
                           0 };

/* ---------------------------------------------------------------------- */
/*
 *      sendInfo.c -- callback that uploads an email message to a SMTP
 *                    server
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */


size_t static sendInfo(void *whatsGoing,
                          size_t size, size_t blocks, void *myPointer) {
  if (debug) fprintf(stdout, "Entering upload callback\n");
  int line = *(reinterpret_cast<int *>(myPointer));
  if (debug) fprintf(stdout, "Working on line %d\n", line);
  const char * workingPointer = message[line++];
  *(reinterpret_cast<int *>(myPointer)) = line;
  size_t completeReturnBufferSize = size * blocks;
  size_t len = 0;
  if (workingPointer) {
    len = strlen(workingPointer);
    if (debug)
      fprintf(stdout,
            " About to copy whats going out into storage buffer - message size: %lu, buffer size: %lu\n",
            len, completeReturnBufferSize);
    if (len > 0) {
      if (strncmp(workingPointer, "payload", sizeof("payload")) == 0) {
        if (debug) fprintf(stdout, "copy payload to whatsGoing out (ie, the new information: %s\n", payload);
        memcpy(whatsGoing, payload, strlen(payload)+1);
        len = strlen(payload);  // change returned length to that of the payload
      } else {
        if (len < completeReturnBufferSize) {
          memcpy(whatsGoing, workingPointer, len);
        } else {
          len = 0;
        }
      }
    }
    if (debug) fprintf(stdout, "Sending:\n %s\n", whatsGoing);
  } else {
    if (debug) fprintf(stdout, "End of message\n");
  }
  return len;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      getAndEncodeMessage.cc -- encode the message from the command line
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
void sendCommand::getAndEncodeMessage(const char * message, int lockIndex) {
  int lockParameters[] = {LOCK_PARAMETERS};
  const char * names[] = {LOCK_NAMES};
  char * binaryPayload = 0;

  encodeTextUtil textEncoderInstance(lockParameters, names, lockIndex);
  int size = textEncoderInstance.translate(message, &binaryPayload);
  int n = snprintf(payload, sizeof(payload), "COMMAND_STREAM {");
  for (int index = 0; index < size; index++) {
    if (index == 0) {
      n += snprintf(payload+n, sizeof(payload), " 0x%2.2x", static_cast<unsigned char>(binaryPayload[index]));
    } else {
      n += snprintf(payload+n, sizeof(payload), ", 0x%2.2x", static_cast<unsigned char>(binaryPayload[index]));
    }
  }
  snprintf(payload+n, sizeof(payload), " }\n");
  free(binaryPayload);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      send.cc -- execute the commands to send an email
 *                    message
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
CURLcode sendCommand::send() {
  lineNumber = 0;
  sendMail = curl_easy_init();
  curl_easy_setopt(sendMail, CURLOPT_URL, url);
  curl_easy_setopt(sendMail, CURLOPT_USERPWD,  credentials);
  curl_easy_setopt(sendMail, CURLOPT_USE_SSL, CURLUSESSL_ALL);
  curl_easy_setopt(sendMail, CURLOPT_MAIL_FROM, source_destination);
  struct curl_slist *list;
  list = curl_slist_append(NULL, source_destination);
  curl_easy_setopt(sendMail, CURLOPT_MAIL_RCPT, list);
  curl_easy_setopt(sendMail, CURLOPT_READFUNCTION, sendInfo);
  curl_easy_setopt(sendMail, CURLOPT_READDATA, & lineNumber);
  curl_easy_setopt(sendMail, CURLOPT_UPLOAD, 1L);
  if (debug) {
    curl_easy_setopt(sendMail, CURLOPT_VERBOSE, 1L);
  }
  CURLcode result = curl_easy_perform(sendMail);
  curl_slist_free_all(list);
  curl_easy_cleanup(sendMail);
  return result;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      sendCommand - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

sendCommand::sendCommand() {
  if (debug) fprintf(stdout, "%s %s %s \n", url, source_destination, credentials);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      sendCommand - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

sendCommand::~sendCommand() {
}
/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {
  lineNumber = 0;
  sendCommand sendCommandInstance;
  int c;

  memset(payload, 0, sizeof(payload));
  memcpy(payload, "test", sizeof("test"));

  fprintf(stdout, "sendCommand VERSION %s\n", STR_VALUE(sendCommand_VERSION_NUMBER));
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

  int index = 0;
  if (argc == 3) {
    index = std::stoi(argv[2]);
  }
  sendCommandInstance.getAndEncodeMessage(argv[1], index);
  if (CURLE_OK != sendCommandInstance.send()) {
    fprintf(stderr, "sendCommand Failed\n");
  } else {
    fprintf(stdout, "sendCommand Successful\n");
  }
  return 0;
}
/* ---------------------------------------------------------------------- */
