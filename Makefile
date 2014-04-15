CFLAGS=-g -O2 -Wall
LDFLAGS=
INCS = `libpng-config --cflags`
LIBS = `libpng-config --libs` -lgd -lm

all: test readme

s2png:
	$(CC) -o s2png rc4.c s2png.c $(CFLAGS) $(LDFLAGS) $(INCS) $(LIBS)

install:
	cp s2png /usr/local/bin

clean:
	rm -f *.o s2png

readme:
	./scripts/gen-readme.sh

test: s2png
	./scripts/test.sh
