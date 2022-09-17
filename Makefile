CC ?= gcc
ARCH ?= $(shell uname -m)
CFLAGS ?= -march=native -O2 -fstack-protector-strong -fno-plt -fstack-clash-protection -mharden-sls=all -falign-functions=32
LDFLAGS ?= -s -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now,-z,noexecstack

PREFIX ?= /usr
LIB_PATH := $(PREFIX)/lib
HEADER_PATH := $(PREFIX)/include

CFLAGS += -fPIC -fvisibility=hidden
LDFLAGS += -shared -lcrypto

TARGET := libarc4random.so
OBJECTS := arc4random.o
HEADERS := arc4random.h

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
