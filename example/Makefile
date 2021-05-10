
CFLAGS = -Os -m64 -g3 -g -march=native --std=c17
foo:
	clang $(CFLAGS) -o u main.c -L /usr/local/lib64 -luCurses
#	strip -R .comment u
clean:
	rm -f u

u:
	make --silent foo

run: u
	LD_LIBRARY_PATH=/usr/local/lib ./u
