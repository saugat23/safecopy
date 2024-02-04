CFLAGS = -Wall -pedantic -std=gnu99
LIBS = `pkg-config --cflags --libs glib-2.0 libnotify`

all: safecopy

safecopy: safecopy.c | build
	gcc $(CFLAGS) $(LIBS) $< -o build/$@

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: clean
