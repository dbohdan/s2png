CC = cc
CFLAGS=-g -O2
INCS = `libpng-config --cflags`
LIBS = `libpng-config --libs` -lgd -lm

all:
	$(CC) $(CFLAGS) s2png.c -o s2png $(INCS) $(LIBS)

install:
	cp s2png /usr/local/bin

clean:
	rm -f *.o s2png

