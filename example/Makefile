
OBJDIR=obj
CFLAGS = -O3 -m64 -g3 -g -march=native --std=c17 -fno-inline -fPIE

SOURCES = main.c menus.c hello.c

OBJECTS = $(addprefix $(OBJDIR)/, main.o menus.o hello.o)

$(shell   mkdir -p obj)

demo : $(OBJECTS) window raycast dots mandel lion
	$(CC) -L /usr/local/lib64 -o demo $(OBJECTS) -lm -luCurses 
#	strip -R .comment demo

$(OBJDIR)/%.o : %.c demo.h $(SOURCES)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -fr obj demo shell wtest window raycast dots mandel lion *.o

shell: shell.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o shell shell.c

wtest: wtest.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o wtest wtest.c

window: window.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o window window.c

raycast: raycast.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o raycast raycast.c

dots: dots.o sintab.o shapes.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o dots dots.c sintab.c shapes.c

mandel: mandel.o sintab.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o mandel mandel.c sintab.c

lion:
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o lion lion.c
