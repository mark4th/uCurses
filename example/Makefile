
OBJDIR=obj
CFLAGS = -O3 -m64 -g3 -g -march=native --std=c17 -fno-inline -fPIE

SOURCES = main.c menus.c hello.c

OBJECTS = $(addprefix $(OBJDIR)/, main.o menus.o hello.o)

$(shell   mkdir -p obj)

demo : $(OBJECTS) window raycast dots mandel lion widgets
	$(CC) -L /usr/local/lib64 -o demo $(OBJECTS) -lm -luCurses 
#	strip -R .comment demo

$(OBJDIR)/%.o : %.c demo.h $(SOURCES)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -fr obj demo console wtest window raycast dots mandel lion widgets list_demo *.o

console: console.c
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o console console.c

widgets: widgets.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o widgets widgets.c
#	strip -R .comment widgets

window: window.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o window window.c
#	strip -R .comment window

raycast: raycast.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o raycast raycast.c
	strip -R .comment raycast

dots: dots.o shapes.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o dots dots.c shapes.c
	strip -R .comment dots

mandel: mandel.o
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o mandel mandel.c
	strip -R .comment mandel

lion:
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o lion lion.c
	strip -R .comment lion

list_demo: list_demo.c
	$(CC) $(CFLAGS) -L /usr/local/lib64 -lm -luCurses -o list_demo list_demo.c

valgrind: demo
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --trace-children=yes \
	--log-file=v.log ./demo
	