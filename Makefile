CC = gcc
CFLAGS = -Wall -fPIC -O2
LDFLAGS = -shared -ldl
TARGET = libcapi.so
SRC = libcapi.c
HDR = libcapi.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
