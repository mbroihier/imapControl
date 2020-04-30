#ifndef INCLUDE_SENDCOMMAND_H_
#define INCLUDE_SENDCOMMAND_H_
/*
 *      sendCommand.h - header for sendCommand object
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
#include <string>
#include "encodeTextUtil.h"
/* ---------------------------------------------------------------------- */
#define STRINGIZER(arg)  #arg
#define STR_VALUE(arg) STRINGIZER(arg)
const char url[] = STR_VALUE(PROTOCOL) "//" STR_VALUE(URL);
const char source_destination[] = STR_VALUE(SOURCE_DESTINATION);
const char credentials[] = STR_VALUE(CREDENTIALS);
static bool debug = DEBUG;
class sendCommand {
 private:
  CURL *sendMail;

 public:
  void send(void);
  void getAndEncodeMessage(const char * message, int lockIndex);
  sendCommand();
  ~sendCommand();
};
#endif  // INCLUDE_SENDCOMMAND_H_
