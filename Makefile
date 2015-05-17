BUILDS=builds
PREFIX_DEFAULT=/usr/local
bindir=$(PREFIX_DEFAULT)/bin
CC=g++
CFLAGS= -Wno-write-strings -std=c++11 -I./src
INCFLAGS= -I./cmdline -I./easyloggingpp/src -pthread
UUIDFLAGS= -luuid

ifdef prefix
bindir=$(prefix)/bin
endif

all: main.cpp
	mkdir -p $(BUILDS)
	$(CC) $(CFLAGS) $(INCFLAGS) main.cpp $(UUIDFLAGS) -o $(BUILDS)/sequencerd
	$(CC) $(CFLAGS) $(INCFLAGS) startup.cpp $(UUIDFLAGS) -o $(BUILDS)/startup

install: all
	cp $(BUILDS)/sequencerd $(bindir)/sequencerd
	$(BUILDS)/startup $(bindir)/sequencerd > $(bindir)/sequencer
	chmod +x $(bindir)/sequencerd
	chmod +x $(bindir)/sequencer
	@echo
	@echo "Successfully installed."

clean:
	rm -f $(BUILDS)/*