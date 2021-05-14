
CFLAGS = -Os -m64 -g3 -g -march=native --std=c17
CC = clang

SOURCES = main.c menus.c demo1.c

%.o: %.d $(SOURCES)
	$(CC) -c -o $@ $< $(CFLAGS)

demo: main.o menus.o demo1.o
	$(CC) -L /usr/local/lib64 -luCurses -o demo main.o menus.o demo1.o
#	strip -R .comment demo

clean:
	rm -f *.o demo

run: u
	LD_LIBRARY_PATH=/usr/local/lib64 ./demo
