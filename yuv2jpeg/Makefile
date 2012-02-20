##############################
# spcaview Makefile
##############################

ROOT =$(PWD)

CC=gcc
CPP=g++
INSTALL=install
APP_BINARY=yuv2jpeg
BIN=/usr/local/bin
VERSION = 0.2.1

JPEGLIB = $(ROOT)/jpeglib/jpeg-6b/

CFLAGS +=  -O2
CFLAGS += -I$(JPEGLIB) -ljpeg

CPPFLAGS = $(CFLAGS)

OBJECTS= yuv2jpeg.o


all:	yuv2jpeg

clean:
	@echo "Cleaning up ..."
	rm -f *.a *.o $(APP_BINARY)

# Applications:
yuv2jpeg:	$(OBJECTS)
	$(CC)	$(CFLAGS) $(OBJECTS) $(XPM_LIB) -o $(APP_BINARY)
	chmod 755 $(APP_BINARY)


install: luvcview
	$(INSTALL) -s -m 755 -g root -o root $(APP_BINARY) $(BIN)
	rm -f $(BIN)/uvcview
