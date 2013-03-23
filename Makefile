CC = cc
CFLAGS=-g -O2
INCS = `libpng-config --cflags`
LIBS = `libpng-config --libs` -lgd -lm

all: test
	
main: rc4.o s2png.o
	$(CC) $(CFLAGS) -o s2png s2png.o rc4.o $(INCS) $(LIBS)

rc4.o: rc4.c rc4.h

s2png.o: s2png.c rc4.h

install:
	cp s2png /usr/local/bin

clean:
	rm -f *.o s2png

test: main
	./test.sh
