# imapControl

This repository contains code that implements a program that executes remote commands sent by email from an IMAP server.  There is a sendCommand utility that sends an encoded command to the server and there is imapControl service that monitors the IMAP mailbox, examines messages, decodes the command, and, if it is valid, executes the command.  Messages are encoded uniquely using rolling random numbers and imapControl requires that each message it processes is unique.  This prevents spoofing of command messages from a nonauthorized source.

The IMAP server and mailbox can be configured in the CMakeLists.txt file.  Do not commit a modified version of this file to your cloned copy of this repository.  It will contain authentication information for accessing an email account.

The rolling code parameters in the CMakeLists.txt file should be altered to produce your own unique number patterns although some randomization is done internally.  The patterns are adjusted by modifying the LOCK_PARAMETERS which are in groups of three integers: a slope, a constant, and a modulo.  You want the modulo to be large and less than 2^31 - 1 since this works with 32 bit integers.  Every three parameters defines the characteristics for a "lock", a trailing zero stops the lock definitions.  Every lock can have a name, and every lock can be associated with a program to run (a path name to an executable).

When sendCommand "lockname" is run,  the "lockname" is encoded and sent to the IMAP server.  imapControl polls the server, sees that there is a new message, and checks the message to see if it has an encoded "lockname".  If it does, it exeucted the code at the path associated with the name.



Installation (assuming a Raspbian buster or better installation - should run on other Debian Linux machines also)

  1)  sudo apt-get update
  2)  sudo apt-get upgrade
  3)  sudo apt-get install git
  4)  git clone https://github.com/mbroihier/imapControl
      - cd imapControl
      - mkdir build
      - vi CMakeLists.txt and modify parameters discussed above
      - cd build
      - cmake ..
      - make
  5)  if you want imapControl to run as a service
      - vi imapControl.service - modify to point to your local paths
      - vi imapControlWrapper - modify to point to your local paths

