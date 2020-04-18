#ifndef INCLUDE_IMAPCONTROL_H_
#define INCLUDE_IMAPCONTROL_H_
/*
 *      imapControl.h - header for imapControl object
 *
 *      Copyright (C) 2020
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <iostream>
#include <regex>
#include <string>
#include "locks.h"
/* ---------------------------------------------------------------------- */
class imapControl {
 private:
  CURL *examineMailbox;
  CURL *getEmail;
  int newestUID;
  int oldUID;
  int highestmodseq;
  int oldHighestmodseq;
  int serialNumber;
  int oldSerialNumber;
  void doCommand(void);
  int parseExamineRequest(void);
  int verifyPattern(std::string digits);
  int parseEmail(void);

 public:
  struct BufferInfoT {
    char *buffer;
    size_t size;
  };

  struct BufferInfoT bufferInfo;
  locks * locksObject;
  int run(void);
  imapControl();
  ~imapControl();
};
#define STRINGIZER(arg)  #arg
#define STR_VALUE(arg) STRINGIZER(arg)
static bool debug = DEBUG;
static int  pollRate = POLL;
size_t static acceptReply(void *whatCameBack, size_t size, size_t blocks, void *myPointer);
#endif  // INCLUDE_IMAPCONTROL_H_
