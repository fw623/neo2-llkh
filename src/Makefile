GET=i386-mingw32- #for mac
CC=$(TARGET)gcc
LD=$(TARGET)gcc
WINDRES=$(TARGET)windres
CFLAGS=-std=c99 -O3 -DWINVER=0x500 -DWIN32_WINNT=0x500
LDFLAGS+=-mwindows
OBJECTS=config.o dfk.o mouse.o init-layout.o key-handler.o main.o resources.o states.o tray-icon.o
DIST=../dist
ifdef DEBUG
	CFLAGS+= -g
	LDFLAGS:=$(filter-out -mwindows, $(LDFLAGS))
endif

all: $(DIST)/neo-llkh.exe

$(DIST)/neo-llkh.exe: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.rc
	$(WINDRES) -i $^ -o $@ -I.

clean:
	@rm -f $(OBJECTS) neo-llkh.exe
