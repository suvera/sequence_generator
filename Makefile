BUILDS=builds
PREFIX_DEFAULT=/usr/local
bindir=$(PREFIX_DEFAULT)/bin
CONFIG_INI=
PID_CMD=
GET_PID_FILE="abcd"
SEQ_HOME=/var/lib/sequencer
CC=g++
CFLAGS= -Wno-write-strings -std=c++11 -I./src
INCFLAGS= -I./cmdline -I./easyloggingpp/src -pthread
UUIDFLAGS= -luuid

ifdef prefix
bindir=$(prefix)/bin
mkdir -p $(prefix)
mkdir -p $(bindir)
endif

GET_PID_FILE=$(BUILDS)/sequencerd -I

ifdef config
CONFIG_INI=$(config)
GET_PID_FILE=$(BUILDS)/sequencerd -I -c $(CONFIG_INI)
else
$(shell mkdir -p $(SEQ_HOME))
$(shell mkdir -p $(SEQ_HOME)/data)
endif

all: main.cpp
	mkdir -p $(BUILDS)
	$(CC) $(CFLAGS) $(INCFLAGS) main.cpp $(UUIDFLAGS) -o $(BUILDS)/sequencerd
	chmod +x $(BUILDS)/sequencerd
	$(CC) $(CFLAGS) startup.cpp -o $(BUILDS)/startup
	chmod +x $(BUILDS)/startup

install: all
	cp $(BUILDS)/sequencerd $(bindir)/sequencerd
	$(BUILDS)/startup $(bindir)/sequencerd "$(CONFIG_INI)" "$(shell $(GET_PID_FILE))" > $(bindir)/sequencer
	chmod +x $(bindir)/sequencerd
	chmod +x $(bindir)/sequencer
	@echo
	@echo "Successfully installed."

clean:
	rm -f $(BUILDS)/*