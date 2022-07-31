CC ?= gcc
ARCH ?= $(shell uname -m)
CFLAGS ?= -O2 -fstack-protector-strong -fno-plt -fstack-clash-protection -mharden-sls=all
LDFLAGS ?= -s -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now,-z,noexecstack

PREFIX ?= /usr
LIB_PATH := $(PREFIX)/lib
HEADER_PATH := $(PREFIX)/include

CFLAGS += -fPIC
LDFLAGS += -shared -fvisibility=hidden

TARGET := libarc4random.so
OBJECTS := arc4random.o
HEADERS := arc4random.h

ifeq ($(ARCH),x86_64)
CFLAGS += -maes
OBJECTS += aes-stream/src/aes-stream.o
else ifeq ($(ARCH),amd64)
CFLAGS += -maes
OBJECTS += aes-stream/src/aes-stream.o
else ifeq ($(ARCH),i686)
CFLAGS += -maes
OBJECTS += aes-stream/src/aes-stream.o
else ifeq ($(ARCH),i386)
CFLAGS += -maes
OBJECTS += aes-stream/src/aes-stream.o
endif

.PHONY: all
all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: install
install: $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)$(LIB_PATH)
	install -m 644 $(HEADERS) $(DESTDIR)$(HEADER_PATH)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
