## -----------------------------------------------------------------------
##  Configuration

CC     = clang

OBJS   = $(addprefix $(O)/, \
   main.o util.o tui.o parse.o tifile.o color.o list.o \
   window.o screen.o border.o keys.o windisp.o scrdisp.o)

HDRS   = $(addprefix $(H)/, color.h tui.h uCurses.h list.h)

CFLAGS = -Os -fno-inline -pedantic -Wall -Werror \
  -std=c17 -pipe -march=native -g3 -xc -c

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
	@rm o/*.o
	@rm u

reformat:
	clang-format -i *.c h/*.h
	@for i in *.c h/*.h; do grep -q ' $$' $$i && (echo $$i had trailing whitespace; sed 's/ *$$//' -i $$i) || true; done

## =======================================================================
