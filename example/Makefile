
CFLAGS = -Os -m64 -g3 -g -march=native 
foo:
	clang $(CFLAGS) -o u main.c -LLIBDIR=/usr/local/lib -luCurses
#	strip -R .comment u
clean:
	rm -f u
	
	