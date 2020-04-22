PARAMS_LIBS = -g -lm -lstdc++ -lfftw3 -lcurl

INCLUDES = -I./include

CC=gcc

CFLAGS= $(if $(shell uname -a | grep -i armv), -c -Wall -DLE_MACHINE -D_GNU_SOURCE $(PARAMS_LOOPVECT) $(PARAMS_SIMD) $(PARAMS_MISC), -c -Wall -DLE_MACHINE -D_GNU_SOURCE )
CXX = $(CC)
CXXFLAGS = $(CFLAGS) # set these flags for use of suffix rules for cc
LDFLAGS= $(PARAMS_LIBS)

COMMONSRC=src/lock.cc src/locks.cc src/decodeTextUtil.cc src/duplicateKeys.cc
COMMONOBJ=$(COMMONSRC:.cc=.o)
IMAPCONTROLSRC = src/imapControl.cc
IMAPCONTROLOBJ=$(IMAPCONTROLSRC:.cc=.o)
ENCODESRC = src/encodeText.cc 
ENCODEOBJ=$(ENCODESRC:.cc=.o)
DECODESRC = src/decodeText.cc 
DECODEOBJ=$(DECODESRC:.cc=.o)

EXECUTABLE=imapControl encodeText decodeText

all: $(EXECUTABLE)

$(EXECUTABLE): $(COMMONSRC) $(COMMONOBJ) $(IMAPCONTROLSRC) $(IMAPCONTROLOBJ) $(ENCODESRC) $(ENCODEOBJ) $(DECODESRC) $(DECODEOBJ)
	$(CC) $(LDFLAGS) $(IMAPCONTROLOBJ) $(COMMONOBJ) -o imapControl
	$(CC) $(LDFLAGS) $(ENCODEOBJ) $(COMMONOBJ) -o encodeText
	$(CC) $(LDFLAGS) $(DECODEOBJ) $(COMMONOBJ) -o decodeText

$(COMMONOBJ) : $(COMMONSRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(IMAPCONTROLOBJ) : $(IMAPCONTROLSRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(ENCODEOBJ) : $(ENCODESRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@
$(DECODEOBJ) : $(DECODESRC)
	$(CC) $(CFLAGS) $(INCLUDES) $*.cc -o $@

clean:
	rm -fr $(IMAPCONTROLOBJ) $(COMMONOBJ) $(ENCODEOBJ) $(DECODEOBJ) $(EXECUTABLE) *.o