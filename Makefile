CC = gcc
PKG_DEPS = cairo x11 xext xfixes xrandr
CFLAGS := $(shell pkg-config --cflags $(PKG_DEPS)) -O2 -Wall -Wextra -pedantic
LDFLAGS := $(shell pkg-config --libs $(PKG_DEPS))

TARGET = clipscreen
SOURCES = clipscreen.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

format:
	clang-format -i $(SOURCES)

clean:
	rm -f $(TARGET)
