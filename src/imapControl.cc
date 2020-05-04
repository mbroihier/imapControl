/*
 *      imapControl - when contacted, activate script
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <getopt.h>
#include <unistd.h>
#include "imapControlConfig.h"
#include "imapControl.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] = "\n"
        "Usage: %s \n"
        "  -h                       : help - no parameters - just does it\n"
        "  --help                   : help - no parameters - just does it\n";

static struct option longOpts[] = {
  {"help", no_argument, NULL, 1},
  { NULL, 0, NULL, 0 }
};

/* ---------------------------------------------------------------------- */
/*
 *      acceptReply.c -- callback that puts remote reply into memory
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */


size_t static acceptReply(void *whatCameBack,
                          size_t size, size_t blocks, void *myPointer) {
  struct imapControl::BufferInfoT *workingPointer =
    (struct imapControl::BufferInfoT *) myPointer;
  size_t completeReturnBufferSize = size * blocks;
  if (!workingPointer->buffer) {  // if this is the first time being called
    workingPointer->buffer =
      reinterpret_cast<char *>(malloc(completeReturnBufferSize+1));
  } else {
    workingPointer->buffer =
      reinterpret_cast<char *>(realloc(workingPointer->buffer,
                                       completeReturnBufferSize+workingPointer->size));
  }
  if (!workingPointer->buffer) {
    fprintf(stderr, "ran out of memory\n");
    return 0;
  }
  if (debug)
    fprintf(stdout,
            " About to copy what came back into storage buffer - old size: %lu, new size: %lu, length: %lu\n",
            workingPointer->size, completeReturnBufferSize + workingPointer->size, completeReturnBufferSize);
  memcpy(&workingPointer->buffer[workingPointer->size-1], whatCameBack,
         completeReturnBufferSize);
  workingPointer->size += completeReturnBufferSize;
  workingPointer->buffer[workingPointer->size-1] = 0;  // terminate "string"

  if (debug) fprintf(stdout, "Reply:\n %s\n", workingPointer->buffer);

  return completeReturnBufferSize;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      doCommand.cc -- execute a command associated with this control
 *                      message
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
void imapControl::doCommand() {
  const char * startPaths[] = { START_PATHS };
  int index = locksObject->getLastLockMatchedIndex();
  if (index >= 0) {
    fprintf(stdout, "Issuing command: %s\n", startPaths[index]);
    system(startPaths[index]);
  } else {
    fprintf(stderr, "%s %s line %d - Internal error - no index to match command table\n", __FILE__, __func__, __LINE__);
  }
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      parseExamineRequest.cc -- extract the examine mail folder results
 *                                from the query response
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int imapControl::parseExamineRequest() {
  std::regex uidNextPat("UIDNEXT ([0-9]+)");
  std::regex highestmodseqPat("HIGHESTMODSEQ ([0-9]+)");
  std::cmatch matches;
  if (!bufferInfo.buffer) {
    fprintf(stderr, "No buffer to examine - curl operation to examine mailbox must have failed\n");
    exit(1);
  }
  std::regex_search(bufferInfo.buffer, matches, uidNextPat);
  if (debug) {
    for (int i = 0; i < static_cast<int>(matches.size()); i++) {
      fprintf(stdout, "%s\n", matches.str(i).c_str());
    }
  }
  if (matches.size() != 2) return 1;
  newestUID = std::stoi(matches[1]) - 1;
  std::regex_search(bufferInfo.buffer, matches, highestmodseqPat);
  if (debug) {
    for (int i = 0; i < static_cast<int>(matches.size()); i++) {
      fprintf(stdout, "%s\n", matches.str(i).c_str());
    }
  }
  if (matches.size() != 2) return 1;
  highestmodseq = std::stoi(matches[1]);
  return 0;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      parseEmail.cc -- extract the information from the email
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int imapControl::parseEmail() {
  int returnCode = 2;  // default to stop processing
  std::regex commandPat(COMMAND_PATTERN);
  std::cmatch matches;
  std::regex_search(bufferInfo.buffer, matches, commandPat);
  if (!matches.empty()) {
    std::string substring = matches.str(1);  // get digits following text
    returnCode = verifyPattern(substring);
  } else {
    fprintf(stdout, "Skipping email with no apparent command message\n");
    returnCode = 0;
  }
  return returnCode;  // 2 - not ok, stop; 1 - ok process; 0 - ok continue
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      verifyPattern.cc -- verify that the pattern is what we are looking
 *                          for
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
int imapControl::verifyPattern(std::string digits) {
  int returnCode;
  returnCode = locksObject->check(digits);
  return returnCode;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      run.c -- Run the process that checks to see if a command
 *               request has been made
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int imapControl::run() {
  free(bufferInfo.buffer);
  bufferInfo.buffer = 0;
  bufferInfo.size = 1;
  int returnCode = curl_easy_perform(examineMailbox);
  if (returnCode) {
    fprintf(stderr, "Mailbox examination failed: %d\n", returnCode);
    return 1;
  }
  returnCode = parseExamineRequest();
  if (returnCode) {
    fprintf(stderr, "parsing of mailbox examination results failed: %d\n", returnCode);
    return 2;
  }
  if (highestmodseq != oldHighestmodseq) {  // something has changed, see if new mail has a request
    char newURL[1024];
    memset(newURL, 0, sizeof(newURL));
    oldHighestmodseq = highestmodseq;
    int readNoMore = std::max(0, newestUID - MAX_EMAILS);  // limit how many emails are read (no more than MAX_EMAILS)
    for (int uid = newestUID; uid > readNoMore; uid--) {
      free(bufferInfo.buffer);
      bufferInfo.buffer = 0;
      bufferInfo.size = 1;
      if (!debug) {
        snprintf(newURL, sizeof(newURL), "imaps://" STR_VALUE(IMAP_URL)"/INBOX;UID=%d", uid);
        int rc = curl_easy_setopt(getEmail, CURLOPT_URL, newURL);
        if (rc) {
          fprintf(stderr, "curl_easy_setopt returned error! Code: %d\n", rc);
        }
      }
      returnCode = curl_easy_perform(getEmail);
      if (returnCode) {
        fprintf(stderr, "Mailbox read failed: %d\n", returnCode);
        if (returnCode == 78) {  // let's try one more time
          returnCode = curl_easy_perform(getEmail);
        }
        if (returnCode == 78) {  // did not find the email at this UID
          fprintf(stderr, "command was: %s\n", newURL);
          if (bufferInfo.buffer == 0) {
            fprintf(stderr, "no returned data\n");
          } else {
            fprintf(stderr, "received: %s\n", bufferInfo.buffer);
          }
          continue;  // continue with next UID
        } else {
          if (returnCode) {
            return 3;  // fail out - this was bad
          }
          fprintf(stderr, "Reread was successful\n");
        }
      }
      returnCode = parseEmail();
      if (returnCode > 1) {
        fprintf(stdout, "parsing of email found nothing to trigger a command: %d\n", returnCode);
        break;
      }
      if (returnCode == 1) {  // command found
        doCommand();
        sendUtil * sendUtilInstance = new sendUtil("Subject: Command executed", bufferInfo.buffer);
        fprintf(stdout, "%s\n", bufferInfo.buffer);
        sendUtilInstance->send();
        delete(sendUtilInstance);
        break;
      }
    }
  }
  return 0;  // if mail reads worked, return ok
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      imapControl - constructor
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

imapControl::imapControl() {
  int lockParameters[] = {LOCK_PARAMETERS};
  const char * names[] = {LOCK_NAMES};
  locksObject = new locks(lockParameters, names);
  bufferInfo.buffer = 0;
  bufferInfo.size = 1;
  newestUID = 0;
  serialNumber = 0;
  oldSerialNumber = 0;
  examineMailbox = curl_easy_init();
  curl_version_info_data * versionData;
  versionData = curl_version_info(CURLVERSION_NOW);
  if (versionData->version_num < 0x074000) {  // this code wants 7.64.0 or greater
    fprintf(stderr, "This curl version is not new enough - detected version is: %8.8x\n", versionData->version_num);
    exit(1);
  }
  if (debug) {
    fprintf(stdout, "Building a request to examain a pseudo mailbox\n");
    curl_easy_setopt(examineMailbox, CURLOPT_URL, "http://localhost:3000/examine");
    curl_easy_setopt(examineMailbox, CURLOPT_WRITEFUNCTION, acceptReply);
    curl_easy_setopt(examineMailbox, CURLOPT_WRITEDATA, reinterpret_cast<void *>(& bufferInfo));
  } else {
    curl_easy_setopt(examineMailbox, CURLOPT_URL, "imaps://" STR_VALUE(IMAP_URL));
    curl_easy_setopt(examineMailbox, CURLOPT_USERPWD,  STR_VALUE(USER_INFO));
    curl_easy_setopt(examineMailbox, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(examineMailbox, CURLOPT_CUSTOMREQUEST, "EXAMINE INBOX");
    curl_easy_setopt(examineMailbox, CURLOPT_WRITEFUNCTION, acceptReply);
    curl_easy_setopt(examineMailbox, CURLOPT_WRITEDATA, reinterpret_cast<void *>(& bufferInfo));
  }
  curl_easy_perform(examineMailbox);
  if (parseExamineRequest()) {
    fprintf(stderr, "imapControl failed to initialize - could not read mailbox information\n");
    exit(1);
  }
  oldHighestmodseq = highestmodseq;
  getEmail = curl_easy_init();
  if (debug) {
    curl_easy_setopt(getEmail, CURLOPT_URL, "http://localhost:3000/mail");
    curl_easy_setopt(getEmail, CURLOPT_WRITEFUNCTION, acceptReply);
    curl_easy_setopt(getEmail, CURLOPT_WRITEDATA, reinterpret_cast<void *>(& bufferInfo));
  } else {
    curl_easy_setopt(getEmail, CURLOPT_URL, "imaps://" STR_VALUE(IMAP_URL));
    curl_easy_setopt(getEmail, CURLOPT_USERPWD, STR_VALUE(USER_INFO));
    curl_easy_setopt(getEmail, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(getEmail, CURLOPT_WRITEFUNCTION, acceptReply);
    curl_easy_setopt(getEmail, CURLOPT_WRITEDATA, reinterpret_cast<void *>(& bufferInfo));
  }
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
 *      imapControl - destructor
 *
 *      Copyright (C) 2020 
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */

imapControl::~imapControl() {
  curl_easy_cleanup(examineMailbox);
  curl_easy_cleanup(getEmail);
  if (bufferInfo.buffer) {
    free(bufferInfo.buffer);
  }
}
/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {
  imapControl imapControlInstance;

  int c;

  fprintf(stdout, "IMAP Control VERSION %s\n", STR_VALUE(imapControl_VERSION_NUMBER));
  if (argc > 1) {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  int doneProcessing = 0;
  while ((c = getopt_long(argc, argv, "h", longOpts, NULL)) >= 0) {
    switch (c) {
      case 'h': {
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      default:
        break;
    }
  }

  fprintf(stdout, "Entering main processing loop\n");
  while (!doneProcessing) {
    sleep(pollRate);
    doneProcessing = imapControlInstance.run();
    fprintf(stdout, "Cycle completed - status: %d\n", doneProcessing);
  }

  return 0;
}
/* ---------------------------------------------------------------------- */
