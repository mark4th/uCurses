## -----------------------------------------------------------------------
##  Configuration

CC     = gcc
OBJS   = $(addprefix $(O)/, \
          main.o parse.o tifile.o color.o list.o \
         util.o )
HDRS   = $(addprefix $(H)/, color.h tui.h uCurses.h list.h)
CFLAGS = -O0 -fomit-frame-pointer -pipe -march=native -g3 -c

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
	rm o/*.o
	rm u

## =======================================================================
