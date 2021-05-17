
OBJDIR=obj
CFLAGS = -Os -m64 -g3 -g -march=native --std=c17
CC = clang

SOURCES = main.c menus.c demo1.c dots.c sintab.c shapes.c mandel.c

objects = $(addprefix $(OBJDIR)/, main.o menus.o demo1.o dots.o sintab.o shapes.o mandel.o)

$(shell   mkdir -p obj)

demo : $(objects)
	$(CC) -L /usr/local/lib64 -luCurses -o demo $(objects)
#	strip -R .comment demo

$(OBJDIR)/%.o : %.c demo.h $(SOURCES)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -fr obj demo

run: demo
	LD_LIBRARY_PATH=/usr/local/lib64 ./demo
