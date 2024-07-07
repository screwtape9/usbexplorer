CC=g++
#CXXFLAGS=-std=c++11 -c -Wall -Wextra -Wshadow -Wredundant-decls -Wunreachable-code -Winline
CXXFLAGS=-std=c++11 -c
INCLUDES=$(shell pkg-config --cflags gtkmm-3.0 libusb-1.0)
LIBS=$(shell pkg-config --libs gtkmm-3.0 libusb-1.0) -ludev

ifeq ($(DEBUG),1)
CXXFLAGS+=-g
endif

OBJ:=usbdevice.o win.o main.o
EXE=usbexplorer

COMPILE.1=$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $<
ifeq ($(VERBOSE),)
COMPILE=@printf "  > compiling %s\n" $(<F) && $(COMPILE.1)
else
COMPILE=$(COMPILE.1)
endif

%.o: %.cpp
	$(COMPILE)

.PHONY: all clean rebuild

all: $(EXE)

$(EXE): $(OBJ) $(OUTPUT_DIR)
	$(CC) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f $(EXE) $(OBJ)

rebuild: clean all
