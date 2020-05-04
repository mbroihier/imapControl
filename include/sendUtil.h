#ifndef INCLUDE_SENDUTIL_H_
#define INCLUDE_SENDUTIL_H_
/*
 *      sendUtil.h - header for send utility object
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
#include "sendUtilConfig.h"
/* ---------------------------------------------------------------------- */
#define STRINGIZER(arg)  #arg
#define STR_VALUE(arg) STRINGIZER(arg)
const char url[] = STR_VALUE(PROTOCOL) "//" STR_VALUE(URL);
const char source_destination[] = STR_VALUE(SOURCE_DESTINATION);
const char credentials[] = STR_VALUE(CREDENTIALS);
class sendUtil {
 public:
  static const bool debug = DEBUG;
 private:
  CURL *sendMail;

 public:
  void send(void);
  sendUtil(const char * subjectOfMessage, char * message);
  ~sendUtil();
};
#endif  // INCLUDE_SENDUTIL_H_
