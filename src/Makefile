GET=i386-mingw32- #for mac
CC=$(TARGET)gcc
LD=$(TARGET)gcc
WINDRES=$(TARGET)windres
CFLAGS=-std=c99 -O3 -DWINVER=0x500 -DWIN32_WINNT=0x500
LDFLAGS+=-mwindows
OBJECTS=config.o init-layout.o key-handler.o main.o resources.o states.o trayicon.o
ifdef DEBUG
	CFLAGS+= -g
	LDFLAGS:=$(filter-out -mwindows, $(LDFLAGS))
endif

all: neo-llkh.exe

neo-llkh.exe: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.rc
	$(WINDRES) -i $^ -o $@

clean:
	@rm -f $(OBJECTS) neo-llkh.exe
