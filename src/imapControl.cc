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
#include "imapControl.h"
#include "imapControlConfig.h"
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
    fprintf(stderr,
            " About to copy what came back into storage buffer - old size: %d, new size: %d, length: %d\n",
            workingPointer->size, completeReturnBufferSize + workingPointer->size, completeReturnBufferSize);
  memcpy(&workingPointer->buffer[workingPointer->size-1], whatCameBack,
         completeReturnBufferSize);
  workingPointer->size += completeReturnBufferSize;
  workingPointer->buffer[workingPointer->size-1] = 0;  // terminate "string"

  if (debug) fprintf(stderr, "Reply:\n %s\n", workingPointer->buffer);

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
  fprintf(stdout, "Issuing command\n");
  char startPath[] = STR_VALUE(START_PATH);
  system(startPath);
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
  std::regex existsPat(" ([0-9]+) EXISTS");
  std::regex highestmodseqPat("HIGHESTMODSEQ ([0-9]+)");
  std::cmatch matches;
  if (!bufferInfo.buffer) {
    std::cerr << "No buffer to examine - curl operation to examine mailbox must have failed" << std::endl;
    exit(1);
  }
  std::regex_search(bufferInfo.buffer, matches, existsPat);
  if (debug) {
    for (int i = 0; i < static_cast<int>(matches.size()); i++) {
      std::cerr << matches[i] << std::endl;
    }
  }
  if (matches.size() != 2) return 1;
  newestUID = std::stoi(matches[1]);
  std::regex_search(bufferInfo.buffer, matches, highestmodseqPat);
  if (debug) {
    for (int i = 0; i < static_cast<int>(matches.size()); i++) {
      std::cerr << matches[i] << std::endl;
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
    oldHighestmodseq = highestmodseq;
    for (int uid = newestUID; uid > 0; uid--) {
      snprintf(newURL, sizeof(newURL), "imaps://" STR_VALUE(IMAP_URL)"/INBOX;UID=%d", uid);
      free(bufferInfo.buffer);
      bufferInfo.buffer = 0;
      bufferInfo.size = 1;
      if (!debug) {
        curl_easy_setopt(getEmail, CURLOPT_URL, newURL);
      }
      returnCode = curl_easy_perform(getEmail);
      if (returnCode) {
        fprintf(stderr, "Mailbox read failed: %d\n", returnCode);
        if (returnCode == 78) {  // did not find the email at this UID
          continue;
        }
        return 3;
      }
      returnCode = parseEmail();
      if (returnCode > 1) {
        fprintf(stderr, "parsing of email found nothing to trigger a command: %d\n", returnCode);
        break;
      }
      if (returnCode == 1) {  // command found
        doCommand();
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
  debug = 1;
  int lockParameters[] = {1807, 45289, 214326, 0, 0, 0};
  const char * names[] = {"action1", 0};
  locksObject = new locks(lockParameters, names);
  bufferInfo.buffer = 0;
  bufferInfo.size = 1;
  newestUID = 0;
  serialNumber = 0;
  oldSerialNumber = 0;
  examineMailbox = curl_easy_init();
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
    std::cerr << "imapControl failed to initialize - could not read mailbox information" << std::endl;
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

  std::cout << "IMAP Control VERSION " << imapControl_VERSION_NUMBER << "." << std::endl;
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

  std::cout << "Entering main processing loop" << std::endl;
  while (!doneProcessing) {
    sleep(30);
    doneProcessing = imapControlInstance.run();
    std::cout << "Cycle completed - status: " << doneProcessing << std::endl;
  }

  return 0;
}
/* ---------------------------------------------------------------------- */