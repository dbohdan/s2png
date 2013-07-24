CFLAGS=-g -O2 -Wall
INCS = `libpng-config --cflags`
LIBS = `libpng-config --libs` -lgd -lm

all: test readme

s2png: rc4.o s2png.o
	$(CC) $^ -o $@ $(CFLAGS) $(INCS) $(LIBS)

install:
	cp s2png /usr/local/bin

clean:
	rm -f *.o s2png

readme:
	./scripts/gen-readme.sh

test: s2png
	./scripts/test.sh
