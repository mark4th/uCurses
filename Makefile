## -----------------------------------------------------------------------
##  Configuration

CC     = clang

OBJS   = $(addprefix $(O)/, \
   main.o util.o parse.o tifile.o color.o list.o \
   window.o screen.o border.o win_printf.o keys.o utf8.o)

HDRS   = $(addprefix $(H)/, \
  border.h color.h list.h tui.h uCurses.h util.h)

CFLAGS = -Os -fno-inline -fPIE -pedantic -Wall -Werror \
  -std=gnu17 -pipe -march=native -g3 -xc -c

O = o
H = h

## -----------------------------------------------------------------------

o/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

u: $(OBJS)
	$(CC) -o u $(OBJS)
#	strip -R .comment u

$(OBJS): $(HDRS) | $O

$(O):
	mkdir $(O)

.PHONY: clean
clean:
	rm -f o/*.o
	rm -f u
	rm -f *~

## -----------------------------------------------------------------------

analyze:
	make clean
	scan-build make

## =======================================================================
