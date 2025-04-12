CC = gcc
CFLAGS = -Wall -fPIC -O2
LDFLAGS = -shared -ldl
TARGET = libcapi.so
SRC = libcapi.c
HDR = libcapi.h
INSTALL_DIR = /usr/local/lib

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

install: $(TARGET)
	sudo cp $(TARGET) $(INSTALL_DIR)
	sudo ldconfig

clean:
	rm -f $(TARGET)
