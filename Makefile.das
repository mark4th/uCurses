## -----------------------------------------------------------------------
##  Configuration

CC = $(shell [ "$$CC" == "" ] && echo -n "gcc" || echo $$CC)

OBJS   = $(addprefix $(O)/, \
          main.o parse.o tifile.o color.o list.o \
         util.o )
HDRS   = $(addprefix $(H)/, color.h tui.h uCurses.h list.h)

CFLAGS0 = -O0 -fomit-frame-pointer -pipe -march=native -g3 -c

CFLAGS = $(shell [ "$$CFLAGS" == "" ] && echo -n "${CFLAGS0}" || echo $$CFLAGS )

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

clean:
	rm -f o/*.o
	rm -f u

## =======================================================================

reformat:
	clang-format -i *.c h/*.h
	@for i in *.c h/*.h; do grep -q ' $$' $$i && (echo $$i had trailing whitespace; sed 's/ *$$//' -i $$i) || true; done
