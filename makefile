PARAMS_LIBS = -g -lm -lstdc++ -lcurl

INCLUDES = -I./include

CC=gcc

CFLAGS= -c -Wall -DLE_MACHINE -D_GNU_SOURCE
CXX = $(CC)
CXXFLAGS = $(CFLAGS) # set these flags for use of suffix rules for cc
LDFLAGS= $(PARAMS_LIBS)

COMMONSRC=src/lock.cc src/locks.cc src/decodeTextUtil.cc src/encodeTextUtil.cc src/duplicateKeys.cc
COMMONOBJ=$(COMMONSRC:.cc=.o)
IMAPCONTROLSRC = src/imapControl.cc
IMAPCONTROLOBJ=$(IMAPCONTROLSRC:.cc=.o)
ENCODESRC = src/encodeText.cc 
ENCODEOBJ=$(ENCODESRC:.cc=.o)
DECODESRC = src/decodeText.cc 
DECODEOBJ=$(DECODESRC:.cc=.o)
SENDCMDSRC = src/sendCommand.cc 
SENDCMDOBJ=$(SENDCMDSRC:.cc=.o)

EXECUTABLE=imapControl encodeText decodeText sendCommand

all: $(EXECUTABLE)

$(EXECUTABLE): $(COMMONSRC) $(COMMONOBJ) $(IMAPCONTROLSRC) $(IMAPCONTROLOBJ) $(ENCODESRC) $(ENCODEOBJ) $(DECODESRC) $(DECODEOBJ) $(SENDCMDSRC) $(SENDCMDOBJ)
	$(CC) $(LDFLAGS) $(IMAPCONTROLOBJ) $(COMMONOBJ) -o imapControl
	$(CC) $(LDFLAGS) $(ENCODEOBJ) $(COMMONOBJ) -o encodeText
	$(CC) $(LDFLAGS) $(DECODEOBJ) $(COMMONOBJ) -o decodeText
	$(CC) $(LDFLAGS) $(SENDCMDOBJ) $(COMMONOBJ) -o sendCommand

$(COMMONOBJ) : $(COMMONSRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(IMAPCONTROLOBJ) : $(IMAPCONTROLSRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(ENCODEOBJ) : $(ENCODESRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(DECODEOBJ) : $(DECODESRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(SENDCMDOBJ) : $(SENDCMDSRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@

clean:
	rm -fr $(IMAPCONTROLOBJ) $(COMMONOBJ) $(ENCODEOBJ) $(DECODEOBJ) $(SENDCMDOBJ) $(EXECUTABLE) *.o
