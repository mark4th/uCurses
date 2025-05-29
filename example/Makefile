
OBJDIR=obj
CFLAGS = -O3 -m64 -g3 -g -march=native --std=c17 -fno-inline -fPIE -I../h

SOURCES = main.c menus.c hello.c

OBJECTS = $(addprefix $(OBJDIR)/, main.o menus.o hello.o)
# window_demo.o dots.o sintab.o shapes.o mandel.o lion.o raycast.o)

$(shell   mkdir -p obj)

demo : $(OBJECTS) window raycast dots mandel lion
	$(CC) -L /usr/local/lib64 -lm -luCurses -o demo $(OBJECTS)
#	strip -R .comment demo

$(OBJDIR)/%.o : %.c demo.h $(SOURCES)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -fr obj demo shell wtest window raycast dots mandel lion *.o

run: demo
	LD_LIBRARY_PATH=/usr/local/lib64 ./demo

shell: chell.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o shell shell.c

wtest: wtest.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -g -o wtest wtest.c

window: window.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o window window.c

raycast: raycast.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o raycast raycast.c

dots: dots.o sintab.o shapes.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o dots dots.c sintab.c shapes.c

mandel: mandel.o sintab.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o mandel mandel.c sintab.c

lion: mandel.o sintab.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -I../h -lm -luCurses -o lion lion.c

