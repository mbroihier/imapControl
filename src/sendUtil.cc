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
#include "sendUtilConfig.h"
#include "sendUtil.h"
/* ---------------------------------------------------------------------- */


static char payload[1024*8];
static char subject[128];
static int lineNumber;

static const char * message[] = {
                           "Subject",
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
  if (sendUtil::debug) fprintf(stdout, "Entering upload callback\n");
  int line = *(reinterpret_cast<int *>(myPointer));
  if (sendUtil::debug) fprintf(stdout, "Working on line %d\n", line);
  const char * workingPointer = message[line++];
  *(reinterpret_cast<int *>(myPointer)) = line;
  size_t completeReturnBufferSize = size * blocks;
  size_t len = 0;
  if (workingPointer) {
    len = strlen(workingPointer);
    if (sendUtil::debug)
      fprintf(stdout,
            " About to copy whats going out into storage buffer - message size: %lu, buffer size: %lu\n",
            len, completeReturnBufferSize);
    if (len > 0) {
      if (strncmp(workingPointer, "payload", sizeof("payload")) == 0) {
        if (sendUtil::debug) fprintf(stdout, "copy payload to whatsGoing out (ie, the new information) %s\n", payload);
        memcpy(whatsGoing, payload, strlen(payload)+1);
        len = strlen(payload);  // change returned length to that of the payload
      } else if (strncmp(workingPointer, "Subject", sizeof("Subject")) == 0) {
        if (sendUtil::debug) fprintf(stdout, "copy subject to whatsGoing out (ie, the new information) %s\n", subject);
        memcpy(whatsGoing, subject, strlen(subject)+1);
        len = strlen(subject);  // change returned length to that of the payload
      } else {
        if (len < completeReturnBufferSize) {
          memcpy(whatsGoing, workingPointer, len);
        } else {
          len = 0;
        }
      }
    }
    if (sendUtil::debug) fprintf(stdout, "Sending:\n %s\n", whatsGoing);
  } else {
    if (sendUtil::debug) fprintf(stdout, "End of message\n");
  }
  return len;
}

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
CURLcode sendUtil::send() {
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
 *      sendUtil - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

sendUtil::sendUtil(const char * subjectOfMessage, char * message) {
  if (debug) fprintf(stdout, "%s %s %s \n", url, source_destination, credentials);
  strncpy(subject, subjectOfMessage, sizeof(subject));
  strncpy(payload, message, sizeof(payload));
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      sendUtil - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
sendUtil::~sendUtil() {
}
/* ---------------------------------------------------------------------- */
