# uCurses
```text
██╗   ██╗ ██████╗██╗   ██╗██████╗ ███████╗███████╗███████╗
██║   ██║██╔════╝██║   ██║██╔══██╗██╔════╝██╔════╝██╔════╝
██║   ██║██║     ██║   ██║██████╔╝███████╗█████╗  ███████╗
██║   ██║██║     ██║   ██║██╔══██╗╚════██║██╔══╝  ╚════██║
╚██████╔╝╚██████╗╚██████╔╝██║  ██║███████║███████╗███████║
 ╚═════╝  ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝

              The micro-sized curses library
               for Linux virtual terminals.

            https://github.com/mark4th/uCurses

            Copyright © 2025 Mark I Manning IV

  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  ┃                                                  ┃
  ┃      If something can be read without effort     ┃
  ┃               then great effort                  ┃
  ┃           has gone into its writing.             ┃
  ┃                                                  ┃
  ┃          -- Enrique Jardiel Poncela --           ┃
  ┃                                                  ┃
  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```


## Table of Contents
```text

    1..........Introduction
    1.1............Build process
    1.2............What's documented

    2..........Terminfo
    2.1...........uC_ti_parse.c
    2.2...........Terminfo format string specifiers.
    2.2.1.............If / And / But
    2.2.2.............Format String Specifiers
    2.2.3.............The Beef
    2.2.4.............uC_terminfo.h
    2.2.5.............uC_terminfo.c

    3..........Terminal Attributes
    3.1............uC_attribs.c
    3.1.1.............Modifying attributes variables
    3.2............Terminfo database files.
    3.3............uC_console_attribs.c

    4..........Keyboard input
    4.1............uC_key_read.c
    4.2............key_sequence.c
    4.3............uC_key_table.c
    4.4............uC_mouse.c

    5..........The Screen
    5.1............uC_screen_ctrl.c
    5.2............Drawing it all
    5.3............Updating the console
    5.4............uC_screen_draw.c

    6..........The Window
    6.1............uC_window_ctrl.c
    6.2............uC_window_attr.c
    6.3............uC_border.c
    6.4............uC_window_draw.c

    7..........Status windows
    7.1............uC_status.c

    8..........Menus
    8.1............uC_menu_bar.c
    8.2............uC_menu_pulldown.c
    8.3............uC_menu_item.c
    8.4............uC_menu_key.c

    9..........Widgets
    9.1............uC_widget.c
    9.2............uC_widget_view.c
    9.3............uC_widget_view_group.c
    9.4............uC_widget_draw.c
    9.5............uC_widget_button.c
    9.6............uC_widget_check.c
    9.7............uC_widget_radio.c
    9.8............uC_widget_textbox.c
    9.9............uC_widget_scan.c
    9.10...........uC_widget_keys.c

    10.........Utilities
    10.1...........uC_utils.c
    10.2...........uC_alloc.c
    10.3...........uC_braille.c
    10.4...........uC_entry.c
    10.4.1.............Executing the .so directly - the ELF trick
    10.4.2.............The menu dispatch code generation workflow
    10.5...........uC_eval.c
    10.6...........uC_list.c
    10.7...........uC_switch.c
    10.8...........uC_utf8.c
    10.9...........uC_win_printf.c
    10.10..........uC_winch.c

    11.........JSON
    11.1...........json.c
    11.2...........json_token.c
    11.3...........json_schema.c
    11.3.1.............Schema entries
    11.3.2.............Schema helpers
    11.3.3.............Key dispatch
    11.4...........json_value.c
    11.5...........json_populate.c
    11.6...........json_build.c

    12.........Function coverage appendix

    13.........Final words


```


## 1. Introduction
First of all, this library is not a replacement for ncurses and it was
never intended to be.  Its primary purpose is to give developers of
embedded Linux systems with limited resources the ability to create simple
yet powerful terminal based user interfaces.

It would actually be next to impossible for me to create anything that
could even come close to competing with ncurses because Thomas E. Dickey
who is the current (long time) maintainer of ncurses has extensive
knowledge of the domain that I currently lack.  He also produces a library
that has two things going for it that uCurses does not and may never have!

1. Consistent Reliability
2. Reliable Consistency

There is a very *very* good reason why ncurses is ubiquitous in the *nix
universe - How could an ID10T like me compete with that? <3

Note: While I do not actually program ncurses and in fact know
absolutely nothing about developing with it, this library would not be
possible at all without ncurses or its developer.

This library does however have a few things that ncurses does not.

It is extremely small (but growing :/ ).

At the time of writing, ncurses is a little over 4.3 megabytes in
size and I may have missed counting a few files.  It also takes a
little over seven minutes to compile on my laptop.

uCurses on the other hand currently takes up about 77k and
compiles in about two or three seconds ish depending on planetary
alignment.  This means that recompiling this on any embedded
system it is installed on would probably not bog that system down
during any such build.   No need for this to be target compiled!

P.S.  This is not a criticism of ncurses, with all that 'beef' it
brings a lot of utility to the table and it fully supports pretty
much any hardware you throw at it, including ancient hardware.

uCurses is a newer library than ncurses and has not yet received
the same level of real-world testing across terminal emulators.
It is known to work well in gnome-terminal, ptyxis,  Konsole and
xterm.  As with any new library, YMMV.

Moveable, overlapping windows.

This I believe would be almost impossible using ncurses yet is
ultra trivial here.  I do however have confidence that anyone
experienced with ncurses development could probably achieve the
same thing though probably not with ease.

A custom JSON Parser for user interface descriptions.

This custom parser literally doubled the size of my library when
it was implemented but it allows the application developer to
describe almost their entire user interface in JSON.

The exception here is widgets which are considered ephemeral so
can only be created and destroyed programmatically on an as needed
basis.

Menus, which might normally be considered a class of "widget" and
which might normally be handled by the widget handler elsewhere
are handled independently of them here because menus are not
considered ephemeral.

Menus and widgets are also independent build options which can be
stripped out of the build to save space for functionality not
needed in an application.

The developer of course still has the ability to create and
destroy menus programmatically outside the scope of the JSON
parser.

The JSON data can be either a separate plain text file loaded at
application launch, allowing application end users to possibly
theme the application, or it can be built into your executable
and possibly be compressed.

The menu system, widgets and JSON parser are all independent build
options and excluding one or more of them can significantly shrink
the overall size of the build for your target.


### 1.1. Build process
The top level `Makefile` is a convenience wrapper around Meson and Ninja.
Running `make` creates the `build/` directory when needed, configures the
project with Meson, then runs Ninja to build both the shared and static
libraries.

```sh
make
```

The wrapper prefers Clang and LLD when they are available.  GCC is also
supported by the Meson build, and the build system selects size-oriented
optimization flags for the compiler in use: `-Oz` for Clang and `-Os` for
GCC.

The common developer targets are:

```sh
make              # configure and build the library
make examples     # build and copy runnable demos into example/
make tests        # build and run the Meson test suite
make clean        # remove the build tree and copied example binaries
make rebuild      # clean, configure, and build from scratch
make install      # install the library and public headers
```

The `example/` directory also has a small forwarding makefile, so
`make -C example` rebuilds the examples from the project root.

Optional features are controlled by Meson options in `meson_options.txt`.
Menus, JSON, status windows, widgets and popups default to enabled.  Mouse
support defaults to disabled.

```sh
meson configure build -Dmouse=true
meson configure build -Djson=false -Dwidgets=false
make
```

If source files are added to or removed from one of the source directories,
Meson needs to be reconfigured because the file lists are collected during
configuration.  The simplest option is `make rebuild`; the lighter option
is `meson setup build --reconfigure` followed by `make`.

NOTE: This library is built with a `-fvisibility=hidden` arg passed to the C
compiler.  This will cause all symbols that are not marked as public to be
stripped out of the executable at link time.  All public API entities are
prefixed with an API macro which is defined in a header file.  If any
entity lacks this prefix it is not visible to any application using this
library.

All items that are marked as static are only visible within the module
wherein they are defined and any item that is neither prefixed with the
API macro nor marked as static will have global visibility only within the
library itself.


### 1.2. What's documented (or will be)
The documentation for uCurses will detail every aspect of this library.
It will show you not only how to use it but how it works and why it does
the things it does the way it does them.  This specific document is
intended to cover every function within every C source file, including
private helpers.  Variables, structures and individual fields are
documented where they explain the design, but they are not the coverage
target of this document.

How the public pieces are intended to be used within an application is
covered in separate, smaller documents.

One of the biggest criticisms I have for the documentation of other API's
is that while they provide extensive descriptions for every PUBLIC
function, what parameters they expect and what values they return etc they
almost never explain to you how you are supposed to use all of those
234755864237 public API calls in an application.   They also never that I
have seen document any non public entity within their sources.

WARNING: I might also rant just a little bit here and there about the way
developers of Linux virtual terminals do not honor the contract they sign
in their terminfo databases.  The more I learn the more I come to
understand that this might not actually be *entirely* their fault but...
I still rant because...

> I consider terminfo to be a BINDING contract so when (for example) your terminal randomly decides to return either `0x08` or `0x7f` when someone presses the backspace key (based on who compiled it?), when your terminfo explicitly states that it will return `0x08`, then I consider you to be in flagrant, wilful violation of your contract.

Things like this force me to add special needs 'duct tape' code to fix
things that in my mind should not need fixing.  This and other "quirks"
probably accounts for at least part of why ncurses is as large as it is.

Note to TED:  If there is *anything* stated in the uCurses documentation
that is even slightly incorrect I will gladly take instruction and change
these texts.  The same applies to any of my code.  I will even take
destructive criticism and flames from "certain quarters" and try to make
corrections :)


## 2. Terminfo
This library makes extensive use of terminfo.  Every single color change
or cursor movement is performed using the specific escape sequences for
the specific virtual terminal being used.  Terminfo supplies templates for
each(ish) escape sequence supported by a given terminal in the form of
format strings.

Different terminals often use different escape sequences to perform the
same operations.  The format string for a given terminal allow this
library to compile the correct escape sequence for the required operation
on almost any terminal.

These 'format strings' are written in a very simple interpreted language.
They are passed to an interpreter along with whatever parameters are
required which will parse them and compile the appropriate escape
sequences.

A format string is a small text string that contains either characters to
be written out directly to the compiled escape sequence buffer or
operators which perform various operations to aid in the compilation of
other things to be written to said buffer.

In uCurses, all of this is done by the functions in...


### 2.1. `uC_ti_parse.c`
I believe it is somewhat important for developers to have at least a
partial understanding of what goes on in here.  This file is the heart of
the entire system and is literally the first code developed for uCurses.
The entire rest of the system was built up from right here.

All variables used by this parser are stored in an allocated structure
called `ti_vars` which is of type `ti_vars_t`.  This structure is defined
in the `uC_terminfo.h` file.

This structure contains the following entities.  The purpose of each of
these is described in more detail below.

- A pointer to the current position within the current terminfo
  format string.
- The format string stack.
- Arrays for the static and dynamic terminfo variables.
- An array of terminfo format string parameters.
- The address of a buffer to store compiled escape sequences.
- Variables associated with the terminals memory mapped terminfo
  database file (containing the format strings etc).
- The keyboard input buffer and other related variables.

As each escape sequence is compiled it is appended onto the end of the
above mentioned escape sequence buffer but nothing is physically written
out to the console until either the escape sequences for the entire update
have been compiled or the escape buffer becomes full.

While some effort has been made to allow for flushing of the buffer when
it is filled, this will not work correctly if a flush occurs mid-way
through an escape sequence.  In practice the 64k buffer is large enough
that this situation does not arise, so it is documented here as a known
theoretical limitation rather than an active bug.

The parsing of terminfo format strings is a very simple affair.  You have
a format string, you have parameters, you have variables and you have a
stack.  Parameters are passed to the format string interpreter and are
stored in a parameters array.  Variables are usually calculated based on
values contained within the format strings themselves.  By convention,
variables are read / write, parameters are read only while parsing is in
progress.

The stack is used as temporary storage by the parser to hold values to be
acted on by the various operators during said parsing (see below).

The following functions support the format parser and the various
primitives it references based on specifiers contained in the format
strings which are explained below.

```c
void alloc_parse(void)
```

This function allocates the structure containing the variables
used during the parsing of terminfo format strings.  The allocator
used here is also part of uCurses (and public) and is detailed
elsewhere in this document.

This function also allocates the escape sequence buffer which at
the time of writing has a hard coded size of 64k.  This is
probably overkill.  A debug build option will be added to track
the high-water mark of the buffer so that application developers
targeting memory-constrained systems can tune it to its minimum
required size plus some headroom.

```c
API void uC_terminfo_flush(void)
```

This function is where all the compiled escape sequences are
written out to the virtual terminals stdout.  If the write fails
a single immediate retry is attempted.  This covers transient
`EAGAIN` / `EINTR` conditions.  Persistent failure is silently
ignored; the terminal is considered unrecoverable at that point.

```c
void terminfo_purge(void)
```

Internal helper that discards all currently compiled escape
sequences without writing them to the terminal.  It is used when
a resize invalidates the partially compiled output buffer.  flush
writes; purge discards.

```c
void c_emit(char c1)
```

This function simply writes one character to the escape buffer.
If however the buffer is full it will first flush the current
contents of the buffer out to the terminal so we can start
compiling another 64k of escape sequences (yay!).

As stated above, this is not quite right in its implementation
because if the new character is part of an already partially
compiled escape sequence then we probably can't flush half of that
incomplete sequence now and half later.

This is a known theoretical limitation.  Given the 64k buffer
size and typical usage patterns this condition has never been
observed in practice.

```c
static void fs_push(int64_t n)
```

This pushes the passed integer onto the terminfo format stack.  I
have set an arbitrary maximum depth of five items here using a
special magic number '5'.  If any attempt is made to push a sixth
item the program will complain and die horribly (tm).

Maybe this should not be a magic number and maybe, somewhere out
there in the wild there is a format string that needs to use more
stack.  Who knows?

```c
static int64_t fs_pop(void)
```

Pops one item off the format stack and returns it to the caller.
If any attempt is made to pop an item off this stack when it is
empty then the program will complain and die horribly (tm).

The above "die horribly (tm)" is because both the stack overflow and
underflow are considered to be critical errors in this parser.  In the few
years I have been developing this library I have never actually seen this
occur and as we all know, the absence of proof is the proof of absence!

```c
static void fs_reset(void)
```

Clears the scratch stack used while compiling a single terminfo format
string.  This is called at the start of `uC_parse_format()` so no
leftover stack state from one format string can affect the next.

```c
static void fs_drop_leftovers(void)
```

Discards any values a terminfo format string left on the scratch stack.
A leftover value means the format pushed something it did not consume,
but this is not fatal because the generated escape sequence is already
complete and the scratch value must not leak into the next parse.

```c
static int64_t *get_var_addr(void)
```

This helper function will read one character out of the current
format string which should be in the range `a` to `z` or `A` to
`Z`.

These are interpreted as single character names for either the
static or dynamic variables which in turn are used as indices into
their respective variable arrays.  The address is returned as an
int64_t pointer to the caller.

The terminfo man page states that in practice there has never been
any usage difference between the two variable types.


### 2.2. Terminfo format string specifiers.
The specifiers within the terminfo format strings are very similar to the
ones used in C's printf functions.  I.E.  They all start with a percent
symbol.

Each specifier is described below.  They all perform very simple
operations but there are quite a few of them to go over.  None of these
make use of C's evil variadic functions.  I do that elsewhere tho } : )

NOTE: Being able to mentally interpret a terminfo format string might
not be vital to the application developers understanding of or the use
of this library but I still consider it important enough to document.
This 'skill' does have its uses.


#### 2.2.1. Conditionals
The terminfo format string language supports conditionals of the form:

```text
%? ccc %t ttt %e fff %;
```

where ccc is the condition expression, ttt is the true branch and fff is
the optional false branch.  Read it as: if ccc then ttt else fff end.

The condition expression ccc is evaluated by whatever format specifiers
appear between %? and %t.  Those specifiers leave a value on the stack
exactly as they would anywhere else in the format string.  %? itself
is a structural delimiter with no stack or output effect.

%t pops the condition value.  If true, interpretation continues with ttt;
when the parser subsequently encounters %e it scans forward to %; and
stops, skipping fff entirely.  If false, %t scans forward past ttt to the
%e or %; and resumes from there, executing fff if present.

%; is also a structural delimiter with no stack or output effect.  By the
time the parser reaches it the correct branch has already been executed or
skipped, so nothing remains to be done.

Conditionals may be nested.  This is almost exactly the way a Forth
system handles if / else / then.

The following helper function is used when parsing conditionals.

```c
static char scan(void)
```

This static helper function will repeatedly parse the current
format string up to the next format specifier within it.  Once the
next format specifier (percent character) within the string has
been found the specific letter of that specifier is returned to
the caller.


#### 2.2.2. Format String Specifiers
The following list shows the form of each specifier as it would appear in
a format string and the name of the function that performs that
specifier's associated operation.

```text
%%  static void _percent(void)
```

Simply emits a '%' character to the escape buffer.

```text
%&  static void _and(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and performs a bitwise AND
operation on them.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's & operator.

```text
%A  static void _andl(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and performs a logical AND
operation on them.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's && operator.

```text
%|  static void _or(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and performs a bitwise OR
operation on them.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's | operator.

```text
%O  static void _orl(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and performs a logical OR
operation on them.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's || operator.

```text
%~  static void _not(void)
```

Pops one item off the terminfo format stack which must have been
placed there by a previous operation and performs a bitwise NOT
(ones complement) operation on it.  The result is then pushed onto
the stack to be returned to the interpreter.

This is similar in nature to C's ~ operator.

```text
%!  static void _notl(void)
```

Pops one item off the terminfo format stack which must have been
placed there by a previous operation and performs a logical NOT
operation on it.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's ! operator.

```text
%^  static void _xor(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and performs a bitwise XOR
operation on them.  The result is then pushed onto the stack to be
returned to the interpreter.

This is similar in nature to C's ^ operator.

```text
%+  static void _plus(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and adds the two values
together.  The result is then pushed onto the stack to be returned
to the interpreter.

```text
%-  static void _minus(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and subtracts the value of the
second item pushed from the value of the first item pushed.  The
result is then pushed onto the stack to be returned to the
interpreter.

```text
%*  static void _star(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and multiplies them together.
The result is then pushed onto the stack to be returned to the
interpreter.

```text
%/  static void _slash(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and divides the value of the
first item pushed by the value of the second item pushed.  The
quotient of the result is then pushed onto the stack to be
returned to the interpreter.

There is no divide by zero exception here, in the case where the
divisor is zero the result is zero.

```text
%m  static void _mod(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and divides the value of the
first item pushed by the value of the second item pushed.  The
remainder of the result is then pushed onto the stack to be
returned to the interpreter.

There is no divide by zero exception here, in the case where the
divisor is zero the result is zero.

```text
%=  static void _equals(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and tests them for equality.
A result of 0 (false) or 1 (true) is then pushed onto the stack to
be returned to the interpreter.

```text
%>  static void _greater(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and tests that the item pushed
first is greater than the item pushed second.  A result of 0
(false) or 1 (true) is then pushed onto the stack to be returned
to the interpreter.

```text
%<  static void _less(void)
```

Pops two items off the terminfo format stack which must have been
placed there by previous operations and tests that the item
pushed first is less than the item pushed second.  A result of 0
(false) or 1 (true) is then pushed onto the stack to be returned
to the interpreter.

```text
%'  static void _tick(void)
```

Reads the next character out of the format string and pushes that
character onto the terminfo format stack.  The next character is
then also parsed and discarded.  It is assumed to be a closing '
character.

The code does not verify that the format string is correct here as
it is assumed that the terminfo compiler checks for such errors.

```text
%i  static void _i(void)
```

Increments the values of the first two items in the terminfo
parameter array by one.  This is done because ANSI terminals
expect parameters to be 'one' based where as other terminals
expect them to be 'zero' based.  Having this operator allows for
all parameters to be passed in the same format.

```text
%s  static void _s(void)
```

Pops the address of an application based ASCIIZ string (from your
applications address space) off the terminfo format stack which
must have been placed there by a previous operation and writes
each character of that string out to the escape buffer.

```text
%l  static void _l(void)
```

Pops the address of an application based ASCIIZ string (from your
applications address space) off the terminfo format stack which
must have been placed there by a previous operation and pushes a
value equal to the length of whatever string is at this address
onto the stack.

In uCurses it is assumed that this string might be a UTF-8 string
and the length returned is equal to the total number of bytes used
by the string.  Characters that are UTF-8 encoded can be one, two,
three or four bytes in length.

```text
%P  static void _P(void)
```

Gets the address of the static or dynamic variable whose single
character name is specified in the next character of the current
format string and then pops the top item of the format stack into
that address.

This sets the specified variable to the value of the item on the
stack.  Variables are retained across all format strings to be
interpreted.

Question for TED: Should the variables ever be zero'd and if so
when?

```text
%g  static void _g(void)
```

Gets the address of the static or dynamic variable whose single
character name is specified in the next character of the current
format string and pushes the value of that variable onto the
format stack to be returned to the interpreter.

```text
%{  static void _brace(void)
```

Parses the following characters of the current format string up to
the closing } and interprets those characters as a decimal number.
The value of this decimal number is pushed onto the stack to be
returned to the interpreter.

```text
%?  void uC_noop(void)
```

Marks the start of a conditional.  No stack or output effect;
see section 2.2.1 for the full conditional semantics.

```text
%t  static void _t(void)
```

Pops the condition value left on the stack by the ccc expression.
If true, returns immediately and the then-branch executes
normally.  If false, scans forward past the then-branch to the
next %e or %; and resumes from there.

```text
%e  static void _e(void)
```

Reached at the end of a true then-branch.  Scans forward to the
closing %; and resumes from there, skipping the false branch.

```text
%;  void uC_noop(void)
```

Marks the end of a conditional.  No stack or output effect;
the correct branch has already been executed or skipped by the
time the parser arrives here.

```text
%d  static void _d(void)
```

This format specifier will pop one item off the terminfo format
stack which must have been placed there by a previous operation
and will then write it out to the escape buffer as an ASCII string
representation of its value in decimal.

The value written out can be written in as many digits as it takes
to represent it or as a hard fast mandated two or three character
string with leading zeros if need be.  I am assuming that there
are some crazy terminals out there that require certain values to
be represented in a very specific way and this is how that is
accomplished.

The width here is based on the `2` or `3` prefix character (if
any) that was parsed by the `next_c()` function documented below.

```text
%c  static void _c(void)
```

Pops one item off the terminfo format stack which must have been
placed there by a previous operation and emits that as a single
character to the escape buffer.

```text
%p  static void _p(void)
```

Pops one item off the terminfo format stack which must have been
placed there by a previous operation and uses that as an index
into the terminfo parameters array.  The index in the format
string is 1-based but is decremented to 0-based before use.

The indexed parameter is then pushed onto the stack to be returned
to the interpreter.


#### 2.2.3. The Beef
The following functions are the core of the format string parser.  They
interpret the various format specifiers and act on them appropriately.

```c
static char next_c(void)
```

This helper function is called to parse the next character after
having parsed a format specifiers % character.

If the next character is either a `2` or a `3` then the next
character after that is assumed to be a `d`.  The digit value here
is interpreted as a width prefix for emitting a decimal value via
the above documented %d specifier.  It allows for %d, %2d and %3d
specifiers which might output `1` or `01` or `001` for example.

In this instance we save the width specifier for a 'one time use'
and then parse the next character from the format string which
would be the `d` as stated above.

If the next character was neither a `2` or a `3` then it is the
name of some format specifier and we simply return that as is.

```c
static const uC_switch_t p_codes[]
```

This is an array of `uC_switch_t` entries which contain an option
character and a function vector.  This is used to direct the
parser to the correct handler for the most recently parsed format
specifier.

I personally consider C's switch statements to be horrendously
FUGLY and I see this as a much neater presentation.  It may be a
fact that this code won't optimize as efficiently as an actual
switch statement but...  err...  this is a text based user
interface and that is going to be so horrendously slow (in
computer terms not human) that this 'inefficiency' is literally in
the noise.

P.S. I hate code that looks like an unmade bed.

```c
static void specifier(char c1)
```

This helper function has one purpose.  Given a format string
specifier name (character) and using the above array, branch to
the specific handler for that format specifier.

```c
API void uC_parse_format(const char *f)
```

This public function parses a complete format string at a
specified address and compiles the specific escape sequence
associated with that format string into the escape buffer.  This
format string may or may not also require parameters and if they
are required they are assumed to already have been written to the
format string parameters array.

A format string is always an ASCIIZ string so a '\0' character
marks the end of the format string.

```c
void uC_format(int16_t i)
```

This function will locate a format string within the current
virtual terminals memory mapped terminfo database based off of the
index parameter passed to it.

This is an index into the terminfo files strings section which is
in turn an offset into the terminfo files string table section
(where all the format strings live).

Each operation to be performed within a terminal that has a format
string (such as positioning the cursor, turning on bold, changing
a color attribute etc) will have a pre-defined entry within the
terminfo files string section at a pre-defined index.


#### 2.2.4. `uC_terminfo.h`
Within this header file there are numerous macros which reference the
above `uC_format()` function.  Each of these macros passes a hard coded,
well known terminfo string section offset to it.

These macros are used within this library to inline one or other of the
numerous escape sequences.  Some of these have public wrappers allowing
application developers to leverage them, most however do not.

Each of these macros has a name identical to the terminfo name for their
escape sequence with a `ti_` prefix added.  Most are considered not part
of the public API and are only used internally (if at all).


#### 2.2.5. `uC_terminfo.c`
This file contains wrappers to the above terminfo macros giving the
application developer access to them.   Most of these have names
identical the terminfo format string name but with a uC_ prefix on that
name.  Others have names that describe their function better.

```c
API void uC_curoff(void)
```

This function makes the terminals cursor invisible so it does not
detract from the applications user interface.   This function is
referenced by the uCurses initialization functions.

```c
API void uC_curon(void)
```

This function makes the terminals cursor visible which would be
necessary on exit from any uCurses application.  This function is
referenced by the uCurses de-initialization functions.

```c
API void uC_clear(void)
```

This function clears the console display and sets the cursor to
the upper left of the display.  If there is currently an active
screen then the state buffers of this structure are also erased
and its cursor is also reset to top left.

```c
API void uC_hpa(int16_t x)
```

HPA stands for "Horizontal Position Absolute".  This function
moves the cursor to a specific X coordinate on which ever line the
consoles cursor is situated.  This also moves the cursor within
the current active screen to the same location.

```c
API void uC_cup(int16_t x, int16_t y)
```

This function sets the terminals cursor position.   It moves the
cursor to a specified X / Y coordinate within the terminal.  This
also moves the cursor within the current active screen to the same
location.

```c
API void uC_cud1(void)
```

This function moves the terminals cursor down one line.  If the
cursor is on the bottom line of the terminal this will affect a
scroll on the display.   It also moves the cursor of the currently
active screen down one line UNLESS it is on the bottom line.
There is also no concept of scrolling the active screens contents.

```c
API void uC_home(void)
```

This function moves the cursor to its home position of 0 / 0
within the console.  It will also move the currently active
screens tracked cursor to the same location.

```c
API void uC_cub1(void)
```

This function moves the consoles cursor left one position.  If the
cursor started at the left edge of the console then the cursor
will be moved to the end of the line above.

This also moves the currently active screens cursor left one
position and will also wrap to the line above if need be.

```c
API void uC_cuf1(void)
```

This function moves the consoles cursor right one position.  If
the cursor was at the far right edge of the console then the
cursor will be moved to the start of the line below.
If the cursor was on the bottom line then the display will also be
scrolled up.

The active screens cursor location is also moved in a similar way
but it will not be scrolled.

```c
API void uC_cuu1(void)
```

This function will move the cursor up one line within the console
unless it is already on the top line.  It will also move the
currently active screens cursor up one line unless it was also on
the top line of the screen.

```c
API void uC_dch1(void)
```

This function will delete the character to the left of the
consoles cursor location.  Doing this will move other characters
to the right of that one left one position.

It also moves the cursor of the currently active screen left one
position.  It does not actually delete or move any characters on
the line.

```c
API void uC_cud(int16_t n1)
```

This function moves the consoles cursor down a specified number of
lines.  If, during this operation the cursor is on the bottom line
of the display then each cursor down operation here will instead
scroll the console up one line.

It will also move the currently active screens cursor down the
same number of lines up to the bottom line of the screen but it
will not scroll the screen.

```c
API void uC_ich(void)
```

This function will insert a space into the consoles current screen
line at its current cursor location.  All characters to the right
of the cursor will be moved right.  The cursor itself does not
move.

This call does not affect the screen in any way.

```c
API void uC_cub(int16_t n1)
```

This function moves the consoles cursor back the specified number
of positions.  It will wrap the cursor to the end of the line
above if it is at the far left edge of the console.

It will also perform the same operation on the currently active
screen.

```c
API void uC_cuf(int16_t n1)
```

This function moves the consoles cursor forward the specified
number of positions.  It will wrap the cursor to the start of the
line below if it is moved beyond the far right edge of the console
and potentially scroll the display.

This will also perform the same operation on the currently active
screen but will not scroll the screen.

```c
API void uC_cuu(int16_t n1)
```

This function will move the consoles cursor up the specified
number of lines or until the cursor reaches the top line.

It will also perform the same operation on the currently active
screen.

```c
API void uC_vpa(uint16_t y)
```

VPA stands for "Vertical Position Absolute".  This function moves
the consoles cursor to the specified Y coordinate.  It will also
perform the same operation on the currently active screen.

```c
API void uC_cr(void)
```

This function writes both the CR and the LF characters out to the
console.  It also moves the currently active screens cursor to the
start of the line below it unless it is already on the bottom
line.

```c
API void uC_smkx(void)
```

This function is documented in man 5 terminfo as placing the
keypad in transmit mode.  I have no idea what that means but I
have seen that the cursor UP and DOWN keys can return different
sequences depending on whether or not the keypad is in this
mysterious mode or not.

Placing the keypad in transmit mode allows the uCurses keyboard
handlers to recognize the cursor up and down keys using the
format strings that are contained within the terminals terminfo
database.   Not being in transmit mode prevents it from being able
to do so.

`uC_smkx()` is now called automatically by `uCurses_init()`.
Application code should not need to call it directly.

```c
API void uC_rmkx(void)
```

This function turns off the keypad transmit mode whatever that
is.  See smkx above.

`uC_rmkx()` is now called automatically by `uC_restore_terminal()`
which is called by `uCurses_deInit()`.  Application code should
not need to call it directly.


## 3. Terminal Attributes
Once we have the ability to compile format strings into escape sequences
we can emit said escape sequences to modify character properties such as
foreground color, background color, bold, underline and reverse video etc.

uCurses attributes are stored in `uC_attribs_t` structures.  These hold the
various attributes which are used when writing characters into user
interface entities such as windows or the screen.

Supported attributes include foreground and background colors using a
normal palette-indexed value, a full 24 bit RGB value or a gray scale
color.  The attributes structure contains flags which indicate which of
these are being used. The following list describes each supported flag...

- ul         Characters are underlined
- rev        Characters are drawn in reverse video
- bold       Characters are drawn in bold
- rgb_fg     Characters have a 24 bit RGB foreground
- rgb_bg     Characters have a 24 bit RGB background
- gray_fg    Characters have a gray scale foreground
- gray_bg    Characters have a gray scale background

Note: uCurses does not support standout mode or that annoying flashy
blinky text thing (ick).

While many terminals do support both 24 bit RGB and gray scale coloring,
no terminal that I know of has any format strings in their terminfo
databases to aid in the compilation of their specific escape sequences for
those color formats because they are not an officially recognized part of
the terminfo specification (yet?).

Note: Actually, I think there are 7465728345 different specs and
ncurses has to support all of them or people might get uppity.  I
won't be supporting all of them because I get uppity too.

This means that applications compiled to use uCurses which make use of
these color formats will probably not act in a reliable manner across all
terminal types.

I.E.  YMMV (tm)

With a greater understanding of the problem I might, in the future, be
able to fix this to a degree but I will rabidly reject the notion that I
must add 56435 different format strings for each of the intended terminal
types and add special needs duct tape code to arbitrate which of these is
the correct one to use.

NOT happening!

If these features are one day added to the spec(s) they will probably also
be given a terminfo capabilities entry and applications could then decide
at run time which features were or were not supported and take appropriate
action.

Note: uCurses deliberately ignores terminal capabilities.  The
library targets modern xterm-compatible terminals and relies on
that baseline rather than probing at runtime.

As each character is written out to some user interface entity it gets its
own instance of the attributes structure along with that characters
encoded UTF-8 code point.  Every character in said entity therefore has
its own independent set of attributes.

The foreground and background colors for each character are entirely
independent of one another so each can be drawn using any of the above
stated color representations.

Note: Not all escape sequences which are supported by a given virtual
terminal will have an associated format string entry in their terminfo
database and not all terminals support all escape sequences.  If a
terminal does not support a given escape sequence then its string
section entry for that item will be a -1 value (`0xffff`).

For example, the Linux console does not have an "enacs" format string
because the "alt char set" does not first need to be enabled to be
used.

Note: alt charset support was removed when UTF-8 box-drawing was
added, which effectively dropped support for the Linux console TTY
(which has poor UTF-8 support).  Re-adding it as an optional build
target is under consideration for a future branch.


### 3.1. `uC_attribs.c`
Emitting the escape sequences for enabling both foreground and background
RGB attributes requires a hard coded format string which may not be
applicable across all terminals.  Ditto gray scales.  These sequences are
'hard coded' into the following functions.

```c
static void rgb_fg(void)
```

This function takes the Red, Green and Blue foreground entries out
of the current attributes variable and stores them in the first
three parameters of the terminfo format string parser.  It then
has that parser interpret the hard coded format string contained
herein.

This will compile an escape sequence which, when written out to
the terminal will set the console foreground color to the
specified RGB values.

This is known to work correctly in most modern terminals.

```c
static void rgb_bg(void)
```

This function takes the Red, Green and Blue background entries out
of the current attributes variable and stores them in the first
three parameters of the terminfo format string parser.  It then
has that parser interpret the hard coded format string contained
herein.

This will compile an escape sequence which, when written out to
the terminal will set the console background color to the
specified RGB values.

See above! (tm)

```c
static void gray_fg(void)
```

This function first adds 232 to the first parameter of the
terminfo parser which will have been previously set to a value
between 0 and 23 (the specific gray scale value requested) and
then has the format string parser interpret the hard coded format
string contained herein.

This will compile an escape sequence to set the console foreground
to the specified gray scale value.

```c
static void gray_bg(void)
```

This function first adds 232 to the first parameter of the
terminfo parser which will have been previously set to a value
between 0 and 23 (the specific gray scale value requested) and
then has the format string parser interpret the hard coded format
string contained herein.

This will compile an escape sequence to set the console background
to the specified gray scale value.

```c
static void apply_fg(void)
```

When any change has been made to the foreground color within the
current attributes variable this helper function is called to
arbitrate which escape sequences needs to be emitted in order to
affect that change.

This will either be one of the above `rgb_fg()` or `gray_fg()` calls
or will simply be a call to one or other of the official terminfo
functions to set a foreground color, `setf()` or `setaf()`.

If the terminal supports ANSI colors then the terminfo strings
section will have an entry for the setaf format string.  If so
then this is the escape sequence emitted here.

If the terminal does not support ANSI colors the strings section
entry for that format string will be -1.  In this case we simply
assume that setf is supported (tm).

```c
static void apply_bg(void)
```

When any change has been made to the background color within the
current attributes variable this helper function is called to
arbitrate which escape sequence needs to be emitted in order to
affect that change.

This will either be one of the above `rgb_bg()` or `gray_bg()` calls
or will simply be a call to one or other of the official terminfo
functions to set a background color, `setb()` or `setab()`.

If the terminal supports ANSI colors then the terminfo strings
section will have an entry for the setab format string.  If so
then this is the escape sequence emitted here.

If the terminal does not support ANSI colors the strings section
entry for that format string will be -1.  In this case we simply
assume that setb is supported.

```c
if_apply_fg(uint8_t changes)
```

This helper function is used to determine if any change has been
made to the current attributes variable that requires an update to
the foreground color.  Non color changes such as turning bold off
(for reasons explained below) will mandate that even if the
current foreground color has not been changed we must still update
it (an annoyance).

The `changes` parameter here contains flags that indicate whether
any non color attribute have been modified.  If any such
modification exists which mandates an update to the foreground
color or if the foreground color has actually been modified then
this function will call the above `apply_fg()` function.

```c
if_apply_bg(uint8_t changes)
```

This helper function is used to determine if any change has been
made to the current attributes variable that requires an update to
the background color.  Non color changes such as turning bold off
(for reasons explained below) will mandate that even if the
current background color has not been changed we must still update
it. (STILL an annoyance).

The `changes` parameter here contains flags that indicate whether
any non color attribute have been modified.  If any such
modification exists which mandates an update to the background
color or if the background color has actually been changed then
this function will call the above `apply_bg()` function.

```c
apply_attribs(void)
```

When modifications are made to the current attribs variable this
code will determine what, if any escape sequences need to be
emitted in order to affect those changes.  It first checks if any
changes have been made to either the bold or reverse-video
settings.

Any changes to either of these will potentially necessitate a
complete wipe of all colors and all attributes by emitting an sgr0
(set graphics rendering) escape sequence followed by emitting the
escape sequences of those attributes or colors we did not actually
want to change!

Turning off bold always requires a reinstatement because there is
no escape sequence in terminfo to turn bold off, only on.  The
only way to reliably turn bold off is to either hard code 278456
different escape sequences for the 278456 (or so) different
terminal types (assuming any of them actually have a bold off
escape sequence) or.....  to simply hard reset then reinstate.

Ditto reverse video.

If any change to either reverse video or bold have been made and
either of those changes is to turn that attribute off then this
code will rip the rug out from under itself, reset all colors and
attributes and then....  reinstate any attributes that were not
meant to be changed.

The code then determines if any changes were made to the underline
attribute and if so then that attribute is updated.  If the above
sgr0 was referenced there could be a tiny optimization made here
in that we don't actually need to emit the sequence to turn
underline off because the sgr0 will have already done that for us.

Note: uCurses does not support flashy blinky text or standout
mode.

Interesting Note:  Translation: Rant Incoming! (tm):

The single slowest thing you can do within terminal is to
change character attributes.  For this reason, when updating
the screen, uCurses will literally set ONE attribute then
bounce the cursor all over the screen so it can update every
single character that shares that ONE attribute.  It then
moves to the next character which needs updating and sets that
characters attributes etc etc loop till done.

Every attribute change is written exactly ONE TIME, then we
ping-pong the cursor all over the map because that is faster.

Being forced to hard reset all attributes to affect one itsy
bitsy change and then to reinstate all those other attributes
that we actually wanted to keep is probably the single worst
design decision the implementers of terminfo could have made.
They SHOULD have implemented both ON and OFF escape sequences
for *EVERY SINGLE* attribute.

P.S. As we know, Some things make me uppity!

Note to TED:  I have recently learned that if a color
format change is made in Xterm such as changing from rgb to
gray scale coloring then an sgr0 is required.  Any chance
this can be fixed?


#### 3.1.1. Modifying attributes variables
The following functions aid in the setting and updating of color
attributes.  They modify a specified attributes variable which can be
any instance of a `uC_attribs_t` structure such as is found in...

- The global default attributes variable
- The various attributes associated with a window
- The various attributes associated with menus
- A window or screen character cell
- ... etc ...

```c
static uC_ti_attr_flags_t attr_add_flags(uC_ti_attr_flags_t flags,
uint16_t bits)
```

This helper function is used to modify the attribute flags used to
indicate which color format is being used.  It ensures that no
mutually exclusive setting can be selected.

E.G.  If the foreground color is being changed from gray scale to
RGB then this function will clear the flag indicating that a gray
scale is being used.

```c
API void uC_attr_set_flags(uC_attribs_t *attribs, uint16_t bits)
```

This public API function will turn on any given attribute within a
passed in `uC_attribs_t` parameter.  Each attribute that is to be
turned on is specified within the bits parameter to this function.

Mutually exclusive settings will result in one or other of those
settings being cleared.

```c
API void uC_attr_clr_flags(uC_attribs_t *attribs, uint16_t bits)
```

This public API function will turn off any given attribute within
a passed in `uC_attribs_t` parameter.  Each attribute that is to be
turned off is specified within the bits parameter to this
function.

```c
void set_fg(uC_attribs_t *attr, uC_color_t color)
```

This helper function is used to set a normal palette-indexed
foreground color within the specified attributes structure.

```c
void set_bg(uC_attribs_t *attr, uC_color_t color)
```

This helper function is used to set a normal palette-indexed
background color within the specified attributes structure.

```c
void set_gray_fg(uC_attribs_t *attr, uC_colors_gray_t color)
```

This helper function is used to set a foreground color within an
attributes structure to a gray scale value.

```c
void set_gray_bg(uC_attribs_t *attr, uC_colors_gray_t color)
```

This helper function is used to set a background color within an
attributes structure to a gray scale value.

```c
void set_rgb_fg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
uC_color_t b)
```

This helper function is used to set a foreground color within an
attributes structure to a 32 bit RGB value.

```c
void set_rgb_bg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
uC_color_t b)
```

This helper function is used to set a background color within an
attributes structure to a 32 bit RGB value.


### 3.2. Terminfo database files.
The terminfo database files contain information about each terminal types
capabilities and supported escape sequences.  uCurses queries the current
environment for a `$TERM` variable to determine which terminal it is
running in and it will then attempt to load the terminfo file for that
specific terminal.

If it is not able to find any terminfo file for the current terminal then
this is considered a critical error and the application will abort.

uCurses will look in the following directories for this file.

- /usr/share/terminfo
- /lib/terminfo
- /etc/terminfo

I personally consider /lib/terminfo to be an utterly incorrect place to
put these files but that's where Debian places them (why?).  The terminfo
man pages also suggest that ~/terminfo might be a possibility too but this
library does not check there as I consider it unsafe.

The following files are used to search for and load the correct terminfo
database file for the current console.

```c
static bool try_path(int i, const char *env_term)
```

Attempt to open the current consoles terminfo file from one of the
above stated directory paths.

If the specified file is found this function will memory map it
and return a true result.  If, during this operation uCurses
encounters an out of memory error the application will abort.

If the file is not found this function will return a false result.

```c
static void map_tifile(void)
```

This function first attempts to read the `$TERM` environment
variable to determine which terminal type the application is
running in.  If this variable is not set uCurses will abort.

If the environment variable is set then this function will search
each of the above specified directories for a file with this name.
Not finding this file in any of the above directories is a
critical error and the application will abort.

If no abort happens then the caller can assume the file was found
and is now memory mapped.

```c
static void is_valid(void)
```

This function verifies that the recently memory mapped terminfo
file is valid.  It compares the terminfo files "magic" signature
to known values.  If this entry is invalid we abort.

This magic signature is in the files header and can be one of two
values as follows...

- `0x011a`
- `0x021e`

If it is the latter then this signifies that the numbers section
within this terminfo file uses 32 bit entries instead of the old
16 bit entries.

```c
void alloc_info(void)
```

This function calls the above functions to memory map the current
consoles terminfo database file and to verify its validity.  If
the file is valid then pointer addresses are calculated to the
various sections contained within that mapping.

Note: Not all of these tables are utilized by this library but we
still set pointers to them for possible future use.

Pointers to The following sections are initialized.

- Names section.
- Bool section.
- Numbers section.
- Strings section.
- String table section.

uCurses is mainly interested in the strings and string table
sections.   The strings section is an array of offsets to format
strings contained within the string table section.  The order of
items in the strings section is well defined so the offset to a
specific escapes sequence format string can easily be found.


### 3.3. `uC_console_attribs.c`
This file contains public API calls which set or clear the various
attributes on the terminal.

```c
API void uC_console_reset_attrs(void)
```

This function writes an escape sequence out to the terminal that
will reset its foreground and background colors to the uCurses
defined defaults.   These can be changed.

```c
API void uC_console_set_fg(uC_color_t color)
```

This function writes an escape sequence out to the terminal that
sets its foreground color to the specified value.

```c
API void uC_console_set_bg(uC_color_t color)
```

This function writes an escape sequence out to the terminal that
sets its background color to the specified value.

```c
API void uC_console_set_gray_fg(uC_colors_gray_t color)
```

This function writes an escape sequence out to the terminal that
sets its foreground color to a specified gray scale value.

```c
API void uC_console_set_gray_bg(uC_colors_gray_t color)
```

This function writes an escape sequence out to the terminal that
sets its background color to a specified gray scale value.

```c
API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g,
uC_color_t b)
```

This function writes an escape sequence out to the terminal that
sets its foreground to a specified 24 bit RGB color.

```c
API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b)
```

This function writes an escape sequence out to the terminal that
sets its background to a specified 24 bit RGB color.

```c
void console_clr_flags(uint16_t flags)
```

This function writes all the escape sequences required to clear a
specified set of flags which control things like bold, underline,
reverse video being active.   Clearing certain flags and thereby
modifying the state of the console can require multiple escape
sequences be emitted to ensure that other attributes are not lost.

```c
void console_set_flags(uint16_t flags)
```

This function writes all the escape sequences required to set a
specified set of flags which control things like bold, underline,
reverse video being active.   A flag being set will have the
library write the specific escape sequences required to affect
that change.

The following public API calls use the above functions to enable or
disable various attributes in the console.  Turning either underline mode
or bold mode off has some associated caveats documented elsewhere here.

```c
API void uC_console_clr_attr(int16_t flags)
```

This function clears the specified attribute flag bits from the
terminal's current console attributes and reapplies the result.

```c
API void uC_console_set_ul(void)
```

This function writes the escape sequence which will turn on
underline mode for all characters subsequently written to the
terminal.

```c
API void uC_console_set_rev(void)
```

This function writes the escape sequence which will turn on
reverse video mode for all characters subsequently written to
the terminal.

```c
API void uC_console_set_bold(void)
```

This function writes the escape sequence which will turn on
bold mode for all characters subsequently written to the
terminal.

```c
API void uC_console_clr_ul(void)
```

This function writes the escape sequences which will turn off
underline mode for the terminal.

```c
API void uC_console_clr_rev(void)
```

This function writes the escape sequences which will turn off
reverse video mode for the terminal.

```c
API void uC_console_clr_bold(void)
```

This function writes the escape sequences which will turn off
bold mode for the terminal.


## 4. Keyboard Input
Keyboard input from a terminal is not as simple as reading a single
character for every single key press.  Certain keys return entire escape
sequences as described above and the escape sequence returned by a certain
key on a certain terminal might not be the same sequence returned by the
same key on any other terminal.

For this reason, some (not all) key presses that return an escape sequence
will have an associated format string entry in the terminfo database for
that terminal.

FYI:  Keyboard input in Linux / Unix terminals is an absolute
NIGHTMARE.  Ancient technology is the albatross around the modern
developers neck.  It is for this reason that VI uses escape colon
shift `F10` backspace `F3` to enable overwrite mode and the like (pretty
sure that's exactly what it uses).

P.S. I believe Termcap was implemented by the original author of vi
specifically to enable him to reliably interpret keyboard input on any
of the then existing terminal types.   Termcap is the precursor to the
more modern terminfo used here.

There is also the problem of being able to read and reliably recognize key
presses with modifier keys (control, alt, shift, meta).  uCurses targets
xterm-compatible terminals which expose most modifier combinations as
well-defined escape sequences.  Full cross-terminal support for every
possible modifier combination is not feasible and is not a goal;  Also,
any key press that is intercepted by the desktop environment or the
terminal emulator itself is simply not available to the application.

P.S.  The way uCurses handles the keyboard is not as clean as I would
like it to be and I am considering some changes but I am resisting
changes that would necessitate making uCurses SUID root such as
reading keyboard scan codes directly (tempting though!).

It would be REALLY REALLY nice and helpful (hint hint) if the Linux
kernel had an API to return flags for which modifier keys were
currently pressed.   Pretty plzkthxbai?


### 4.1. `uC_key_read.c`
This source file contains all the code related to the reading of and
recognition of key presses.  All characters associated with a given key
press are written into a keyboard input buffer for later interpretation.
A pollfd structure is used to test if the file descriptor associated with
terminal input has any characters ready to read.

```c
API int8_t uC_test_keys(void)
```

This function will return a result of zero if there are no
characters ready to read from the terminals standard input file
descriptor.  If there are any characters ready to read then this
function will return a non zero result.

```c
static int8_t read_key(void)
```

This helper function will repeatedly read the terminals standard
input file descriptor until a valid character is returned.  This
is somewhat janky (no pun intended) because it could result in an
infinite loop (tm).

On success, this function returns the character read.

```c
void uC_read_keys(void)
```

This call which is only internally visible to the library, will
repeatedly read the terminals standard input file descriptor until
there are no more characters left to read or the buffer becomes
full.  All characters are saved to the keyboard buffer array and
the number of characters read is saved in the num_k variable.

It is assumed that after having read one character from standard
input, if there are still characters available to be read then
every available character is associated with the press of a single
key.  I.E.  We are reading a key sequence not multiple separate
key presses.

This assumption can interfere with type-ahead.  The standard
mitigation is a short read timeout (typically 10-50ms) after
seeing an `ESC` byte: if further bytes arrive within the window
the `ESC` opens a sequence; if not, it is a bare `ESC` key press.
This approach is under consideration for a future revision.


### 4.2. `key_sequence.c`
When a key is pressed and that key press returns an escape sequence the
characters of that escape sequence are all saved to the keyboard input
buffer.

This code will then interpret each terminfo format string associated with
keyboard input and compile its escape sequence into the escape buffer (the
same one used for output).

The compiled escape sequence is then compared with the one in the keyboard
input buffer.  If the escape sequence in the escape buffer is identical to
the escape sequence in the keyboard input buffer then we know exactly
which key was pressed.   If not, we try again till we run out of format
strings to try.

uCurses does not support every possible key press and I may add entries to
handle some of the missing items but even then, I still don't have a
reliable way of interpreting the infinite combinations of control, alt,
shift, meta plus key.  see above "pretty plzkthxbai".

```c
static void set_esc0(uint8_t c)
```

This helper function is used to inject a single character key
press into the terminfo escape sequence buffer and set the input
length to one.  This is used internally to translate a key
sequence into a single key value which can be returned to the
applications keyboard input functions.

```c
static void kent(void)
```

This helper function injects a hard coded `0x0a` character (the
enter key) into the terminfo escape sequence buffer to be compared
with which ever key which was actually pressed.

```c
static void kbs(void)
```

This helper function injects a hard coded `0x08` character (the
backspace key) into the terminfo escape sequence buffer to be
compared with which ever key was actually pressed.

NOTE: The backspace key can actually return either `0x08` or `0x7f`
utterly independent of what value is specified within the
terminals terminfo database.  In fact, the same terminal, in the
same distribution can potentially return either of these values
entirely based on which maintainer compiled it, what their mood
was at the time or what specific planetary alignment existed at
that time. UTTER FUCKING INSANITY!

If a terminfo database specifies that a press of the backspace key
returns `0x08` then it should damned well return `0x08` and *ONLY*
`0x08`.  If the terminfo specifies `0x7f` then it should return `0x7f`
and *ONLY* `0x7f`.  PERIOD.  This REALLY needs to be fixed.

```c
static void kcuu1(void)   static void kcud1(void)
static void kcub1(void)   static void kcuf1(void)
static void kdch1(void)   static void kich1(void)
static void khome(void)   static void kend(void)
static void kcbt(void)    static void knp(void)
static void kpp(void)     static void kf1(void)
static void kf2(void)     static void kf3(void)
static void kf4(void)     static void kf5(void)
static void kf6(void)     static void kf7(void)
static void kf8(void)     static void kf9(void)
static void kf10(void)    static void kf11(void)
static void kf12(void)
```

Each of these helper functions references one of the many macros
within the `uC_terminfo.h` source file.  They will each have their
specific keys format string interpreted and their associated
escape sequence compiled into the escape buffer.

The contents of this buffer can then be compared with the escape
sequence which was previously read into the keyboard input buffer
so that uCurses can recognize which key was pressed and take the
appropriate action based on that key press.

```c
void (*k_table[])(void)
```

This is an array of function pointers to each of the above helper
functions.  This table is woefully incomplete.  Each entry in this
array will be called in sequence so that the escape sequence for
each supported key can be compared against which ever key was
actually pressed.  Loop till match found or loop done (tm).

```c
int16_t match_key(void)
```

This internal function is used to compare compiled escape
sequences for each supported key with the sequence which was
previously read into the keyboard input buffer.  This code does
not use the 'unsafe' memcmp function but instead computes a FNV-1
hash for each of the items to be compared.  This may very well be
just as unsafe.

If a match is found the index of that match is returned.  If not
then a result of -1 is returned.

TODO: make an init run over those format strings to compile a list of
escape sequences to compare aginst ONE TIME ONLY, not for every single
key press.


### 4.3. `uC_key_table.c`
The tables contained within this source file allow the application
developer and the library itself to define behaviors for each supported
'special' key press.  A matched escape sequence calls a handler which may
stuff a single synthetic key code into the keyboard buffer.  The caller
then receives that key code exactly as though it had been typed directly.

Common non-printing keys now have stable `UC_KEY_`* values.  The cursor
keys, insert/delete, home/end, page up/down, back-tab and `F10` therefore no
longer disappear by default.  Applications can still allocate their own
key table and override individual handlers when a different translation is
needed.

The currently reserved single-byte return values are:

```text
UC_KEY_NONE     0x00
UC_KEY_BS       0x08
UC_KEY_TAB      0x09
UC_KEY_ENTER    0x0a
UC_KEY_ESC      0x1b
UC_KEY_UP       0x81
UC_KEY_DOWN     0x82
UC_KEY_LEFT     0x83
UC_KEY_RIGHT    0x84
UC_KEY_INSERT   0x85
UC_KEY_DELETE   0x86
UC_KEY_HOME     0x87
UC_KEY_END      0x88
UC_KEY_MOUSE    0x89
UC_KEY_F10      0x8a
UC_KEY_BACKTAB  0x8b
UC_KEY_PGDN     0x8c
UC_KEY_PGUP     0x8d
```

These values are what application code sees after terminfo sequence
matching and default key translation.  `F1` through `F12` also have key table
indexes, but most function keys default to `uC_noop` until an application
or a library subsystem assigns behavior to them.  This keeps the key
table stable without making F keys accidentally active.

The key_index_t enumeration is the index into the active handler table.
It currently covers enter, arrows, backspace, delete, insert, home, end,
page down, page up, back-tab and `F1` through `F12`.  The enumeration order is
part of the handler-table contract; any custom table must use those
indexes rather than assuming the physical order in the source file is
unimportant.

The public `uC_key()` path is now the consolidated dispatcher.  It reads one
decoded key, gives the screen-level shortcut registry first chance to
handle it, then gives the menu system a chance to handle menu navigation,
then gives the widget system a chance to handle active widgets.  If none
of those systems consume the key it is returned to the application.

When a uCurses textbox is actively being edited the global shortcut
registry is skipped automatically.  Focus alone is not editing; a textbox
may be selected for navigation without owning printable input.  This
prevents printable shortcut keys from being stolen while text is being
edited.  Applications that implement their own text editors should disable
screen shortcuts while those editors own input.

Shortcuts are represented by `uC_shortcut_t`.  The low byte holds the base
key and the upper bits hold modifier flags:

```text
UC_SHORTCUT_KEY_MASK
UC_SHORTCUT_MOD_CTRL
UC_SHORTCUT_MOD_ALT
UC_SHORTCUT_MOD_META
```

The preferred construction macros are:

```c
UC_SHORTCUT(k)
UC_SHORTCUT_CTRL(k)
UC_SHORTCUT_ALT(k)
UC_SHORTCUT_META(k)
```

Plain alphabetic shortcuts are normalized case-insensitively, so a
registered R also matches r.  Control shortcuts are matched against the
normal ASCII control-code range, so `UC_SHORTCUT_CTRL('X')` matches a
received `0x18`.  Alt and Meta shortcut values can be stored and displayed
by the menu code but are not matched by the current keyboard reader.
Those require the future byte-at-a-time escape sequence state machine.
Once that parser exists, the intent is to match shortcut modifiers more
generally, including reasonable combinations of Control, Alt and Meta
where the terminal can report them unambiguously.

Menu shortcut display uses the same `uC_shortcut_t` value.  Plain printable
shortcuts display as their uppercase base key, control shortcuts display
as ^X, Alt shortcuts as A-X and Meta shortcuts as M-X.  Modifier prefixes
can combine for display only; for example a Ctrl+Alt shortcut can be
shown in a pulldown, but the current input reader will not execute any
shortcut containing Alt or Meta.  Only plain shortcuts and Ctrl shortcuts
are currently matched.

```c
API void uC_set_key(uint8_t c)
```

This public API call can be used by user applications to inject a
key into the keyboard input buffer as though an actual key press
had occurred.  It can also be used to translate a key sequence
into some single character value to be returned to the user
application. (The widgets key loop does this!)

```c
API void uC_flush_keys(void)
```

This public API call is used to erase the keyboard input buffer.

```c
static void k_bs(void)      static void k_ent(void)
static void k_up(void)      static void k_down(void)
static void k_left(void)    static void k_right(void)
static void k_delete(void)  static void k_insert(void)
static void k_home(void)    static void k_end(void)
static void k_pgdn(void)    static void k_pgup(void)
static void k_cbt(void)
```

These static functions are used internally to translate certain
well known keys which occasionally return different values into a
consistent result.  They can also be used to simply add a given
key press value which was matched against as an escape sequence.
The enter key for example.

```text
uC_key_handler_t *default_key_actions[]
```

This table contains function vectors to the default handlers for
each key press which uCurses currently recognizes.  Application
developers can allocate their own tables and modify the actions
within them based on application context.

```c
API uC_kh_t uC_alloc_kh(void)
```

This public API call is used to allocate a new keyboard function
table.  There can only be one active keyboard table at a time so
when a new one is allocated the previous one is returned and must
be saved.

When the current application context has finished or is exited
the saved handler must be reinstated or bad things could happen!

```c
API void uC_release_kh(uC_kh_t saved)
```

This public API function is used to release an allocated key table
and to restore the saved key table.

For example, a widget handler can release its own keyboard table
and restore the previously active table when the widget editor is
closed.

```c
API uC_key_handler_t *uC_set_key_action(key_index_t index,
uC_key_handler_t *action)
```

This public API call is used to set a specific keys action within
the applications allocated table of keyboard handlers.

```c
uint8_t uC_key_raw(void)
```

This internal function performs the blocking read and escape
sequence translation step only.  It does not route the resulting
key through screen shortcuts, menus or widgets.  The consolidated
`uC_key()` dispatcher and a few internal legacy loops use this when
they need to avoid recursively dispatching the same key.

```c
bool uC_shortcut_register(struct uC_screen_s *scr,
uC_shortcut_t shortcut, uC_shortcut_action_t *action,
void *context, void *owner)
```

This internal helper registers a screen-level shortcut.  It is
used by menus and by widget button shortcuts.  Other uCurses
internals may use the same registry when they need global,
screen-scoped keyboard accelerators.  Duplicate registered
shortcuts on the same screen are rejected and the function returns
false.  The owner pointer is used for automatic deregistration
when a menu item, menu bar, widget button or widget view group is
disabled, detached or closed.

```c
bool uC_shortcut_run(struct uC_screen_s *scr, uint8_t key)
```

Scans the screen shortcut registry for a matching key.  If one is
found the registered action is executed and the original key is
consumed.  The action may stuff a replacement application key with
`uC_set_key()`; if it does not, `uC_key()` returns `UC_KEY_NONE`.

```c
bool uC_shortcut_run_popup(struct uC_screen_s *scr, uint8_t key)
```

Scans the screen shortcut registry for a matching key owned by a
widget inside the currently attached popup widget view group.  This
is used by the modal popup path in `uC_key()`.  When a widget popup
is active, normal menu shortcuts and normal background widget
shortcuts are ignored; only shortcuts belonging to widgets in the
active popup are allowed to run.

```c
void uC_shortcut_remove_owner(struct uC_screen_s *scr, void *owner)
void uC_shortcut_clear(struct uC_screen_s *scr)
```

Remove shortcuts associated with a specific owner or clear the
whole screen registry.  Menu items and widget buttons use
themselves as owners so registry entries are cleaned automatically
when the menu bar is closed or when a widget view group is detached
or closed.

```c
API uint8_t uC_key(void)
```

This public API call is the main keyboard input handler for the
uCurses library.  When called it will remain blocked until a key
press is available.  It first obtains a decoded key through
`uC_key_raw()`, then routes that key through the screen shortcut
registry, the menu handler and the widget handler before returning
any unhandled key to the caller.

Popup dispatch is modal.  If a raw popup window is active without a
widget popup, `uC_key()` skips normal screen shortcuts, menus and
widgets and returns the decoded key to the application.  If a
widget popup is active, `uC_key()` allows only shortcuts owned by
widgets in that popup and routes widget input to the popup widget
state; menu handling and background shortcuts remain blocked.

A matched shortcut or UI handler consumes the original key.  If
that handler stuffs a replacement key, the replacement is returned
to the application.  If it does not, `uC_key()` returns `UC_KEY_NONE`.


### 4.4. `uC_mouse.c`
This long overdue module adds SGR mouse protocol support.  When the widget
system is enabled (`UC_WIDGETS` build flag) mouse reporting is enabled
automatically by `uCurses_init()` and disabled by `uCurses_deInit()`.
Applications that bypass the widget system can enable and disable mouse
reporting manually.

```c
API void uC_mouse_enable(void)
```

Enables SGR mouse reporting: button press/release, drag, and
all-motion (hover) tracking.  These are enabled as a fixed set;
individual modes cannot be toggled independently.  For most
applications the enabled set is appropriate for the lifetime of
the program.

```c
API void uC_mouse_disable(void)
```

Disables all SGR mouse reporting.  Called automatically by
`uC_restore_terminal()`.

```text
uC_mouse_event_t uC_mouse_event
```

Global structure filled by the mouse parser whenever `uC_key()`
returns `WIDGET_KEY_MOUSE` (`0x89`).  Fields:

```text
int16_t x       1-based terminal column (Ansi coordinates)
int16_t y       1-based terminal row
uint8_t button  button/event type (see constants below)
bool pressed    true = press or wheel tick, false = release
```

Button constants defined in `uC_mouse.h`:

```text
UC_MOUSE_LEFT      0   left button
UC_MOUSE_MIDDLE    1   middle button
UC_MOUSE_RIGHT     2   right button
UC_MOUSE_MOVE      35  motion with no button held
UC_MOUSE_WHEEL_UP  64  scroll wheel up
UC_MOUSE_WHEEL_DN  65  scroll wheel down
UC_MOUSE_WHEEL_LT  66  horizontal scroll left
UC_MOUSE_WHEEL_RT  67  horizontal scroll right
```

Wheel events always arrive as pressed=true; there is no release
event for scroll wheel ticks.

```c
bool uC_mouse_parse(void)   [visibility hidden]
```

Called internally by `uC_key()` to detect and parse the SGR escape
sequence `ESC`[<btn;x;yM|m from the key buffer.  On success fills
`uC_mouse_event` and stuffs `WIDGET_KEY_MOUSE` into the key buffer
so the caller sees it as a normal key return value.

TODO: this mouse interface needs to not enable position reporting by
default.  it should have a public API to turn it on / off but widgets and
menus should only query position on click.


## 5. The Screen
While the terminfo parser is the heart of the uCurses library (for both
input and output), the screen structure is the forward facing visual part
of it.  A screen structure is an array of UTF-8 characters and their
individual associated attributes.  All windows, widgets and menu related
thingies are drawn into the screen buffers to be later blasted out to the
terminal for display.

At the time of writing there can be only one active screen but I may in
the future add the ability to have multiple screens and allow flipping
between them in a way similar to how the Amiga did it (shiny!).  This
however is not very high on my list of potential todo's because it is
antithetical to the purpose of the library remaining as resource friendly
as possible.

This could also be added as a build option but to be honest I actually do
not like adding too much of that.  Conditional compilation can turn
simple, well presented sources into a horrendous, unreadable cluster
($&^%*#)! and I absolutely reject sources that look like that, even my
own.

The screen structure contains the following entities, all of which are
described in detail later in this document ...

- A list of application defined windows to be drawn into the screen
  character buffers.
- A list of status windows attached to the screen.
- A list of widget view groups attached to the screen.
- A list of screen-level keyboard shortcuts.
- A pointer to a popup window, if `UC_POPUPS` is enabled.
- A pointer to the built-in too-small popup, if `UC_POPUPS` is enabled.
- A pointer to an attached menu bar.
- A pointer to which ever window currently has focus.
- Variables specifying the screens dimensions and optional minimum
  usable dimensions.
- Variables specifying the current cursor location within the screen.
- A flag controlling whether screen-level shortcuts are currently
  enabled.
- Pointers to the screens current and previous character buffers.

The screen-level keyboard shortcut list is an internal registry, not an
application-managed list of key bindings.  Menus and widgets register
their accelerators here so `uC_key()` can dispatch shortcuts before normal
menu navigation or widget handling.  The encoding and matching rules for
these shortcuts are documented in section 4, Keyboard Input.


### 5.1. `uC_screen_ctrl.c`
This source file contains all the code needed to create, destroy and
control a screen structure.  It also contains a few functions specific to
the control or placement of windows because putting them here simplified
the Gordian knot of circular include dependencies.

```c
int16_t scr_alloc(uC_screen_t *scr)
```

This function is slightly mis-named: it does not allocate the
screen structure itself.  It requires an already-allocated,
partially initialized screen structure to be passed in.

It allocates two cell buffers sized to the screen dimensions.  The
first buffer holds every character and its attributes for the
current (new) frame.  The second holds the same data for the
previous frame.  On each update only cells that differ between the
two buffers are written to the terminal, which makes the update
procedure significantly more efficient.

This function was factored out primarily to give it a descriptive
name and to allow the JSON parser to call it directly.  This is a
good illustration of why even trivial anonymous blocks benefit
from being extracted into named functions — it makes code
significantly more readable at essentially no cost.

```c
API uC_screen_t *uC_scr_open(int16_t width, int16_t height)
```

The main API call for creating a new screen.  Width and height
should match the current terminal dimensions; passing mismatched
values is undefined behavior (tm).

After allocating the structure and setting its dimensions it calls
`scr_alloc()` to set up the two state buffers.  If that allocation
fails the structure is freed and `NULL` is returned.  On success the
new screen is returned and also stored in the library-internal
active_screen variable.

The screen-level shortcut registry is initialized here and
shortcuts are enabled by default.

```c
API void uC_scr_enable_shortcuts(uC_screen_t *scr)
API void uC_scr_disable_shortcuts(uC_screen_t *scr)
API bool uC_scr_shortcuts_enabled(uC_screen_t *scr)
```

These public API calls control the screen-level keyboard shortcut
registry used by the menu and widget systems.  Registered
shortcuts can be disabled (required during widget text box
editing) and they remain attached to the screen, but `uC_key()` will
not process them until they are re-enabled.

```c
API void uC_scr_close_view_groups(uC_screen_t *scr)
```

This public API function is available only when the library is
built with `UC_WIDGETS` enabled.  It is called as part of the
interface tear-down procedure.  It iterates through every widget
view group attached to the screen; for each group it closes all
attached views and their widgets, then closes the group itself.
Repeat until no view groups remain.

Making this function public lets application code tear down the
widget interface without destroying the rest of the UI.  This can
be used to rebuild the widget interface on the fly based on
application context.

Technically this belongs in the widget sources but placing it here
avoids giving those sources any knowledge of the screen structure.

```c
API void uC_scr_close(uC_screen_t *scr)
```

Closes a screen and deallocates everything associated with it in
the following order:

- The two state buffers.
- The backdrop window (if any).
- All application windows.
- The popup and too-small popup windows (if `UC_POPUPS`).
- All widget view groups, views and widgets (if `UC_WIDGETS`).
- The menu bar, pull-downs and menu items (if `UC_MENUS`).
- The screen-level shortcut registry.
- Any status windows.
- The screen structure itself.

If the closed screen is the current active_screen that internal
pointer is cleared to prevent stale access.

```c
void init_backdrop(uC_screen_t *scr, uC_window_t *win)
```

Initializes a backdrop window and attaches it to the screen.  The
backdrop always fills the screen minus a one-cell margin on each
side (to leave room for its border), is scroll-locked, and is
given a single-line border with pre-defined gray-scale foreground
and background colors.

Application developers can modify any of these defaults after the
fact by pulling the window address out of scr->backdrop and using
the normal window API calls.

The backdrop is always the first entity drawn on a screen update
so all other windows and entities are composited on top of it.
This prevents moving windows from leaving trails on the display.

```c
API void uC_scr_add_backdrop(uC_screen_t *scr)
```

Allocates a new window and calls `init_backdrop()` to configure it
as the screen's backdrop.  This is now called automatically by
`uCurses_init()` for programmatic applications (those that pass `NULL`
for all three parameters).  JSON-based applications have their
backdrop created via the JSON build path.  Application code should
not normally need to call this directly.

```c
int16_t win_chk_pos(uC_window_t *win, uC_screen_t *scr, int16_t x,
int16_t y)
```

Verifies that a proposed window placement is legal.  No part of
any window, including its border, may fall outside the bounds of
the screen.  Returns 0 if the placement is valid, -1 otherwise.

```c
API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win)
```

Attaches a window to a screen, making it visible on the next
update.  If the window is already attached to any screen it is
first detached from there.  Windows are drawn in list order so
attaching a window pops it to the front of the view (it is drawn
last, i.e. on top).  This mechanism also provides a clean path for
moving windows between screens should multiple screens ever be
implemented.

```c
API void uC_scr_win_detach(uC_window_t *win)
```

Detaches a window from whichever screen it is currently attached
to and removes it from the view.  The window structure is not
destroyed.  If the window is not attached to any screen this call
is a silent no-op.

```c
API void uC_scr_set_min_size(uC_screen_t *scr, int16_t width,
int16_t height)
```

Sets optional minimum usable screen dimensions.  When `UC_POPUPS` is
enabled, uCurses compares these values against the current screen
dimensions during screen drawing.  If the screen is smaller than
either minimum and there is enough room for a small overlay,
uCurses draws a compact centered "Too small" popup as the final
layer.  Applications still decide whether to build their normal
layout when below that minimum; this API only provides the common
visual warning.

```c
API void uC_scr_popup_attach(uC_screen_t *scr, uC_window_t *win)
```

Available when `UC_POPUPS` is enabled.  Attaches a normal window as
the screen's popup instead of adding it to the normal window list.
There is only one popup slot per screen; attaching a new popup
detaches the previous popup without destroying it.  Popup windows
are intended for modal overlays and transient information panels.
They are drawn after widget view groups, menus, and status windows.
uCurses does not assign a lifetime, timeout, or input behavior to
popups; application code decides when to detach them.

```c
API void uC_scr_popup_detach(uC_window_t *win)
```

Available when `UC_POPUPS` is enabled.  Detaches a popup window from
whichever screen it is currently attached to.  The window structure
is not destroyed.

```c
API void uC_scr_popup_cancel(uC_screen_t *scr)
```

Available when `UC_POPUPS` is enabled.  Detaches the currently
attached raw popup window, if any, and the currently attached
widget popup view group, if any.  Neither popup object is
destroyed.  This is used internally when resize handling observes
`SIGWINCH` so popup attachments do not survive a screen rebuild.
Applications may also call it when they want a single "cancel any
active popup" operation.

```c
API bool uC_widget_popup_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
```

Available when both `UC_POPUPS` and `UC_WIDGETS` are enabled.  Attaches
a widget view group as the screen's modal popup.  The popup view
group is drawn in the popup phase, after normal widget view groups,
menus and status windows.  Normal widget view groups on the screen
are marked inactive while the popup is attached, but they are not
detached or destroyed.

There is only one widget popup slot per screen.  Attaching a new
widget popup detaches the previous widget popup first.  Popup view
groups may contain normal widgets.  Attach selects the first
available widget tab stop in the popup so popup widgets immediately
receive keyboard input.  Application code may explicitly select a
different widget afterward and still decides what key closes the
popup.

```c
API void uC_widget_popup_detach(uC_widget_vg_t *vg)
```

Detaches the specified widget popup, removes any button shortcuts
owned by its widgets, clears focus if the popup owned focus, and
restores the inactive state of normal widget view groups.

```c
API void uC_scr_win_tab_next(uC_screen_t *scr)
```

uCurses allows windows to have input focus and for application
context to be set accordingly.  Each window can have its own
dedicated keyboard handler.

This call advances focus to the next window in the tab order for
the specified screen.  The tab order is entirely independent of
the window draw order.


### 5.2. Drawing it all
Drawing the screen is a multi step process.  The first step is to emit a
sgr0 escape sequence to obliterate any and all attributes associated with
the previous screen draw.  This was added to account for the case where
any characters in the previous frame were underlined and said underlined
characters were drawn last, leaving that state active.  This ensures that
underline or any other potentially incorrect attributes are made inactive.

Next the screens backdrop window (if any) is drawn into the screens state
buffer followed by all the user application windows.

Widget view groups are drawn next.  If the application makes use of pull
down menus these are drawn after widgets, then any status windows are
drawn.

Note: A status window is an optional 'one line' high window which can
be used to display status information such as current line in a file
being edited or your characters current map coordinates in a game etc
etc, you decide!  I use the status window as a debug printf type aid
as I develop this library (tm)!

Finally, any popup windows are drawn.  A popup window can be informational
or have any type of widget within it.

At this stage the screens state buffer contains the state of every
character and their associated attributes which now all need to be blasted
out to the console as fast as possible.


### 5.3. Updating the console
The console update is performed by two nested loops called `inner_update()`
and `outer_update()`.  When the outer loop is called it scans the new screen
state buffer and compares it with the previous state buffer.  If any
character is found to have been modified during the most recent update
then that characters index is passed to the inner update loop described
below.

The outer update loop is repeated until every modified character found has
been passed to the inner loop.

The `inner_update()` function will first emit the escape sequences required
to set the attributes associated with the character at the index passed to
it.

It will then, scan the screen state buffers from the current (passed in
index) to the end for any modified characters that share those attributes.
For each character found it will then, if necessary, emit the escape
sequence to move the cursor to that characters location.  It will then
emit the character itself.

During this scan, if it finds a modified character with a different
attribute it will remember this characters index and return it to the
outer loop function (only the first one found).  This allows the outer
loop to skip all the unmodified characters the inner loop just scanned
over and continue from the first known modified character with a new set
of attributes.

If the inner loop scans to the end of the display without finding any
further modified characters then it will return an index of zero to the
outer loop informing it that the entire update is now complete.

The escape buffer will now contain every single escape sequence and every
single character for this new state of the display.  These are now written
out to the terminal with a call to the Linux `write()` function.

This means that most of the time only one single (slow) 'user space' to
'kernel space' transition will be required to update the ENTIRE display.

P.S.  If, during the compilation of those escape sequences the escape
buffer is filled then it will need to be flushed prior to compiling
any new escape sequences so..  In fact, the screen update could take
two or (unlikely) more calls to `write()`.  Ignore the man behind the
curtain!

Note: write-call count tracking will be added as a debug build option
alongside buffer high-water-mark tracking.


### 5.4. `uC_screen_draw.c`
The functions in this file perform the above console update procedure.
They write all windows, menus, widgets or other thingies into the screen
state buffer and then write that new state out to the console for display.

```c
static void draw_win_name(uC_window_t *win)
```

When a window is drawn it may or may not have a border of some
type.  If it has a border it may or may not also have a name.  If
it has a name then that name will be drawn into a small part of
the area taken up by said border characters.  (top left ish).

```c
void scr_draw_win(uC_window_t *win)
```

This function draws one window and its contents and any border
that may or may not be present into its associated screen state
buffer.  The window structure itself contains a pointer to the its
parent screen.

If either the window pointer passed to this function or the screen
pointer within the referenced window structure is `NULL` then this
function will silently perform no action.

```c
static void scr_draw_windows(uC_list_t *list)
```

This function will, given a pointer to a list of windows, draw
every window contained within that list.  If the list is empty
this function will silently perform no action.

```c
static void scr_cup(uC_screen_t *scr, int16_t x, int16_t y)
```

This function moves the console cursor location to a new position
by emitting a `cup` escape sequence.  If however the
console cursor position (which is tracked) is already in the
desired new location then this function silently performs no
operation (I hope!).

```c
static bool scr_is_modified(uC_screen_t *scr, uint16_t index)
```

This function compares the contents of the indexed cell within the
current (new) state of the screen buffers with the cell at the
same index within the screens previous state buffer.

If either the cells attributes or the cells UTF-8 encoded
character have been modified then this function returns a true
result.  If neither the attributes nor the character have been
modified this function returns a false result (no update required
for this char).

```c
static void new_attrs(uC_attribs_t a)
```

If, during the screen update procedure a new set of attributes is
required, those attributes are written into the global attributes
variable and a call is made to `apply_attribs()`.  This will write
an escape sequence to the output buffer to affect any selected
attributes changes.

```c
static void scr_normalize_wide_buffer(uC_screen_t *scr)
```

Scans the fully composed screen buffer before it is compared with
the previous screen state.  If a `DEADC0DE` cell no longer has a
valid owning wide character immediately to its left, or if a wide
character no longer has the expected `DEADC0DE` continuation cell,
the invalid half is converted into a normal space.

This protects direct screen overlays such as borders and pulldown
menus.  The window buffer code already performs the same cleanup
for ordinary writes into windows.

```c
static void _scr_emit(uC_screen_t *scr, int16_t index, cell_t *p1,
cell_t *p2)
```

This somewhat complex function writes a single character out to
the terminal.  Care must be taken to ensure that the cursor
location is correctly tracked because emitting one character can
in fact advance the cursor more than one space on the display.
This is because UTF-8 characters can be multi width.

Note: uCurses supports 1-wide and 2-wide characters.
Zero-width combining characters and bidi text are out of
scope; see section 10.8 for the rationale.

This code also accounts for conditions where a single width
character is emitted over the top of one or other half of a double
width character.  This prevents glitches that make the screen look
very messy but doing so makes the code look messy.  You can't win!

```c
void scr_emit(uC_screen_t *scr, int16_t index)
```

Writes the character at the current index out to the escape
buffer.

If this cell is the continuation cell of a double width character
then its code point will be `DEADC0DE`.  This is an internal
sentinel, not a printable character.  The left cell owns the real
glyph and the `DEADC0DE` cell only records that this screen column
is already occupied by the glyph that began in the previous
column.  This cell is not emitted directly.

After writing the character (or not) the data at the current index
within the current state is written into the screens previous
state buffer.  This marks that cell as having been updated.

I.E.  The contents of both buffers at the specified index will now
be the same.

```c
static int16_t inner_update(uC_screen_t *scr, int16_t index,
int16_t end)
```

Given the index of a character that has been modified since the
last time the screen was drawn this function will set the new
characters attributes by emitting all escape sequences associated
with them.  This includes setting foreground and background colors
and turning bold, underline or reverse video on or off for that
character.

This function then proceeds to scan for and write every single
modified character within the screens state buffer that shares
these new attributes, relocating the cursor as required.

The inner loop will save the index of the first modified character
that it finds that does not share these attributes and will return
that index to the outer loop function documented below.  If the
inner loop scans all the way to the end of the screen without
finding any such character it will return an index of zero
indicating to the outer loop that the update is complete.

```c
static void outer_update(uC_screen_t *scr)
```

This function loops over every character within the current state
buffer of the screen and compares it with the character in the
previous state buffer at that same index.  If this character has
been modified its index is passed to the `inner_update()` function
which will update that character and every other character that
shares identical attributes.

```c
API bool uC_scr_resize_hold(uC_screen_t *scr)
```

This public API function is used after `SIGWINCH` has been observed
and `uC_winch_pending()` returns true.  It snapshots the current
screen, acknowledges the pending `WINCH` state, draws a temporary
shadow view during the resize stream, and waits until the resize
has been quiet for a short interval.  It returns true when it
handled a pending resize and false when there was no pending
resize to handle.

If `UC_POPUPS` is enabled, this function calls `uC_scr_popup_cancel()`
before drawing the resize hold view.  Popup attachments are
therefore cancelled before the application rebuilds the interface
for the new terminal dimensions.  The popup objects themselves are
not destroyed.

```c
API void uC_scr_draw_screen(uC_screen_t *scr)
```

This public API function is what an application would call to
update the display of the entire screen state.  Applications will
need to call this function on an as needed basis as it is not
called automatically.

It first emits an sgr0 escape sequence to the escape buffer to
ensure that no unwanted attributes are currently active.  It then
draws the screens backdrop window followed by all user application
windows.

Any active widgets are then drawn into the buffers.  If the user
application has menus defined, the menu bar and any active
pulldown are drawn after widgets so pulldowns are not covered by
widget view groups.  Active status windows are drawn after menus,
which allows a one-line status window to occupy unused space on a
menu/status row.  If `UC_POPUPS` is enabled, application popups are
drawn after status windows, followed by the built-in too-small
popup when the screen is below its configured minimum size.

Once the entire new state of the display has been drawn into the
screen state buffers they are blasted out to the escape sequence
buffer by calling the `outer_update()` function.

The final operation here is to write the entire terminal escape
sequence buffer out to the terminal by calling `uC_terminfo_flush()`.
If `SIGWINCH` became pending while the frame was being composed,
uCurses cancels active popups when `UC_POPUPS` is enabled, purges
the partially composed terminal buffer, and leaves the pending
`WINCH` state for the application to handle with its normal resize
path.


## 6. The Window
The window structure is where application developers draw almost
everything they want displayed on the screen.  These are subsequently
drawn into the screen as described above.

A window has a configurable set of flags used to define how all text and
other items should be drawn within it and how the window should look and
behave.   The following flags are defined...

- `uC_WIN_BOXED`      The window has a border
- `uC_WIN_LOCKED`     The window is scroll locked
- `uC_WIN_NAMED`      The window has a display name
- `uC_WIN_FILLED`     The window is filled with its fill character
- `uC_WIN_FOCUS`      The window currently has input focus

Windows can have any dimensions and be located anywhere on the screen but
no part of any window can be situated outside the bounds of the screen.  A
window can be larger than a screen but in that case it cannot be attached
to it (made visible).

The window structure contains the following elements.

- An allocated array for its character contents
- A pointer to which ever screen it is attached to
- A 32 bit hash of its name
- An assigned blank / fill character which might not be a space
- Variables giving its dimensions
- Variables giving its location within its parent screen
- The windows tab selection order
- Variables giving the current cursor location within the window
- A variable giving the windows border type
- A pointer to the windows display name
- Variables giving various attributes for various window entities.

When a window is initially created it does not have position.  Prior
to attaching it to the screen it must be given position and care must be
taken to ensure that the above limitations are catered to.  A border for
example is not counted as part of a window so if there is a border around
a window then that windows lowest legal coordinates will be X = 1, Y = 1.

Using the public APIs to set a windows position ensures that these
requirements are not violated but uCurses does not have any code to
"automatically" lay out a display.  The addition of such algorithms would
add a significant amount of code to the library.


### 6.1. `uC_window_ctrl.c`
This source file contains some of the functions used to create, control
and manipulate windows.  There are also a few within the `uC_screen_ctrl.c`
source file because placing them here would require circular include
dependencies that I am not a huge fan of.

```c
int16_t win_alloc(uC_window_t *win)
```

This function allocates the backing store buffer for a window.  It
was factored out to allow the JSON parser access to its functions
and because it also gives an otherwise anonymous block a name.

```c
API uC_window_t *uC_win_open(int16_t width, int16_t height)
```

This public API call is where application developers create a new
window.  Every aspect of a window has a system wide default except
for its width, height and position which the application developer
can select.

Once a window has been created all of those aforementioned
defaults such as color attributes, border style, border attributes
etc can be overridden by the developer.

The code here first attempts to allocate a buffer for the window
structure itself and if successful it will then apply all those
defaults to it and call the above function to allocate its backing
store.

If all is successful the address of the window structure is
returned, allowing the application developer to perform said
overrides and attach it to a screen at any desired (but lawful)
position therein.

```c
API void uC_win_close(uC_window_t *win)
```

This public API function is used to destroy a window.  It will
de-allocate the windows backing store and the window structure
itself.

If the window being closed is currently attached to a screen then
it is first detached from that screen and thereby removed from its
draw order.   Closing a window does not automatically un-draw it.

```c
API void uC_win_push(uC_window_t *win)
```

This public API call is used to move a window to the start of its
parent screens draw order.  This will, on the next screen update,
cause that window to be drawn immediately after the backdrop.  It
could therefore be overlapped by other windows drawn into the
screen.

The related pop function was removed from the library because this
operation can be performed by simply reattaching a window to a
screen it is already attached to because this operation first
detaches the window before reattaching it.

Which ever window was most recently attached to the screen is
always drawn last!

```c
API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y)
```

This public API call is used to position a window within its
parent screen.  If that window is not currently associated with a
screen or if either of the specified coordinates are less than
zero then this function will return a failed result.

If after the requested placement of the window no part of that
window would be located outside the bounds of the associated
screen then the window is moved to the new location and this
function will return success.

```c
API void uC_win_set_flag(uC_window_t *win, win_flags_t flag)
```

A window structure contains a number of flags used to control its
appearance or behavior which this public API function will set.

The flags are as follows...

- `uC_WIN_BOXED`       The window has a border
- `uC_WIN_LOCKED`      The window is scroll locked
- `uC_WIN_FILLED`      The window is filled with some character
- `uC_WIN_FOCUS`       The window has focus.

The window having focus selects between one of two different border
attributes allowing a visual indication of its status.

The fill character can be any UTF-8 code point but defaults to the
space character (`0x20`).

```c
API void uC_win_clr_flag(uC_window_t *win, win_flags_t flag)
```

A window structure contains a number of flags used to control its
appearance or behavior which this public API function will clear.

The flags are as follows...

- `uC_WIN_BOXED`      The window has a border
- `uC_WIN_LOCKED`     The window is scroll locked
- `uC_WIN_FILLED`     The window is filled with some character
- `uC_WIN_FOCUS`      The window has focus.

The window having focus selects between one of two different border
attributes allowing a visual indication of its status.

```c
API void uC_win_set_border(uC_window_t *win, uint16_t border_type,
uC_attribs_t bdr_attrs, uC_attribs_t focus_attrs)
```

This public API call is used to give a window a border or to
change its border type.  It is also used to set the borders
attributes (focused and non focused).

```c
API void uC_win_set_name(uC_window_t *win, const char *name)
```

This public API call is used to change the display name of a
specified window.   If that window has a border then this name
will be written into the top left corner of that border when the
window is drawn.

```c
API void uC_win_set_focus(uC_window_t *win)
```

This public API call is used to give a specified window focus.
The tab selection order for the screen is also updated.


### 6.2. `uC_window_attr.c`
This source file contains public API functions to both set and clear the
various attributes that can be associated with a window or its borders.

These functions were all factored out of their original source file into a
separate one because they are all functionally related to each other and
doing so helped reduce the size of that original file making it easier to
manage.  No "Wall of text crits you for 50000! You're dead!"

```c
API void uC_win_set_bold(uC_window_t *win)
```

This public API call turns the bold attribute on for the specified
window.  Any characters subsequently written out to the window
will be drawn in bold.

```c
API void uC_win_clr_bold(uC_window_t *win)
```

This public API call turns the bold attribute off for the
specified window.  Any characters subsequently written out to the
window will not be drawn in bold.

```c
API void uC_win_set_bdr_bold(uC_window_t *win)
```

This public API call turns the bold attribute on for the specified
windows border characters.  The next time this window is drawn
into the screen its border characters will be drawn in bold.

```c
API void uC_win_clr_bdr_bold(uC_window_t *win)
```

This public API call turns the bold attribute off for the
specified windows border characters.  The next time this window is
drawn into the screen its border characters will not be drawn in
bold.

```c
API void uC_win_set_rev(uC_window_t *win)
```

This public API call turns the reverse video attribute on for the
specified window.  Any characters subsequently written out to the
window will be drawn in reverse video.

```c
API void uC_win_clr_rev(uC_window_t *win)
```

This public API call turns the reverse video attribute off for the
specified window.  Any characters subsequently written out to the
window will not be drawn in reverse video.

```c
API void uC_win_set_bdr_rev(uC_window_t *win)
```

This public API call turns the reverse video attribute on for the
specified windows border characters.  The next time this window is
drawn into the screen its border characters will be drawn in
reverse video.

```c
API void uC_win_clr_bdr_rev(uC_window_t *win)
```

This public API call turns the reverse video attribute off for the
specified windows border characters.  The next time this window is
drawn into the screen its border characters will not be drawn in
reverse video.

```c
API void uC_win_set_ul(uC_window_t *win)
```

This public API call turns the underline attribute on for the
specified window.  Any characters subsequently written out to the
window will be drawn with an underline.

This attribute is not supported for window borders because...
reasons, and don't be silly!

```c
API void uC_win_clr_ul(uC_window_t *win)
```

This public API call turns the underline attribute off for the
specified window.  Any characters subsequently written out to the
window will not be drawn with an underline.

```c
API void uC_win_set_fg(uC_window_t *win, uC_color_t color)
```

This public API call sets the foreground color for the specified
window.  Any characters subsequently written out to the window
will be drawn using this foreground color.

```c
API void uC_win_set_bg(uC_window_t *win, uC_color_t color)
```

This public API call sets the background color for the specified
window.  Any characters subsequently written out to the window
will be drawn using this background color.

```c
API void uC_win_set_gray_fg(uC_window_t *win, uC_colors_gray_t color)
```

This public API call sets the foreground color for the specified
window to a gray scale value.  Any characters subsequently written
out to the window will be drawn using this gray scale value for
its foreground color.

```c
API void uC_win_set_gray_bg(uC_window_t *win, uC_colors_gray_t color)
```

This public API call sets the background color for the specified
window to a gray scale value.  Any characters subsequently written
out to the window will be drawn using this gray scale value for
its background color.

```c
API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the foreground color for the specified
window to a 24 bit RGB color.  Any characters subsequently written
out to the window will be drawn using this RGB value for its
foreground color.

```c
API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the background color for the specified
window to a 24 bit RGB color.  Any characters subsequently written
out to the window will be drawn using this RGB value for its
background color.

```c
API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color)
```

This public API call sets the foreground color for the specified
windows border.  The next time this window is drawn its border
characters foreground will be drawn using this color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color)
```

This public API call sets the background color for the specified
windows border.  The next time this window is drawn its border
characters background will be drawn in this color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_bdr_gray_fg(uC_window_t *win,
uC_colors_gray_t color)
```

This public API call sets the foreground color for the specified
windows border to a gray scale value.  The next time this window
is drawn its border characters foreground will be drawn in this
gray scale color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_bdr_gray_bg(uC_window_t *win,
uC_colors_gray_t color)
```

This public API call sets the background color for the specified
windows border to a gray scale value.  The next time this window
is drawn its border characters background will be drawn in this
gray scale color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the foreground color for the specified
windows border to a 24 bit RGB value.  The next time this window
is drawn its border characters foreground will be drawn in this
color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the background color for the specified
windows border to a 24 bit RGB value.  The next time this window
is drawn its border characters background will be drawn in this
color.

These attributes only apply when said window does not have focus.

```c
API void uC_win_set_focus_fg(uC_window_t *win, uC_color_t color)
```

This public API call sets the foreground color for the specified
windows focused border.  The next time this window is drawn while
it has focus its border characters foreground will be drawn in
this color.

```c
API void uC_win_set_focus_bg(uC_window_t *win, uC_color_t color)
```

This public API call sets the background color for the specified
windows focused border.  The next time this window is drawn while
it has focus its border characters background will be drawn in
this color.

```c
API void uC_win_set_focus_gray_fg(uC_window_t *win,
uC_colors_gray_t color)
```

This public API call sets the foreground color for the specified
windows focused border to a gray scale value.  The next time this
window is drawn while it has focus its border characters
foreground will be drawn in this gray scale.

```c
API void uC_win_set_focus_gray_bg(uC_window_t *win,
uC_colors_gray_t color)
```

This public API call sets the background color for the specified
windows focused border to a gray scale value.  The next time this
window is drawn while it has focus its border characters
background will be drawn in this gray scale.

```c
API void uC_win_set_focus_rgb_fg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the foreground color for the specified
windows focused border to a 24 bit RGB color.  The next time this
window is drawn while it has focus its border characters
foreground will be drawn in this color.

```c
API void uC_win_set_focus_rgb_bg(uC_window_t *win, uC_color_t r,
uC_color_t g, uC_color_t b)
```

This public API call sets the background color for the specified
windows focused border to a 24 bit RGB color.  The next time this
window is drawn while it has focus its border characters
background will be drawn in this color.


### 6.3. `uC_border.c`
Window borders are drawn directly into a screen around a window.  They are
not counted as being physically part of that windows draw field.  This
means that application developers can, after setting border style and
attributes, ignore them entirely.  Of course, they must still ensure that
no part of the border would be drawn outside the bounds of the screen or
bad things will happen.

This file contains all the code used to draw borders around windows.  It
also contains functions to draw boxes within windows.  Boxes are simply a
way to draw a bounding box around areas within a window, they are not sub
windows.

Borders are drawn one row at a time.  First the top row is drawn, then
each row between the top and bottom row, then, finally the bottom row of
the border is drawn.

The following arrays contain the UTF-8 code points for each border
character.  They also contain the characters which can be used to draw
joined vertical or horizontal separators within windows.

```c
border_t bdr_single[]    ┏┓┗┛━┃┣┫┳┻╋
border_t bdr_double[]    ╔╗╚╝═║╠╣╦╩╬
border_t bdr_curved[]    ╭╮╰╯─│├┤┬┴┼
```

Horizontal and vertical separators have not been implemented for
window borders and probably never will be because borders are
outside the windows draw field.

The intent is to eventually add the code to draw them within boxes
which are described below.  While this is on the todo and would be
quite trivial to implement I am somewhat reluctant to add the code
to the library because it is already significantly larger than I
ever wanted it to be.

```c
border_t *const borders[]
```

This array holds pointers to each of the above arrays.  The order
in here is per the `uC_border_type_t` enumeration.

```c
static void draw_char(uC_window_t *win, int16_t cx, int16_t cy,
int32_t code, int16_t force)
```

This function draws one single UTF-8 border character directly
into the screen state buffer.  It first selects which attributes
should be applied when drawing this character based on whether or
not the window has focus.  It then draws the character itself.

Care is optionally taken here to ensure that visual glitches do
not occur when the drawn character overlaps one or other side of a
multi width character.

```c
static void draw_top_bottom(uC_window_t *win, int32_t c1, int32_t c2,
int32_t c3, int16_t cy)
```

This function is used to draw both the top and bottom rows of a
border.  It has three parameters passed to it for each of the
three characters associated with that line.  The left corner
character, the right corner character and the central characters.

The left and right characters must account for potential visual
glitches where they may overlap multi width characters already
present in the screen.  The central characters within the line do
not need to worry about this at all.

```c
static void draw_mid_row(uC_window_t *win, int32_t c1, int32_t c3,
int16_t cy)
```

This function draws every horizontal row of the window between the
top and bottom rows.  The vertical characters at the left and
right edges are drawn directly into the screen but none of the
central characters of the row are touched.  This is where the
windows actual contents is (or will be) drawn.

```c
void win_draw_borders(uC_window_t *win)
```

This function draws a complete border around a window if, and only
if that window has been given a border.

```c
API void uC_win_draw_box(uC_window_t *win, uint16_t x, uint16_t y,
uint16_t width, uint16_t height, uC_border_type_t bdr_type,
uC_attribs_t box_attrs)
```

This public API function draws a box of a specified dimension
within a window.  The width, height and location of this box
within the window are passed in as parameters.  It is undefined
behavior for a box to not fit entirely within the specified
window.  The border type and attributes are also passed in.

There is no concept of one item overlapping another inside a
window so there is no need for these functions to account for
potential visual glitches.  Boxes also never have 'focus' so only
one set of attributes are applicable.


### 6.4. `uC_window_draw.c`
This source file contains all functions that read or write a window's
cell buffer directly.  This includes cursor movement, character emission,
scrolling, panning, and buffer copy.

```c
API void uC_win_clear_line(uC_window_t *win, int16_t line)
```

Clears one line of a window using the window's current attributes
and blank character.

```c
API void uC_win_clear(uC_window_t *win)
```

Clears all lines of a window and homes the cursor.

```c
API void uC_win_copy_win(uC_window_t *dst, uC_window_t *src)
```

Copies the entire cell buffer of src into dst.  Both windows must
have the same dimensions.  This is used to implement layered
rendering: write into a backing (non-displayed) window, then copy
to the visible window and stamp any fixed overlays on top.
The scroll demo (`example/scroll.c`) demonstrates this pattern.

```c
API void uC_win_scroll_up(uC_window_t *win)
API void uC_win_scroll_dn(uC_window_t *win)
API void uC_win_scroll_lt(uC_window_t *win)
API void uC_win_scroll_rt(uC_window_t *win)
```

Scroll or pan a window by one line or column.  The newly exposed
row or column is cleared to the window's current blank character
and attributes.  These are thin wrappers around the _n variants
below.  The %up %dn %lt %rt win_printf specifiers call these
internally.

```c
API void uC_win_scroll_up_n(uC_window_t *win, int16_t n)
API void uC_win_scroll_dn_n(uC_window_t *win, int16_t n)
API void uC_win_scroll_lt_n(uC_window_t *win, int16_t n)
API void uC_win_scroll_rt_n(uC_window_t *win, int16_t n)
```

Scroll or pan a window by n lines or columns.  Invalid values of
n are silently clamped: n <= 0 is a no-op, n >= height (or width
for left/right) clears the entire window.  These are the
preferred variants when moving more than one line at a time.

```c
API void uC_win_cup(uC_window_t *win, int16_t x, int16_t y)
```

Move the window cursor to the specified (x, y) position.  Out
of bounds coordinates are silently ignored.

```c
API void uC_win_emit(uC_window_t *win, uint32_t c)
```

Write one UTF-8 code point into the window at the current cursor
position and advance the cursor.  CR and LF move to the start of
the next line.  Tabs are rejected.  Wide characters (e.g. CJK)
consume the correct number of columns.

```c
API cell_t *uC_win_peek_xy(uC_window_t *win, int16_t x, int16_t y)
API cell_t *uC_win_peek(uC_window_t *win)
```

Return a pointer to the cell at the specified position or the
current cursor position.  Returns `NULL` if out of bounds.

```c
API void uC_win_el(uC_window_t *win)
```

Erase from the current cursor position to the end of the current
line.


## 7. Status windows
A status window is a one line high window that can be placed at any
location on the screen but which is usually placed either on the top line
of the screen within a menu bar, or on the bottom line of the screen,
possibly within the border of any backdrop window.

A status window can be used to display any information an application
developer can think of.  I use them as a way to display debug information
while developing uCurses.

A screen can have any number of status windows associated with it.

Status windows are also a build option so they can be excluded entirely
from the uCurses build if they are not needed.  Removing them from the
build will save a small amount of space on your target.


### 7.1. `uC_status.c`
```c
API uC_window_t *uC_add_status(uC_screen_t *scr, uint16_t width,
uint16_t xco, uint16_t yco)
```

This public API call opens a new one line high window of the
specified width and locates said window at the specified screen
coordinates if and only if said coordinates do not violate any of
the uCurses window placement restrictions.

On success this function returns the address of the status windows
structure.  On failure it returns `NULL`.

```c
API void uC_clr_status(uC_window_t *win)
```

This public API call will simply clear the specified status window
for later drawing into.

```c
API void uC_set_status(uC_window_t *win, const char *fmt, ...)
```

This public API call is used to write formatted text into the
specified status window.  It accepts a printf-style format string
and any associated arguments, formatting them internally via
`uC_win_vprintf()`.


## 8. Menus
Pull down menus in uCurses are very simple.  The root of the menu system
is the menu bar.  While you could have more than one defined, only one of
them can be active and on a screen.

The menu bar structure contains the following elements.

- A window which contains the menu bar contents.
- An X coord within the window where the next pull down item will go.
- A variable stating whether the menu bar is active or not.
- A variable giving the index of which ever item within it is active.
- A count of the total number of pulldown items associated with it.
- An array of currently associated pulldown items.
- Various attributes which menu elements will be drawn with.

A menu bar is a container for a number of pull down menu items.  The only
limitation on how many items there can be is how many you can fit in the
horizontal space available within the bar.  The bar itself is a one line
high window equal in width to the screen.  Each pull down item has a name
and that name is displayed in the bar.

The structure of a pulldown item contains the following elements...

- A name which is displayed in the menu bar.
- A variable giving the width of the widest item name.
- A variable giving the index of the currently selected item.
- A flags variable (only one flag so far defined)
- A list of menu items associated with the pulldown.
- A pointer to a window which is created when this pulldown is active.
- Various attributes which define how elements within it are drawn.

Each pull down menu can have a number of menu items but these are not
displayed unless their parent pull down is activated.  When activated a
window is opened whose width is equal to the widest menu item and whose
height is equal to the item count within that pull down.  There is no
concept of a scrollable pull down menu.

A pulldown menu item structure has the following elements...

- A pointer to the menu items name
- A flags variable (only one defined so far)
- A pointer to a function to be executed on selection of this item.
- A screen-level shortcut associated with this item.
- A pointer to the screen that owns the registered shortcut.
- A pointer to the parent pulldown menu.
- A flag recording whether the shortcut is currently registered.

The text drawn into a menu bar can be drawn in one of three color
attributes, active, inactive or disabled.

When a pull down menu is activated a window is created and the name of
each item within that pull down is drawn into that window.  As with the
menu bar, each menu item can be drawn in one of three color attributes,
active, inactive or disabled.

If a menu item is created with a non-zero shortcut, that shortcut is
registered against the screen.  Registered menu shortcuts are active
whether the menu is open or closed.  The menu item still stores the
shortcut value so the pull down can display it; display and execution are
therefore both driven by the same item definition.

Shortcut collisions are rejected by the screen registry.  If a duplicate
shortcut is supplied for an enabled menu item, menu item creation fails
and returns -1.  Plain letter shortcuts are matched case-insensitively.
Control-key shortcuts can be registered.  Alt-key and Meta-key shortcuts
can be represented and displayed but are not matched until the keyboard
reader grows stateful escape handling for those modifier sequences.

The menu system itself is in control of which item is currently selected
but the application developer must enable or disable pull down menus and
menu items manually based on application context.  The menu system does
not infer this context; it only applies the enable or disable operations
requested by the application.

Disabling a pulldown menu removes all registered shortcuts owned by its
items.  Re-enabling the pulldown attempts to register those shortcuts
again.  Disabling an individual menu item removes only that items
shortcut, and re-enabling the item attempts to register it again.  If a
shortcut is already owned by another menu item or widget when an enable
call is made, that item remains selectable from the menu but does not get
the global shortcut until the conflict is removed and the item is enabled
again.

Menu navigation skips disabled pulldowns and disabled menu items.  If a
currently selected but closed menu item is disabled, the next menu
activation normalizes selection to an enabled item before drawing.  If a
currently active pulldown or item is disabled, uCurses normalizes the
active selection immediately.  Enter will not execute disabled items.

The menu system maintains three separate color attributes for active,
inactive and disabled menu items.  The values for each of these color
attributes is hard coded into the menu system and assigned as each entity
(pull down or menu item) is created.  If these wonderful colors are not to
your liking you can always modify them.

The above "only one flag defined so far" is used to disable and re-enable
a pulldown menu or a pulldown menu item.


### 8.1. `uC_menu_bar.c`
```c
static void pd_set_attr(int16_t i, pulldown_t *pd, uC_attribs_t *p,
menu_item_t *item)
```

This function is used when drawing the names of the menu items
within a pull down menu window.  It selects which of the three
attributes should be used to draw the current item based on
whether the item is selected, not selected or disabled.

```c
void draw_pd(pulldown_t *pd)
```

This function is used to draw all menu items into a pull down
menus window.  Each item is drawn in one of the above three
mentioned color attributes.

```c
static void bar_set_attrs(int16_t i, menu_bar_t *bar, uC_attribs_t *p,
pulldown_t *pd)
```

This function performs a similar function to the above pd_set_attr
function but is used to set the attributes for the menu bar items.
It is however so similar I am confused as to why there are two
functions!

```c
static void bar_draw_text(uC_screen_t *scr)
```

This function is used to draw the names of each pull down menu
item into the menu bar window.  The attributes for each item are
set by the above function.

```c
static void pd_close(pulldown_t *pd)
```

This function is part of the pull down menu tear down procedure.
It de-allocates each menu item structure within a single pull down
menu, closes the pull down menus window and then de-allocates that
pull down menu structure.

```c
static void bar_close_pds(menu_bar_t *bar)
```

This function is also part of the pull down menus tear down
procedure.  It closes (de-allocates) all pull down menus and all
menu items within each of these.

```c
API void uC_menu_bar_close(uC_screen_t *scr)
```

This public API call is the application developers interface to
destroying a pull down menu.  It will close and de-allocate any
menu bar associated with the specified screen.  It calls the above
functions to de-allocate all pull down menus and all menu items
attached to the bar.

It then closes the menu bars window and de-allocates the menu bar
structure itself.

This can be used to activate and deactivate various menu bars
based on application context.  For example, it might be possible
for an application to activate different menus based on which
window within it has focus.

```c
static void init_bar(uC_screen_t *scr, uC_window_t *win,
menu_bar_t *bar)
```

This function initializes a newly opened menu bar structure and
its associated window.  It scroll locks the window and initializes
its default blank character.  It then initializes the menu bars
attributes to the 'semi' hard coded defaults and attaches that
menu bar to the specified screen.

```c
API int32_t uC_menu_bar_open(uC_screen_t *scr)
```

This public API function is how application developers open a new
menu bar.  The bar is allocated, its window is opened and the bar
is initialized.

If any part of this procedure fails all allocated entities are
freed and a result of -1 is returned.  If no problems were
encountered a result of 0 is returned.

```c
void scr_update_menus(uC_screen_t *scr)
```

This function is part of the menu control loop.  It updates the
display of any active pull down menus.  It is called by the screen
update loop in `uC_screen_draw.c`.


### 8.2. `uC_menu_pulldown.c`
```c
static pulldown_t *pd_find(uC_screen_t *scr, char *name)
```

This helper function is used to find a specific pull down menu
item based on its name.  It is used by the following functions
which either enable or disable a pull down menu.

```c
API void uC_menu_pd_disable(uC_screen_t *scr, const char *name)
```

This public API function will search the specified screens pull
down menus for a pull down with the specified name.  If this item
is found it will be marked disabled.  Once disabled the pull down
menu cannot be activated or even selected until it is re-enabled.
Registered shortcuts owned by its menu items are removed and the
current menu selection is normalized.

```c
API void uC_menu_pd_enable(uC_screen_t *scr, const char *name)
```

This public API function will search the specified screens pull
down menus for a pull down with the specified name.  If this item
is found it will be marked as enabled.  Once enabled it can be
activated when selected.  Shortcuts owned by enabled items in
that pulldown are registered again if they do not conflict with an
existing shortcut.

```c
API void uC_menu_item_disable(uC_screen_t *scr, const char *pd_name,
const char *item_name)
```

This public API function finds a named menu item within a named
pulldown and marks that item disabled.  Its registered shortcut is
removed and menu selection is normalized so the disabled item is
not a selectable target.

```c
API void uC_menu_item_enable(uC_screen_t *scr, const char *pd_name,
const char *item_name)
```

This public API function finds a named menu item within a named
pulldown and marks that item enabled.  If the item has a shortcut
and its parent pulldown is enabled, uCurses attempts to register
the shortcut again.

```c
int32_t bar_create_pd_win(uC_screen_t *scr, pulldown_t *pd)
```

This function opens a new window for an activated pull down menu.
The window is created with a width equal to the widest item within
the pull down menu and a height equal the item count for that
menu.

The window is then given a single line border with rounded corners
and is marked as scroll locked.  The foreground and background
colors for this border are also set here.

```c
API int32_t uC_menu_new_pd(uC_screen_t *scr, char *name)
```

This public API will allocate and initialize a new pull down menu
structure with the specified name and attach it to the menu bar
associated with the specified screen.

If the allocation fails it will return a result of -1.  If it is
successful a result of 0 is returned.


### 8.3. `uC_menu_item.c`
```c
static void init_item(uC_screen_t *scr, pulldown_t *pd,
menu_item_t *item, const char *name, menu_fp_t fp,
uC_shortcut_t shortcut)
```

This function is used to initialize a new menu item structure.  It
adds the item to a pull down structure, sets the display name,
function vector address, screen pointer and keyboard shortcut.

It also keeps track of the widest item label and widest displayed
shortcut added to the pull down.  These are used to set the width
of its window when it is activated.  Shortcut display text is
aligned one blank column after the widest item label.

```c
static int32_t new_item(uC_screen_t *scr, pulldown_t *pd,
const char *name, menu_fp_t fp, uC_shortcut_t shortcut)
```

This function is part of the following function to create a new
menu item.  This function will allocate and initialize a new menu
item structure unless the pull down menu it is to be associated
with is full or the requested shortcut cannot be registered.

On success it returns a result of 0, otherwise it returns a result
of -1.

```c
API int32_t uC_menu_new_item(uC_screen_t *scr, const char *name,
menu_fp_t fp, uC_shortcut_t shortcut)
```

This public API function will create a new menu item and add it to
the bottom of the most recently created pull down menu.

It first verifies that the specified screen is not null and that
it has a non null menu bar.  It then verifies that there is a
pulldown menu on the bar to add the item to.

If shortcut is non-zero it is registered as a screen-level
shortcut before the item is attached to the pull down.  A duplicate
registered shortcut causes this function to fail cleanly.

On success this function will return a result of zero.  On failure
it will return a result of -1.


### 8.4. `uC_menu_key.c`
```c
static void redraw_pulldown(menu_bar_t *bar)
```

This function will redraw the currently active pull down menu
after a menu key has been pressed.

```c
static void menu_activate(void)
```

This function will toggle the active state of the currently
selected pull down menu.  If the pull down menu is being activated
it will be drawn into the screens state buffer.  Otherwise any
currently active pull down will have its window closed.

```c
API bool uC_menu_is_active(uC_screen_t *scr)
```

Returns true when the supplied screen has a menu bar and that menu
bar currently has an open pull down.

```c
API void uC_menu_open(uC_screen_t *scr)
```

Programmatically opens the menu bar on the supplied screen.  This
is useful for applications that want to keep `F10` or another key in
their own event loop and explicitly decide when menu navigation
should take over.

```c
API void uC_menu_close(uC_screen_t *scr)
```

Programmatically closes the active pull down menu, if any.

```c
static void to_prev_menu_item(pulldown_t *pd)
```

This function will deselect the current pull down menu item and
select the one immediately above it.  If the currently selected
item is the top item then selection wraps round to the bottom of
the pull down menu.

```c
static void to_next_menu_item(pulldown_t *pd)
```

This function will deselect the current pull down menu item and
select the one immediately below it.  If the currently selected
item is the bottom item then selection wraps round to the top of
the pull down menu.

```c
static void next_pd(menu_bar_t *bar)
```

This function deselects the currently selected pull down menu and
selects the one immediately to its right.  If the currently
selected pull down is the one furthest to the right then selection
wraps around to the first pull down on the left.

```c
static void prev_pd(menu_bar_t *bar)
```

This function deselects the currently selected pull down menu and
selects the one immediately to its left.  If the currently
selected pull down is the one furthest to the left then the
selection wraps around to the last one on the right.

```c
static void menu_up_down(int dir)
```

This function will move the pull down menu item selection either
up or down within the pull down menus window.  If the item to be
selected is disabled it is skipped.  A check is made to ensure
that this procedure does not perform an infinite loop.

Only Chuck Norris could do those:  RIP!

```c
static void menu_left_rt(int dir)
```

This function will move the pull down menu selection either left
or right on the menu bar.  If the item to be selected is disabled
it is skipped.  A check is made to ensure that this procedure does
not perform an infinite loop.

```c
static void menu_cr(void)
```

This function will close the currently open pull down menu and
execute the currently selected items function vector.  Disabled
pulldowns and disabled items are ignored.

If the selected function stuffs an application key with
`uC_set_key()`, the caller can retrieve that synthetic key after the
selection has completed.

```c
static void menu_up(void)     static void menu_down(void)
static void menu_left(void)   static void menu_right(void)
```

These four functions are the keyboard handlers for the cursor keys
when a pull down menu is activated.

```c
bool menu_key(uC_screen_t *scr, uint8_t key, uint8_t *out)
```

Internal dispatch entry used by `uC_key()`.  `F10` opens or closes the
menu.  When a pull down is active, Escape closes it, cursor keys
navigate it and Enter selects the current item.  Keys that are not
menu navigation are left for the next layer of the consolidated
dispatcher.

Menu item accelerator shortcuts are not scanned here.  Those are
handled by the screen shortcut registry before menu navigation is
considered.

```c
API uint8_t uC_menu_run(uC_screen_t *scr)
```

Runs the active menu until the pull down is closed or a selected
menu item executes its function vector.  This is the preferred API
for applications that want the menu system to own Up, Down, Left,
Right, Enter and `F10` behavior while a menu is active.

Menu item functions may stuff an application key with `uC_set_key()`.
When that happens `uC_menu_run` returns that key to the application
after the menu item has been executed.  If the menu is closed
without selecting an item, it returns zero.

```c
API void uC_menu_cursor_up(uC_screen_t *scr)
API void uC_menu_cursor_down(uC_screen_t *scr)
API void uC_menu_cursor_left(uC_screen_t *scr)
API void uC_menu_cursor_right(uC_screen_t *scr)
```

Programmatic wrappers around the menu cursor handlers.  They are
intended for applications that read keys themselves and then route
synthetic or decoded cursor-key events into the active menu.

```c
API void uC_menu_select(uC_screen_t *scr)
```

Programmatically selects the current menu item, closes the active
pull down, and executes the selected item's function vector.

```c
API void uC_menu_init_keys(void)
```

This public API function initializes the keyboard behavior needed
by the pull down menu system.  It emits smkx through terminfo so
cursor keys are reported in the mode the terminfo table expects.
It also installs the default `F10` translation used by `uC_key()` when
menus are built in.


## 9. Widgets
Widgets were a surprisingly complex problem for me to solve.  I wrote the
code for them numerous times over the course of a few years and I was
never even slightly happy with what I had produced.  This time, I think I
can live with it because while there is room for improvement, the
implementation does not just make my teeth itch looking at it.  It will do
for now!

uCurses does not support every possible type of widget that you might find
in a more complete user interface.  For example, there is currently no
implementation for slidy scrolly types of widget and widget views can
only scroll in the vertical direction.

uCurses currently supports the following types of widget...

- `uC_WIDGET_BUTTON`
- `uC_WIDGET_RADIO`
- `uC_WIDGET_CHECK`
- `uC_WIDGET_TEXTBOX`

Both the radio button and the check box widgets can have one of the
following styles...

```text
uC_RADIO_CHECKBOX        ☐  ☑
uC_RADIO_XBOX            ☐  ☒
uC_RADIO_BOX             □  ▣    There are also numerous other
uC_RADIO_BIGBOX          □  ■    styles which could be added
uC_RADIO_SMALLBOX        ▫  ▪    here
uC_RADIO_DIAMOND         ◇  ◈
uC_RADIO_UP_TRIANGLE     △  ▲
uC_RADIO_DN_TRIANGLE     ▽  ▼
uC_RADIO_LT_TRIANGLE     ◁  ◀
uC_RADIO_RT_TRIANGLE     ▷  ▶
uC_RADIO_UP_SMALL        ▵  ▴
uC_RADIO_DN_SMALL        ▿  ▾
uC_RADIO_LT_SMALL        ◃  ◂
uC_RADIO_RT_SMALL        ▹  ▸
```

In uCurses all widgets must be part of a view and all views must be
contained within a view group.  A view may only contain one type of
widget.  The view structure contains the following entities...

- A flags variable
- A list of widgets associated with this view
- A pointer to the currently selected widgets node structure
- A name string
- Variables giving the dimensions of the view.
- Variables giving this views location in its parent window.
- A tab sequence number for this entire view if it is scrollable.
- An index to the top item in this view if it is scrollable
- The current cursor index within this view.
- Various attributes for entities drawn within this view.

The following flags are defined for widget views...

- `uC_VIEW_BOXED`      The view has a border
- `uC_VIEW_NAMED`      The view has a name to be displayed in its border
- `uC_VIEW_SCROLL`     The view is scrollable

A view group is a window just like any other.  It is used as a container
for associated widget views.  It can contain any number of views of any
type.  A view group structure contains the following elements.

- A pointer to its parent window.
- A list of views associated with this group.
- A flags variable for this view. (only one defined)

If the view group window has a border then that border can be used as a
visual indication for when some widget within it has focus.  The view
group can also have a name displayed within its border allowing the
application developer to give some description of the groups
associativity.

A widget can only have focus if it is contained within the view that has
focus and where that view is contained within the view group that has
focus.

The application developer can assign *ANY* unique tab focus order on any
widgets within any views within any view groups on the display.  Sequence
values do not need to be contiguous; sparse ranges are legal and useful
when a modal popup, tool panel, or view group should have an obvious
sequence range of its own.  Keeping values modest and human-readable is
still recommended for maintainability.

If however, a view is defined as scrollable (can contain more widgets than
will fit in the vertical space assigned to it) then the tab selection
order of the first widget within that view is controlling.  The view
itself controls which widget within it actually has focus.

The focus selection of all other widgets is controlled by the widget
engine based on the developer defined order.


### 9.1. `uC_widget.c`
```c
uC_widget_t *create_widget(uC_widget_type_t type, const char *name,
uint16_t width, uC_attribs_t attrs, uC_attribs_t focus)
```

This function is used to create a new widget.  The widget can be
one of four types as follows...

- `uC_WIDGET_BUTTON`
- `uC_WIDGET_RADIO`
- `uC_WIDGET_CHECK`
- `uC_WIDGET_TEXTBOX`

The name is what would be drawn on the widget itself or in the
case of a textbox to the left of the edit area.  The widget can be
assigned both a normal set of attributes and a focused set of
attributes.

A widget's tab sequence is assigned when the widget is added to a
view with `uC_widget_view_add_widget()`.  Its position is assigned
separately with `uC_widget_set_position()`.

```c
API void uC_widget_set_position(uC_widget_t *widget,
uint16_t xco, uint16_t yco)
```

This public API call positions a widget relative to its parent
view.  It may be called before or after the widget is added to a
view, but the widget must still fit within the view when drawn.

```c
API void uC_widget_close_widget(uC_widget_t *widget)
```

This public API call can be used to close any widget at any time.
The widget is first detached from its parent view if it has one,
any screen-level shortcut owned by that widget is deregistered,
and focus is cleared if that widget currently owns focus.  The
widget is then destroyed.


### 9.2. `uC_widget_view.c`
```c
API uC_widget_view_t *uC_widget_view_create(const char *name,
uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
uC_attribs_t attrs, bool scroll)
```

This public API call is the application developers means of
creating a new widget view.  A widget view is a container for a
group of related widgets of the same type and may be defined as
scrollable but scrolling only happens in the vertical direction.

When creating a new view you can also give it an optional name but
this name will only be displayed if the view is later given a
border.

The coordinates passed to this function are relative to the view
group (window) which this view is to be associated with.

If this view is to be scrollable the width and height specified
here are the viewable dimensions of this view, not how many
widgets can be associated with it.

```c
API void uC_widget_view_add_border(uC_widget_view_t *view,
uC_border_type_t bdr_type, uC_attribs_t bdr_attrs)
```

This public API call is used to give an already created view a
border.  If the view has a name then adding this border will make
that name visible within the border area.

The border may be any of the border types that can be given to
windows.

```c
API bool uC_widget_view_add_widget(uC_widget_view_t *view,
uC_widget_t *widget, uint16_t sequence)
```

This public API call is used to attach a widget to a widget view.
Any number of widgets can be added to a view as long as they will
visually fit within the assigned view space and they are all of
the same type.  The sequence parameter is the widget's tab
selection order.  Sequence values must be unique among active
widgets, but they do not need to be contiguous.

If the view is scrollable the limit is 64k (insanity).

```c
API void uC_widget_view_remove_widget(uC_widget_view_t *view,
uC_widget_t *widget)
```

This public API call is used to detach a widget from a view.  The
view itself is not closed even if the removed widget is the only
one that was attached.  Removing a widget does not destroy it, but
it does deregister any screen-level shortcut owned by that widget
and clears focus if the removed widget currently owns focus.

```c
void widget_close_view(uC_widget_view_t *view)
```

This function is used during screen tear down and will destroy the
specified view.  If there are any widgets associated with this
view then they are also destroyed.

```c
static void view_up(void)
```

For a scrollable view this function will move the focus up one
line within the view and will scroll the view if necessary.
The selection will not wrap within the view if it is at the top
thereof.

```c
static void view_down(void)
```

For a scrollable view this function will move the focus down one
line within the view and will scroll the view if necessary. The
selection will not wrap within the view if it is at the bottom
thereof.

```c
void widget_scroll_view(uint8_t k)
```

This function is used by the widget keyboard handler to scroll a
view in either direction based on which key was pressed.


### 9.3. `uC_widget_view_group.c`
```c
API uC_widget_vg_t *uC_widget_vg_create(const char *name,
uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
uC_attribs_t attrs)
```

This public API call is used to create a new view group.  A view
group is a container for any number of widget views that will fit
within it.   Creating a view group also creates a new window where
any view attached to the group will be displayed.

The view group can also have a name which will be displayed within
the windows border if one is given to it later.

This window can also have any text drawn into it if the
application developer desires.   To do this the developer would
simply extract the window structures address from the view group
structure and use the normal uCurses API calls to draw text into
it.

```c
API void uC_widget_vg_add_border(uC_widget_vg_t *vg,
uC_border_type_t bdr_type, uC_attribs_t bdr_attrs,
uC_attribs_t focus_attrs)
```

This public API call is used to give a border to a view groups
window.  The developer can assign both focused and unfocused
attributes to this border.

```c
API void uC_widget_vg_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
```

This public API call is used to attach a view group to a specified
screen structure.  Once the view group is attached, all widgets
within all widget views associated with the attached group will be
displayed on the next update.

A view group follows the same placement rule as a normal window:
it is attached only if the complete view-group window, including
any border, fits within the target screen at the requested
coordinates.  If it does not fit, this call quietly leaves the
group detached.

Button widgets with assigned shortcut letters are registered with
the screen-level shortcut registry at this point.  If another menu
item or widget already owns that shortcut on the same screen the
duplicate registration is ignored; the button still displays its
assigned letter and still works through focus and Enter.
If no widget currently has focus when this call is made then the
widget system attempts to give focus to the first available tab
stop.  Applications may also explicitly select a widget sequence
after attaching the view group.

```c
API void uC_widget_vg_detach(uC_screen_t *scr, uC_widget_vg_t *vg)
```

This public API call is used to detach a specified view group from
a screen structure.  The detached view group is not destroyed but
will no longer be drawn into the screen.  Detach also removes
button shortcut registrations owned by widgets in the group and
clears focus if the detached group owned the focused widget.  The
view group, its views, its widgets, and its backing window buffer
remain allocated so the group can be attached again later.

```c
API void uC_widget_vg_close(uC_widget_vg_t *vg)
```

This public API call closes a view group, destroying all views and
all widgets associated with them.  The view group is detached from
its parent screen and then destroyed.

```c
bool widget_vg_contains_widget(uC_widget_vg_t *vg,
uC_widget_t *target)
```

Internal helper that scans all views in a view group and reports
whether the specified widget belongs to that group.  Widget popup
shortcut dispatch uses this to allow only shortcuts owned by
widgets in the active popup view group while a modal popup is
attached.

```c
API bool uC_widget_popup_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
```

This public API call attaches a view group as the screen's modal
widget popup.  Normal widget view groups on the screen are marked
inactive while the popup is attached and their previous inactive
state is restored when the popup is detached.  The popup itself is
drawn in the popup phase so it appears above normal widgets and
windows.

The first available widget tab stop in the popup is selected during
attach.  Applications may call `uC_widget_select_widget()` or move a
scrollable view cursor afterward when they need to restore a
specific popup row.

```c
API void uC_widget_popup_detach(uC_widget_vg_t *vg)
```

This public API call detaches a modal widget popup without
destroying the view group, its views, its widgets, or its backing
window buffer.

```c
API bool uC_widget_vg_add_view(uC_widget_vg_t *vg,
uC_widget_view_t *v, uint16_t sequence)
```

This public API call is used to associate a specified view with a
view group.  It returns true when the view was attached and false
when the view or group is null, the view does not fit within the
view-group window, or the list insertion fails.

A boxed view must have at least one cell of offset from the top and
left edges of the view-group window so its border can be drawn
inside the group.  Boxed and unboxed views must fit completely
within the view group.  Oversized views are ignored rather than
being added and later scribbling over unrelated content.


### 9.4. `uC_widget_draw.c`
This file contains functions to draw all widgets within all views within
all view groups associated with a specified screen.  While the individual
widget types are actually drawn within their respective source files (see
below) the calls to those functions all happen in here.

This code will draw all views and all widgets within those views into the
window associated with each view group attached to the specified screen.
The window is then itself drawn into that screen state buffers.

For each view the code will first draw any borders around it if there is
one and within that border will draw its name if there is one.

If the view is not defined as being scrollable then all widgets within it
are then drawn into body of the bounding box drawn above.  If it is
scrollable then the widgets which are drawn into this area will depend on
the scroll position within the view.

If the widgets being drawn are either checkbox or radio buttons then the
graphic for each of these can be any of the following vertical pairs for
their on and off states.

```text
Off: ☐ ☐ □ □ ▫ ◇ △ ▽ ◁ ▷ ▵ ▿ ◃ ▹
On:  ☑ ☒ ▣ ■ ▪ ◈ ▲ ▼ ◀ ▶ ▴ ▾ ◂ ▸
```

The following functions perform the entire drawing operation...

```c
static void draw_widget(uC_window_t *win, uC_widget_t *widget,
uint16_t xco, uint16_t yco)
```

This static function will draw the specified widget into the
specified window at the specified location.   The widget can be
any one of the following types.

- `uC_WIDGET_BUTTON`
- `uC_WIDGET_RADIO`
- `uC_WIDGET_CHECK`
- `uC_WIDGET_TEXTBOX`

```c
static void draw_view_box(uC_window_t *win, uC_widget_view_t *view)
```

This static function draws a box around the area within a view
groups window where a view is about to be drawn.  Not all views
need have a box but where one exists the views name if any will
then also be drawn here.

```c
static void draw_scrollable(uC_window_t *win, uC_widget_view_t *view)
```

This static function draws all visible widgets within a scrollable
view into the window of a view group.  Which widgets are visible
depend on the scroll position within the view.

```c
static void draw_nonscrollable(uC_window_t *win,
uC_widget_view_t *view)
```

This static function draws all widgets within a non scrollable
view into the window of a view group.  These widgets can be
positioned anywhere within the view area within the view group.

```c
static void draw_view(uC_window_t *win, uC_widget_view_t *view)
```

This static function draws all visible widgets within a specified
view within the specified view groups window.  This will draw
either a scrollable view or a non scrollable view either with or
without a bounding box around the view.

```c
static void draw_views(uC_widget_vg_t *vg)
```

This static function will draw all views within the window of the
specified view group.  Views are required to fit inside their view
group before they can be attached.  Individual widget draw helpers
clear and draw only within the widget width and clip label text at
the widget boundary instead of wrapping into adjacent cells.

```c
uint16_t widget_clear_width(uC_window_t *win,
uint16_t x, uint16_t y, uint16_t width)
```

Internal helper used by widget draw code.  It clears at most width
cells on a single row starting at x/y, clipped to the containing
window's right edge, resets the cursor to x/y, and returns the
number of drawable cells available to the widget.

```c
bool widget_emit_clipped(uC_window_t *win, uint32_t codepoint,
uint16_t *remaining)
```

Internal helper used by widget draw code.  It emits one UTF-8
codepoint only if that codepoint fits within both the widget's
remaining width and the current window row.  It returns false when
the codepoint would overflow, allowing callers to stop drawing
without triggering the window emitter's normal line wrap.

```c
void widget_puts_clipped(uC_window_t *win, const char *text,
uint16_t *remaining)
```

Internal helper used by widget draw code.  It writes a UTF-8 string
until the next codepoint would exceed the widget's remaining width
or the current window row.

```c
void draw_view_groups(uC_screen_t *scr)
```

This function is part of the main screen display update procedure.
It will draw all view groups into the specified screen with all
their associated views and widgets.


### 9.5. `uC_widget_button.c`
```c
static void draw_btn_txt(uC_window_t *win, uint16_t x, uint16_t y,
uint16_t width, const char *name, char key)
```

This static function draws the text name into the button.  If the
button has an associated letter defined the first instance of that
character within the name is underlined.  The button text is
clipped to the button width and UTF-8 display width is used when
centering and clipping.

```c
void draw_button(uC_window_t *win, uC_widget_t *widget,
uint16_t x, uint16_t y)
```

This function simply draws a button with its button text.  If the
button also has an associated letter the first instance of that
letter within the text is underlined.  The rendered button is
clipped to the widget width and never wraps into adjacent cells.

```c
uint8_t handle_button(uint8_t k)
```

This function is called as part of the widget keyboard handler
when a button has focus.  If the key pressed was the enter key or
carriage return and the button has an associated letter then that
letter is returned to the application code.  Mouse clicks on that
same button return the same letter.

If the enter key is pressed but the button does not have an
associated letter then the buttons tab sequence is written into
the address pointed to by the buttons select pointer and

```text
UC_KEY_NONE is returned.  In this mode the widget system has
```

already run the button action and no application shortcut key is
generated.

The button letter is automatically registered as a screen-level
shortcut when the containing view group is attached to a screen.
Duplicate shortcut registration is not fatal; the button still
displays and returns its letter when focused or clicked, but the
existing shortcut owner keeps the global accelerator.

Unlike menu shortcuts, a widget button letter is intentionally
restricted to a single ASCII letter that appears somewhere in the
button text.  The match is case-insensitive at creation time and
the stored letter is normalized to the actual character from the
button name so the displayed underline lands on real text.

```c
API uC_widget_t *uC_widget_button_create(uint16_t *select,
const char *name, char letter, uint16_t width,
uC_attribs_t attrs, uC_attribs_t focus)
```

This public API call is used to create a new button widget.  The
letter parameter is the associated return/display letter.  If that
letter is not a single ASCII letter present in the button name, it
is ignored.  If the button has no valid letter specified then
there must be a select parameter specified.  If neither are
specified then a press of this button will be effectively
un-selectable.


### 9.6. `uC_widget_check.c`
```c
void draw_check(uC_window_t *win, uC_widget_t *widget,
uint16_t x, uint16_t y)
```

This function is called as part of the widget draw procedure and
draws a single checkbox widget using the selected UTF-8 characters
for both on and off states.  The checkbox will be drawn with
either its focused or unfocused attributes.  The checkbox graphic,
spacer, and label are clipped to the widget width and never wrap
into adjacent cells.

The widgets state bit is taken from a 32 bit variable pointed to
by the *select element within the widget structure.  The widget
also contains a variable giving which bit within the select is
associated with this checkbox.

```c
uint8_t handle_check(uint8_t k)
```

This function is part of the widget keyboard handler and is called
when a key is pressed while a checkbox has focus.  If the key
pressed was the space key then the bit within the *select element
of the widget given in the bit element of that structure will be
toggled either on or off.

Any number of bits within *select may be turned on in this way.

```c
API uC_widget_t *uC_widget_check_create(
uint32_t *select, uint16_t bit, const char *name,
uC_radio_type_t type, uint16_t width,
uC_attribs_t attrs, uC_attribs_t focus)
```

This public API call is used to create a new checkbox widget.  The
name given here will be drawn to the right of the selected
checkbox graphic specified in the type parameter.

The *select parameter is a pointer to a 32 bit variable which will
contain the state of up to 32 checkbox widgets.  The specific bit
within that variable for this widget is specified in the bit
parameter here.


### 9.7. `uC_widget_radio.c`
Radio buttons are visually very similar to the checkbox widget except
where there can be any number of set buttons within the checkbox view only
one widget may be set within a radio button view.

Note: A checkbox view can have more than 32 buttons assigned to it but
this will split buttons across multiple *select variables and the code
here will not ensure that only one bit across all *select variables is
set.  It will only ensure that only one bit within the *select of the
currently selected button will be set.

```c
void draw_radio(uC_window_t *win, uC_widget_t *widget,
uint16_t x, uint16_t y)
```

This function is called as part of the widget draw procedure and
draws a single radio button widget using the selected UTF-8
characters for both on and off states.  The radio button will be
drawn with either its focused or unfocused attributes.  The radio
graphic, spacer, and label are clipped to the widget width and
never wrap into adjacent cells.

The widgets state bit is taken from a 32 bit variable pointed to
by the *select element within the widget structure.  The widget
also contains a variable giving which bit within the select is
associated with this radio button.

```c
uint8_t handle_radio(uint8_t k)
```

This function is part of the widget keyboard handler and is called
when a key is pressed while a radio button has focus.  If the key
pressed was the space key then the bit within the *select element
of the widget given in the bit element of that structure will be
toggled either on or off.

If the bit is toggled to an on state then any other bits within
*select which are in the on state will be turned off.

```c
API uC_widget_t *uC_widget_radio_create(
uint32_t *select, uint16_t bit, const char *name,
uC_radio_type_t type, uint16_t width,
uC_attribs_t attrs, uC_attribs_t focus)
```

This public API call is used to create a new radio button widget.
The name given here will be drawn to the right of the selected
radio button graphic specified in the type parameter.

The *select parameter is a pointer to a 32 bit variable which will
contain the state of up to 32 radio button widgets.  The specific
bit within that variable for this widget is specified in the bit
parameter here.


### 9.8. `uC_widget_textbox.c`
A textbox allows the application to accept the input of strings into
pre-allocated buffers.  The characters of these strings can be filtered on
one of the following number bases or can be alpha-numeric.

- `uC_INPUT_BINARY`
- `uC_INPUT_OCTAL`
- `uC_INPUT_DECIMAL`
- `uC_INPUT_HEX`
- `uC_INPUT_ALPHA`

Alpha-numeric input allows for editing of any of the following characters
or the space

```text
!@#$%^&*()-=_+[]{}|\"';:`,./?<>
0123456789
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
```

The widget editor allows for basic editing of strings including deleting
characters and moving the cursor within the edit field.

```c
void draw_textbox(uC_window_t *win, uC_widget_t *widget,
uint16_t x, uint16_t y)
```

This function draws a textbox widget.  It first draws the widget
name, and then to the right of that draws the widgets edit box.
If this widget has been or is being edited then the edit string
will be drawn into the edit box one character at a time.

If the next character to be drawn is at the widgets cursor
location then the character will either be drawn with an underline
or in reverse video depending on whether the widgets editor is in
insert mode or overwrite mode.  The underline is used for
overwrite mode and reverse video is used for insert mode.  Should
those be inverted?

Note: A string being edited into an edit box can be longer than
the box is wide.

```c
static bool test_char(uC_widget_textbox_t *t, uint8_t k)
```

This static function verifies that the character entered into the
textbox is valid for that textbox's selected radix.  If the
character is invalid then this function will return false.
Otherwise it will return true.

```c
static void lt(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and moves the
cursor left one space within the edit box.

```c
static void rt(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and moves the
cursor right one space within the edit box.

```c
static void insert_char(uC_widget_textbox_t *t, uint8_t k)
```

This static function is part of the textbox editor and will insert
a single character into the edit box string at the current cursor
location.  It will adjust the location of any other characters in
the string if need be and will not overwrite any existing
characters.

```c
static uint8_t write_char(uC_widget_textbox_t *t, uint8_t k)
```

This static function is part of the textbox editor and will write
a single character into the edit box string.  This will either
insert that character or will overwrite the character at the
current cursor location.

```c
static void del(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and will delete
the character that is at the current cursor location moving any
other characters down as needed.

```c
static void bs(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and will delete
the character that is immediately to the left of the character at
the current cursor location.  It will move any other characters
down as needed.

```c
static void home(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and will move
the cursor to the start of the string within that edit box.

```c
static void end(uC_widget_textbox_t *t)
```

This static function is part of the textbox editor and will move
the cursor to the end of the string within that edit box.

```c
uint8_t handle_textbox(uint8_t k)
```

This function is called by the widget keyboard handler when a
textbox widget has focus.  If the textbox is only focused, Enter
starts editing and printable keys are returned to the caller.  Once
editing is active the textbox handles cursor movement, insert,
delete, backspace, and valid printable characters.  Enter or Esc
ends editing.

```c
API uC_widget_t *uC_widget_textbox_create(
char *data, const char *name, uint16_t size,
uC_textbox_radix_t radix, uint16_t width,
uC_attribs_t attrs, uC_attribs_t focus)
```

This public API function is used by application developers to
create a new textbox widget.  It requires way too many parameters!


### 9.9. `uC_widget_scan.c`
Every widget must have a unique tab select sequence value within the set
of active widgets.  The values do not need to be contiguous.  The tab
sequence is utterly independent of which view or view group a widget is
associated with.

For larger interfaces it is often useful to reserve sequence ranges per
panel, popup, or logical widget group.  For example:

```c
#define CONFIG_BASE 0x0100
#define STATUS_BASE 0x0200
```

```c
#define CONFIG_SEQ(n) (CONFIG_BASE + (n))
#define STATUS_SEQ(n) (STATUS_BASE + (n))
```

This keeps tab sequence ownership obvious in application code.  Because
Tab and Back-Tab scan real active widgets rather than probing every
integer in the range, gaps between these reserved ranges are harmless.

When a widget is associated with a scrollable view then the sequence
number of the first widget within that view is controlling.  The sequence
number of all other widgets within that view are ignored.

The functions in this source file search every view in every active view
group within the current active screen.  Direct selection searches for an
exact sequence value.  Tab and Back-Tab scan actual widgets once and pick
the next or previous real sequence number, wrapping to the lowest or
highest active sequence as needed.  This means sparse sequence ranges are
allowed without forcing the tab handler to probe every integer in a gap.

```c
static bool scan_view(uC_widget_view_t *view, uint16_t sequence)
```

This static function searches through all widgets within a
specified view for one with the specified tab sequence number.  If
a widget with this sequence number is found, that widget is marked
as having focus and a true result is returned.

If no widget is found with this sequence number then a false
result is returned.

```c
static bool scan_vg(uC_widget_vg_t *vg, uint16_t sequence)
```

This static function searches through all views within a view
group for a widget with the specified tab sequence number.  If a
widget is found with this sequence number then...

- The view group's window is marked as having focus.
- The view is set as the current view within the widget_state
  variable.
- The view group is set as the current view group within the
  widget_state variable.
- A true result is returned.

If no widget is found then a false result is returned.

```c
API bool uC_widget_select_widget(uint16_t sequence)
```

This public API function first removes focus from the currently
selected view group's window and marks the currently focused
widget as no longer focused.  It then searches through all view
groups attached to the currently active screen for a widget with a
tab sequence number equal to the specified value.

If a widget with the specified tab sequence number is found then
that widget, its parent view and its parent view group are all
marked as having focus and a true result is returned.  Otherwise a
false result is returned.

```c
uint8_t tab_next_widget(void)
```

This function is part of the widget keyboard handling routines.
It retrieves the sequence number of the currently focused widget
from the widget_state variable and selects the active widget with
the smallest sequence number greater than the current one.  If no
such widget exists, focus wraps to the active widget with the
lowest sequence number.

```c
uint8_t tab_prev_widget(void)
```

This function is part of the widget keyboard handling routines.
It retrieves the sequence number of the currently focused widget
from the widget_state variable and selects the active widget with
the largest sequence number less than the current one.  If no such
widget exists, focus wraps to the active widget with the highest
sequence number.


### 9.10. `uC_widget_keys.c`
This source file contains the keyboard handling glue for the widget
engine.  Single decoded key presses are passed to the individual handlers
for each widget type within their own source files.

In normal applications widgets are handled through the consolidated
`uC_key()` dispatcher.  `uC_key()` reads and decodes the terminal key, then
calls the internal `widget_key()` helper after screen shortcuts and menu
navigation have had their chance to consume the key.

The older `uC_widget_main()` loop is still present for applications that use
the widget engine directly.  That loop reads through `uC_key_raw()` so it
does not recursively call the consolidated dispatcher.

The cursor up and cursor down keys are used to move up or down within a
scrollable widget view if it has focus and to potentially scroll that
widget in one or other direction.

```c
static bool check_scrollable(uint8_t k)
```

This static function first checks if the currently focused view is
scrollable and if so, is either the cursor up key or cursor down
key pressed.

If all of the above tests are true it will then call a function
which will move the focus up or down one widget within that view
and will potentially scroll that view.

```c
static uint8_t handle_widget_key(uint8_t k)
```

This static function handles a single character key press and
passes the key to the handler for the current widget type.  If the
current view is scrollable and the key is a cursor movement then
the view is scrolled either up or down.

After handling the key press it is also returned to the caller.

```c
bool widget_text_input_active(uC_screen_t *scr)
```

Internal helper used by `uC_key()` to determine whether the current
focused widget is a textbox that is actively editing.  When true,
the screen shortcut registry is skipped so printable shortcut keys
are delivered to the textbox editor.  A focused textbox that is not
editing does not block global shortcuts.

```c
bool widget_key(uC_screen_t *scr, uint8_t key, uint8_t *out)
```

Internal helper used by `uC_key()` after shortcut and menu handling.
It checks whether the widget engine is active on the supplied
screen.  If not, it returns false and the key continues through the
dispatcher.

If widgets are active, Tab and Back-Tab move to the next or
previous widget, mouse events are routed to the widget mouse
handler when mouse support is enabled, and all other keys are
passed to `handle_widget_key()`.  Mouse clicks on key-bearing
buttons return that button key.  Mouse clicks on keyless
buttons, radio buttons, and checkboxes run the widget action
internally.  The resulting key is written to *out and the helper
returns true to indicate that the widget layer consumed the
original key.

```c
static uint8_t _widget_key(void)
```

This static function blocks until a key is ready to read.  The
key is tested to see if it is the tab key or back-tab key, in
which case a function is called to move to either the next or the
previous widget.

If it is neither the tab key or back tab key then which ever key
was pressed is passed to the above `handle_widget_key()` function.

If the key pressed was one that returns an escape sequence and is
registered by the widget handler then that escape sequence will be
translated into a single key press which will also be handled
here.

```c
static uint8_t widget_read_key(void)
```

This function is a wrapper for the above `_widget_key()` function.
It repeatedly call said function until there are no more keys left
in the input buffers to be processed.

This allows the widget handler to inject single character key
values into the keyboard input buffer when ever a key is pressed
which returns an escape sequence.  The escape sequence is thereby
translated into a single character to be processed.

The keys which are translated in this way are detailed below.

```c
static void widget_key_up(void)     static void widget_key_down(void)
static void widget_key_left(void)   static void widget_key_right(void)
static void widget_key_insert(void) static void widget_key_home(void)
static void widget_key_delete(void) static void widget_key_end(void)
```

These static functions handle keys which return an escape sequence
and inject a single character back into the keyboard input buffer
in their place.  This allows these key presses to be handled by
the functions above instead of being processed by the uCurses
internal key handler.

```c
static void set_widget_key_actions(void)
```

This function registers each of the above escape sequence handlers
which translate the key presses into a single character.  This is
part of the legacy `uC_widget_main()` path.  The consolidated
`uC_key()` dispatcher normally uses the default `UC_KEY_`* translations
and gives the menu system first shot at `F10`.

```c
API char uC_widget_main(void)
```

This public API is the main keyboard handler for the widget
engine.  It allocates a key actions array for the widget engine
and then registers the keys needed by it.   It then calls the
above `widget_read_key()` function to handle key presses.  When that
function returns the key handler is released and the previous one
is reinstated.

Which ever key was pressed is returned to the caller.


## 10. Utilities
This is where I place modules or functions that are somewhat useful to the
developer or the library itself but which do not belong in any of the
above documented files.

Some of these are contained within the somewhat boringly named `uC_utils.c`
and others are in their own modules such as the linked list module in
`uC_list.c` or the memory allocation trackers in `uC_alloc.c`.   The purpose
of each of these is documented below.


### 10.1. `uC_utils.c`
This source file contains many tiny but useful functions which are
somewhat difficult to categorize within any of the above modules.  They
are used extensively by the library itself but are all public API entities
so are also available to the application developer.

```c
API void uC_noop(void) { ; }
```

This public API call is the single most useful function in the
entire library.  It performs absolutely no operation what so ever.
It can be used to safely neuter a function pointer that is no
longer contextually appropriate but which should not actually be
assigned a `NULL` value.

```c
API void uC_ui_free(void *mem)
```

This public API call is a wrapper function which will free any
memory address associated within the user interface.  This happens
enough to warrant this special wrapper function which would
otherwise be somewhat wasteful to implement.

```c
API int32_t fnv_hash(uint8_t *s)
```

This public API call computes a FNV hash on the string passed to
it.
The algorithm used is FNV-1: the hash is first multiplied by the
FNV prime and then XORed with the next byte.  FNV-1a reverses
those two operations (XOR then multiply).  The constants used
(prime `0x01000193`, basis `0x811c9dc5`) are correct for both
variants so the distinction has no practical effect on the
library.

It is used as a means of negating the need to do string compares
within the library which are widely accepted as being unsafe.  I
am somewhat worried that this might prove to be equally unsafe but
it also has certain utility value that still makes it preferable.

For example, it allows the JSON parser to switch on strings which
is not built into the C language.

```c
API void uC_clock_sleep(int32_t whence)
```

This public API call is used to pause for a specified length of
time in nano seconds.

```c
API void uC_init_terminal(void)
```

This public API function is used as part of the uCurses
initialization procedure.   It configures the terminal to use a
raw mode with no echo on user input.   The current state of the
terminal configuration is saved so that it can be restored on
exit.

```c
API void uC_restore_terminal(void)
```

This public API call restores the terminal configurations back to
their saved state.  It is used as part of the libraries exit
procedure so the terminal is not left in a squirly state.

```c
API __attribute__((noreturn)) void uC_abort(const char *msg)
```

This public API call is used as a somewhat graceful abort.  If a
fatal handler has been registered via `uC_set_fatal_handler()` then
that handler is called first with the error message, giving the
application one last chance to log the error or `longjmp()` to a
recovery point.  If the handler returns normally the terminal is
restored to its original state and the application exits.

```c
API void uC_set_fatal_handler(void (*fp)(const char *msg))
```

This public API call registers a user-supplied callback that will
be invoked by `uC_abort()` before the application exits.  The
callback receives the same error message string that `uC_abort()`
was called with.

The handler may call `longjmp()` to bypass the exit entirely,
allowing the application to attempt recovery.  If the handler
returns normally then `uC_abort()` will proceed with terminal
restoration and exit(1).

Pass `NULL` to deregister a previously registered handler.

```c
API void uC_get_console_size(uint16_t *width, uint16_t *height)
```

This public API call is used to query the current size of the
virtual terminal.   I have come to learn that this method is not
very reliable in all cases and that there are other methods that
I should also be using.  This is on the "some day" todo list to
fix maybe?

```c
API void uC_assert(bool f, const char *file, int line, const char *msg)
```

This public API call asserts that a passed in condition is true.
If the condition is true this function simply returns to the
caller.  If the condition is false the library is de-initialized
and a specified error message is written out to stderr and the
application is aborted.


### 10.2. `uC_alloc.c`
This module is not a memory allocator, it is simply an associative memory
allocation tracker.   This enables the library and user applications to
compartmentalize memory allocations so that all allocations associated
with a given purpose can be de-allocated on demand without ripping the rug
out from under any other allocation type.

Associations are based on memory zones.  At the time of writing there are
currently only eight zones defined.   This is probably overkill but if you
require more you can always rebuild the library (tm).

- `uC_MEM_ZONE_DEFAULT`
- `uC_MEM_ZONE_UI`
- `uC_MEM_ZONE_JSON`
- `uC_MEM_ZONE_USER1`
- `uC_MEM_ZONE_USER2`
- `uC_MEM_ZONE_USER3`
- `uC_MEM_ZONE_USER4`
- `uC_MEM_ZONE_USER5`

The JSON memory zone is only actively being used when the JSON parser is
initializing the user interface.   At all other times this memory zone is
not utilized.

This means that this zone can be repurposed in certain cases.  If your
application is running on a system that could support the sigwinch signal
and you have the JSON engine built in then you would not be able to
repurpose this zone as a winch will tear down and rebuild the user
interface using the JSON engine.  Otherwise knock yourself out!

Each memory zone may have a certain number of allocations made against it.
Each allocation is tracked within an array within the zone structures.  If
an attempt is made to make an allocation against any zone which is 'full'
then the allocation array will be expanded to allow this.  There is no
mechanism to shrink expanded zones.

```c
static void mem_abort(void)
```

This static function is a graceful exit from the application which
is attempting to allocate memory on the condition where the system
is out of memory.

```c
API void uC_mem_purge(uC_mem_zone_t zone)
```

This public API call is used to de-allocate all allocations which
are associated with a specified memory zone.  Only allocations
which are associated with that zone are freed.  Once all
allocations have been freed then zone structure including the
array tracking those allocations is freed.

Any attempt to make allocations against this zone after calling
this function will require the module to re-initialize the zone
with its smallest tracking array.

```c
API size_t uC_zone_query(uC_mem_zone_t zone)
```

This public API will return the total amount of memory which is
allocated within the specified memory zone.  This can be used to
track which parts of your application are using the most memory or
to verify that a memory purge of a specific zone has been
successful.

```c
static void alloc_init(uC_mem_zone_t zone)
```

This static function is used to initialize a memory zone when the
first allocation within it is requested.  It will initialize the
structures for this zone including allocating an array of
allocations made against this zone.

```c
static bool check_zone_full(uC_mem_zone_t zone)
```

This static function is called when an attempt to allocate memory
against a specified zone is made.  It ensures that the maximum
number of allocations have not already been made against that zone
and will, if need be, increase the maximum to accommodate the
requested allocation.

```c
API void *uC_alloc(uC_mem_zone_t zone, size_t size)
```

This public API call is used by applications to allocate memory
against a specified zone.   All allocations are made using the
standard C `calloc()` function.

Note: On a highly resource constrained embedded system `calloc()`
and related function calls are somewhat problematical.  In the
future I might modify this by adding a more resource friendly
allocator to the library but this is not something I would look
forward to and would also increase the size of the library.

```c
API void *uC_realloc(uC_mem_zone_t zone, void *addr, size_t size)
```

This public API call resizes a buffer previously allocated against
the specified memory zone.  If `addr` is `NULL`, the call behaves
as a zone-tracked allocation.  If `size` is zero, the tracked buffer
is freed and `NULL` is returned.

The address must already be associated with the specified zone.  If
the address is not found, this function returns `NULL` without
modifying the allocation tracker.  If the underlying `realloc()`
fails, the original buffer remains tracked and valid.

```c
API void uC_free(uC_mem_zone_t zone, void *addr)
```

This public API call is used to de-allocate a buffer previously
allocated against the specified memory zone.  If no allocation
at the specified address is associated with the specified zone
then this function will silently ignore the error.

Otherwise the memory is freed and the specified address is removed
from the array of tracked allocations.


### 10.3. `uC_braille.c`
This module contains functions to allow displaying of graphical entities
using the UTF-8 Braille character set with UTF-8 Code points from `0x2800`
to `0x28ff` as follows.

```text
00  ⠁⠂⠃⠄⠅⠆⠇    40 ⡀⡁⡂⡃⡄⡅⡆⡇    08 ⠈⠉⠊⠋⠌⠍⠎⠏    48 ⡈⡉⡊⡋⡌⡍⡎⡏
10 ⠐⠑⠒⠓⠔⠕⠖⠗    50 ⡐⡑⡒⡓⡔⡕⡖⡗    18 ⠘⠙⠚⠛⠜⠝⠞⠟    58 ⡘⡙⡚⡛⡜⡝⡞⡟
20 ⠠⠡⠢⠣⠤⠥⠦⠧    60 ⡠⡡⡢⡣⡤⡥⡦⡧    28 ⠨⠩⠪⠫⠬⠭⠮⠯    68 ⡨⡩⡪⡫⡬⡭⡮⡯
30 ⠰⠱⠲⠳⠴⠵⠶⠷    70 ⡰⡱⡲⡳⡴⡵⡶⡷    38 ⠸⠹⠺⠻⠼⠽⠾⠿    78 ⡸⡹⡺⡻⡼⡽⡾⡿
80 ⢀⢁⢂⢃⢄⢅⢆⢇    C0 ⣀⣁⣂⣃⣄⣅⣆⣇    88 ⢈⢉⢊⢋⢌⢍⢎⢏    C8 ⣈⣉⣊⣋⣌⣍⣎⣏
90 ⢐⢑⢒⢓⢔⢕⢖⢗    D0 ⣐⣑⣒⣓⣔⣕⣖⣗    98 ⢘⢙⢚⢛⢜⢝⢞⢟    D8 ⣘⣙⣚⣛⣜⣝⣞⣟
A0 ⢠⢡⢢⢣⢤⢥⢦⢧    E0 ⣠⣡⣢⣣⣤⣥⣦⣧    A8 ⢨⢩⢪⢫⢬⢭⢮⢯    E8 ⣨⣩⣪⣫⣬⣭⣮⣯
B0 ⢰⢱⢲⢳⢴⢵⢶⢷    F0 ⣰⣱⣲⣳⣴⣵⣶⣷    B8 ⢸⢹⢺⢻⢼⢽⢾⢿    F8 ⣸⣹⣺⣻⣼⣽⣾⣿
```

This character set comprises of 256 characters which contain a pattern of
two wide, four high dots ranging from 0 dots to 8 dots total.  They can be
used to draw almost any graphical image such as bar graphs and the like.

The uCurses example code shows them being used to draw a texture mapped
3d maze using a ray casting engine which I found on the net (attributed).

They also demonstrate a very cheezy super-sampled Mandelbrot viewer.

```c
API int16_t uC_braille_xlat(uint8_t chr)
```

This public API call will translate an 8 bit vale from `0x00` to
`0xff` into the UTF-8 code point for one of the above braille
characters with the same bit pattern.

```c
API void uC_braille_8(uC_window_t *win, uint16_t *braille_data,
uint8_t *map, uint16_t width)
```

This public API call will translate a bitmap of a specified size,
containing 8 bits per pixel into a UT`F8` braille character
representation of that image.

```c
API void uC_braille_1(uC_window_t *win, uint16_t *braille_data,
uint8_t *map, uint16_t width, uint16_t height)
```

This public API call will translate a bitmap of a specified size,
containing 1 bit per pixel into a UT`F8` braille character
representation of that image.

```c
API void uC_draw_braille(uC_window_t *win, uint16_t *braille_data)
```

This public API call will draw a UTF-8 braille character image
which is equal in size to the specified window into that window.


### 10.4. `uC_entry.c`
This module contains the entry point code for the library.  This includes
code to be run if the .so file is ever executed directly.  In this case
the code here will parse and interpret the command line arguments passed
to it.


#### 10.4.1. Executing the .so directly - the ELF trick
Shared libraries are not normally executable.  uCurses uses two standard
ELF mechanisms to make `libuCurses.so` directly executable from the shell:

1. A .interp section is embedded in the .so which contains the path
  to the system dynamic linker (`/lib64/ld-linux-x86-64.so`.2).  This
  tells the kernel how to load and run the file as a program.

2. The linker is invoked with -Wl,--entry=entry which causes the
  `entry()` function in this module to be used as the ELF entry point
  rather than the default _start symbol.

The result is that the library can be run as both a shared library loaded
by client applications and as a standalone program:

```sh
./build/libuCurses.so --help
./build/libuCurses.so my_vectors.txt
```

Because this entry point is not `main()`, the standard argc/argv mechanism
is not available.  Command line arguments are instead read directly from
/proc/self/cmdline, whose content is the NUL-separated argv array written
by the kernel for every running process.


#### 10.4.2. The menu dispatch code generation workflow
JSON-described menus attach a "vector" string to each menu item.  This
string is the name of the C function that should be called when the item
is selected.  At runtime the JSON parser needs to resolve that string to
an actual function pointer.  C provides no reflection mechanism for this;
a function pointer cannot be looked up from its name at runtime.

The solution is to pre-compute FNV hash values for every vector name at
build time and embed the resulting dispatch table as ordinary C source code
in the application.  The JSON parser then hashes the vector string once
when the JSON is loaded and performs a single integer comparison against
the table rather than a series of strcmp calls.  The function name strings
themselves never appear in the final application binary.

The workflow is:

Step 1 - create a plain text file containing one vector name per line.
These must exactly match the "vector" values in your JSON file.
Lines beginning with '/' are treated as comments and ignored.

For example, vectors.txt might contain:

file_new
file_open
file_save
file_quit

```text
// edit menu
```

edit_cut
edit_copy
edit_paste

Step 2 - run the library against that file:

```sh
./build/libuCurses.so vectors.txt
```

The library writes ready-to-compile C code to stdout.  Redirect it
to a source file in your project:

```sh
./build/libuCurses.so vectors.txt > menu_dispatch.c
```

The generated output contains a switch_t array of {hash, function
pointer} pairs and a `menu_address_cb()` callback function that the
JSON parser calls to resolve a vector name to its function pointer.

Step 3 - add `menu_dispatch.c` to your application build.  The JSON
parser will call `menu_address_cb()` automatically during menu
construction; no other integration is required.

The --help argument to the library describes this same process.

```c
static uint8_t *next_arg(void)
```

This static function parses the next command line argument out of
the /proc/self/cmdline file which has been previously opened. A
pointer to the parsed argument is returned to the caller.

```c
static void separator(void)
```

This static function draws a simple separator line out to stdout
when the library prints some simple source code to be included
within the target application.

```c
static void print_cb(uint8_t *path)
```

This function writes a small amount of source code out to stdout.
This code is to be included in an applications sources and will be
used to associate a function vector with a strings hash value.

This is used by the pull-down menu initialization code used within
the JSON parser.  See Below.

```c
static void print_switch(uint8_t *path)
```

This function writes a small amount of source code out to stdout.
This code is to be included in an applications sources and will be
used to associate a function vector with a strings hash value.

This is used by the pull-down menu initialization code used within
the JSON parser.  See Below.

```c
static void hash_file(const uint8_t *p)
```

This function is used to compute a number of hash values for the
strings contained within the specified file.  These are names of
functions to be used with the applications pull-down menu system
which will be built by the JSON parser.

It is not possible in C to specify a string at run time which is
to be associated with a given function.  A pointer to the function
must be associated with the string at compile time which makes it
somewhat difficult to construct the dynamic pull-down menus
at run time using the JSON parser.

These functions will, given a list of strings, compute a hash
value for each string and will write out code which user
applications can then include in their build.  When their
application is launched the values calculated here can be
associated with functions of the same name by the JSON parser.

This allows applications to describe their entire menu system
within their JSON UI description allowing the JSON parser to
connect a function name string to said functions address.

```c
static const uint16_t smushed[]
```

This static array is a 'compressed' instance of the usage help
info.  This compression method is ULTRA cheezy and won't be
described here but it actually works quite well.

Patent not pending!

```c
static int8_t read_c1(void)
```

This function is part of the 'smushed' decompression routines.  It
reads compressed data and returns one decompressed character to be
displayed on stdout when the user passes a --help command line
argument.

```c
static void help(void)
```

This static function is called when the user passes a --help
command line argument.  It will decompress and display the entire
help string contained within the smushed data array.

```c
static const char message[]
```

This array contains the same data as contained within the smushed
compressed data array but is not compressed.  Which help data is
included is based on a conditional compilation selection.

```c
static void help(void)
```

This version of the help function is used when the help data is
compiled in an uncompressed format.

```c
static void process_args(void)
```

This static function parses one item out of the command line and
selects one of two different operations based on that string.  If
the string is "--help" the smushed or non smushed help message is
written out to the console.

If the string is not "--help" then it is assumed to be a file
which contains a list of strings, one string per line which are
the names of functions to be called by the pulldown menu system.

A FNV hash is computed for each string and code is written out to
the console which can be included in the application developers
sources to allow the JSON parser to associate code function
addresses with the hash values of each string passed in here.

```c
API void entry(void)
```

This function is analogous to `main()` in a non .so file build.  It
allows for the `libuCurses.so` file to be executed directly from the
command line.   It first displays a version string for the library
and then processes and handles any command line arguments if any
passed to the library.


### 10.5. `uC_eval.c`
The functions within this file will evaluate a string, interpreting said
string as a numerical value in some specified radix.  ANY radix can be
specified and this code will reliably interpret the string and return a
correct value to the caller.

This code is a port of the code I implemented in my X86 forth compiler X4
which was there written in pure x86_32 assembler and which was also later
ported to x86_64 assembler (ftw!).

```c
static uint8_t digit(uint8_t c, uint8_t radix, uint8_t *result)
```

This static function will parse a single character out of a string
and will convert it to some numerical value based on the current
radix.

If the character is valid in the currently selected radix then the
numerical value of that character is returned to the caller.  If
the character is not valid in the current radix then a result of
zero is returned.

This results in an incorrect end result and no error handling is
performed.  The library will not play mommy here or hold your hand
protecting you from you as adding such error handling would add
some code size to the library and I elected to let you suffer }:)

```c
API uint8_t eval(uint8_t *s, uint32_t *result, uint8_t radix)
```

This public API call, when passed a string, an address for a
result to be stored and a radix to be used will convert the string
into a numerical value based on the specified radix (anything from
2 to 36 are valid I believe) and will store the computed numerical
value of that string in the specified address.


### 10.6. `uC_list.c`
These sources provide a very simple but fast linked list engine.  These
functions have been in almost constant use for at least 20 years and I am
absolutely 100% positive that they are now absolutely 100% bug free! (tm)!

```c
API uC_list_node_t *uC_list_scan(uC_list_t *list, uC_list_node_t *n1)
```

This public API function allows application code (and the library
itself) to iterate through a list one node at a time.   On each
call it will return the address of the next node in the list or
a `NULL` if there are no further items in the list.

The application must pass in one if two parameters.  On the first
call it should pass in the address of the list to be scanned and
a `NULL`.   It will then return the address of the first node in
the list if one is present.

On subsequent calls the application should pass in a `NULL` list
address and the address of the node which was returned to it in
the previous call.

The application code will need to extract the (void *) payload
from each node and cast it to the appropriate data type to be
handled as needed.

```c
API bool uC_list_insert_node(uC_list_node_t *node1, void *payload)
```

This public API call will insert a new node which is to contain
the specified payload into the middle of an existing list
immediately following some other node which is already in the
list.

```c
static void node_remove(uC_list_node_t *node1)
```

This static function will remove a specified node from the list
and de-allocate the node structure.  If this node is between two
other nodes or is either at the head or tail of the list then all
links will be adjusted to accommodate the removal of the node.

```c
API void uC_list_remove_node(uC_list_t *list, void *payload)
```

This public API call will remove which ever node, contained within
the specified list carries a payload at the specified address.

If more than one node carries a payload with this address then
only the first one found will be removed.

```c
API bool uC_list_push_head(uC_list_t *list, void *payload)
```

This public API call will push a new node containing the specified
payload onto the head of the specified list.

```c
API bool uC_list_push_tail(uC_list_t *list, void* payload)
```

This public API call will push a new node containing the specified
payload onto the tail of the specified list.

```c
static void *list_pop(uC_list_t *list, bool whence)
```

This static function will pop (remove) one node from either the
head or tail of the specified list.   The payload of this node is
returned to the caller and the node structure itself is freed.

```c
API void *uC_list_pop_head(uC_list_t *list)
API void *uC_list_pop_tail(uC_list_t *list)
```

These public API functions remove one item from either the head or
tail of the specified list, returning the payload of the removed
node.   The node structure which is removed from the list is also
freed.

```c
API bool uC_list_insert_before(uC_list_node_t *n1, void *payload)
```

This public API function will insert a new node in front of the
specified node within the specified list.  The new node will have
the specified payload associated with it.

There is currently no associated "insert after" function.

Stack and queue macros

`uC_list.h` provides four macros that let a `uC_list_t` serve as a
stack (LIFO) or queue (FIFO) without adding any new API functions
or increasing the library size.

```c
// stack (LIFO)
#define uC_stack_push(list, p)  uC_list_push_head(list, p)
#define uC_stack_pop(list)      uC_list_pop_head(list)
```

```c
// queue (FIFO)
#define uC_queue_put(list, p)   uC_list_push_tail(list, p)
#define uC_queue_get(list)      uC_list_pop_head(list)
```

The same `uC_list_t` type is used for all three purposes.  The zone
field must be set before first use:

`uC_list_t` list = { .zone = `uC_MEM_ZONE_USER1` };

Example output from `example/list_demo.c`:

stack (LIFO):
third
second
first
queue (FIFO):
first
second
third


### 10.7. `uC_switch.c`
If I documented my reasoning for creating this module it would come over
as a highly toxic rant.  This is because there are certain aspects of the
C programming language that I have absolutely nothing good to say about
and C's implementation of the switch statement is one of those.

I do still use C's switch mechanisms but I will avoid them where I can.

```c
API int uC_switch(const uC_switch_t *s, int size, int32_t option)
```

The `uC_switch_t` *s here is an array of `uC_switch_t` elements with
size elements.  Each `uC_switch_t` contains a 32 bit option value
and a pointer to a function.

This call will scan the specified array of `uC_switch_t` items for
one with an option equal to the one passed in here.  If found then
that options vector is called.

Clean, tidy and fast and enforces good factoring of code.   The
only disadvantages here are that you would need to re-implement
this call for every variant that expects different parameter types
or returns different value types and that the C compiler will not
warn you about cases not accounted for.

Use of this function is not mandated, you can ignore it with
impunity at your pleasure.


### 10.8. `uC_utf8.c`
Unicode assigns a unique numerical value called a code point to every
character in every writing system.  These values range from `0x000000` to
`0x10FFFF`.  Your terminal does not receive code points directly; they must
first be encoded into UTF-8 byte sequences.  UTF-8 is a variable length
encoding that represents code points using one to four bytes depending on
their value:

```text
0x000000 - 0x00007F   1 byte    0xxxxxxx
0x000080 - 0x0007FF   2 bytes   110xxxxx 10xxxxxx
0x000800 - 0x00FFFF   3 bytes   1110xxxx 10xxxxxx 10xxxxxx
0x010000 - 0x10FFFF   4 bytes   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
```

The high bits of the leading byte encode the sequence length.  Every
continuation byte begins with 10 which means any byte can be immediately
identified as either a lead byte or a continuation byte without any
surrounding context.  This self-synchronizing property is one of UTF-8's
more useful traits.

uCurses handles the full four-byte range for both encoding and decoding.
For display purposes, each code point has a column width of either 1 or 2.
This is determined by calling the POSIX `wcwidth()` function which
implements the Unicode East Asian Width rules.  Characters such as CJK
ideographs and wide emoji occupy 2 columns; everything else occupies 1.

A note on what uCurses does not support and will not support:

Zero-width combining characters (diacritical marks and similar) have
a `wcwidth()` of 0 meaning they overlay the previous character's cell
without advancing the cursor.  uCurses does not handle these; they
will be written as though they were ordinary 1-column characters.

Right-to-left scripts such as Arabic and Hebrew require a
bidirectional layout engine that understands text direction, character
shaping, and the interaction between LTR and RTL runs within a single
line.  Libraries such as FriBidi and Pango exist for exactly this
purpose.  Adding equivalent machinery to uCurses would grow it by an
order of magnitude and is entirely contrary to its goals.  RTL text
is therefore out of scope.

The following functions are contained within this source file.

```c
utf8_encode_t *utf8_encode(int32_t cp)
```

This internal function encodes a Unicode code point into its
UTF-8 byte representation and returns a pointer to a static
utf8_encode_t structure containing the encoded bytes, their
count and the display column width of the character as returned
by `wcwidth()`.

```c
API int16_t uC_utf8_is_wide(uint32_t cp)
```

Returns 1 if the specified code point occupies 2 display columns
and 0 if it occupies 1.  This is used internally when writing a
character into a window to determine how many cells to advance the
cursor and whether the following cell should be marked as dead.

`DEADC0DE` wide-character sentinel

uCurses stores window and screen contents as arrays of fixed-width
cells, but terminals do not draw every Unicode code point in one
column.  Many CJK characters and emoji occupy two columns.  When a
wide character is written into a window, the first cell stores the
actual Unicode code point and the next cell is marked with the
internal sentinel value `DEADC0DE`, defined in `uCurses.h` as
`0xdeadc0de`.

`DEADC0DE` does not mean that anything has gone wrong and it is not
part of the application's text.  It means "this cell is already
occupied by the wide character that began in the previous cell".
The sentinel allows the draw code to keep the terminal's visual
columns synchronized with the window buffer without storing a
second copy of the same character.

The sentinel is important when later drawing overlaps the wide
character.  If a single-width character overwrites the left half of
a wide glyph, uCurses must also write a space over the right half
or the terminal may leave a visual fragment behind.  If a wide
character overwrites an area whose right-hand cell contains an
ordinary character, uCurses must force that following cell to be
redrawn.  The screen drawing code uses the `DEADC0DE` marker to
detect and repair both cases.

uCurses does not try to support overlapping wide glyph fragments.
If a write begins on, or extends across, any cell owned by an
existing wide glyph, the old glyph is invalidated as a whole and
the new write wins.  This keeps the window buffer in a state that
can be represented by a normal fixed-cell terminal.

After all windows, widgets, menus, status windows and popups have
been composed into the screen buffer, uCurses normalizes the screen
buffer before diffing and emitting it.  This catches direct
overlays such as borders and pulldown menus, which are drawn into
the screen rather than into the window that originally owned the
wide glyph.

Application code should not intentionally write `DEADC0DE` as a
character.  It is an implementation detail used by `uC_win_emit()`,
`uC_scr_draw()`, and `uC_utf8_emit()`.  When `uC_utf8_emit()` receives
`DEADC0DE` it emits nothing, because the visible character was
already emitted from the owning cell to the left.

uCurses currently treats display width as either one or two
columns for layout purposes.  Zero-width combining characters and
bidirectional layout are explicitly out of scope for this library.

```c
API void uC_utf8_emit(uint32_t cp)
```

Encodes the specified code point and writes its UTF-8 bytes
directly into the terminfo escape sequence buffer for output to
the terminal.  If the code point is the sentinel value `DEADC0DE`
this function does nothing.

```c
API uint8_t utf8_decode(uint32_t *cp, uint8_t *s)
```

Decodes one UTF-8 character from the byte string at s, stores
the resulting code point at the address given by cp and returns
the number of bytes consumed.  If the byte sequence is malformed
the replacement character (U+FFFD, encoded as `0xEFBFBD`) is
stored instead.

```c
API uint8_t uC_utf8_char_length(uint8_t *s)
```

Returns the byte length of the UTF-8 character whose leading
byte is at s.  This is determined solely from the leading byte's
high bits and does not validate the continuation bytes.

```c
API int16_t uC_utf8_width(uint8_t *s)
```

Returns the total number of display columns that the UTF-8 string
at s will occupy when written to the terminal.  This accounts for
any 2-column wide characters in the string.

```c
API int16_t uC_utf8_strlen(uint8_t *s)
```

Returns the number of Unicode characters in the UTF-8 string at
s.  This is a character count, not a byte count.

```c
API int16_t uC_utf8_strncmp(uint8_t *s1, uint8_t *s2, int16_t len)
```

Compares up to len characters of two UTF-8 strings.  Comparison
is performed character by character on the encoded byte sequences.
Returns 0 if the strings are equal up to len characters or a
non-zero difference value if they are not.


### 10.9. `uC_win_printf.c`
This module will probably make purists cringe but it has proved to be one
of the most useful extensions added to this library.  It greatly
simplifies the use of a significant part of the uCurses API.

While these functions associates themselves with C's printf they do not
support the standard printf format specifiers but implement a custom set
thereof.  Some of these use more than one character (after the %) or may
expect more than one parameter.  Not something that is commonly seen.

Each of these operate on a window passed in to `uC_win_printf()` itself and
this window is saved to a static global variable (for the win!).

Where appropriate, for each of the following functions which is associated
with a format specifier I document both the specifier and the function
which handles it.

```c
API void uC_win_puts(uC_window_t *win, const char *p)
```

This public API call writes the specified UTF-8 string into a
window at that windows current cursor location.  It will advance
that windows cursor by the cell width of each character in the
string.

Note: The %up, %dn, %lt and %rt specifiers all use two characters for
consistency.  The %r specifier has multiple uses (documented below)
and that ambiguity makes the second character necessary for all four.

```c
static void up(void)               %up(n)
```

This static function handles the %up format specifier which will
scroll a window up n lines, erasing the bottom n lines.

This specifier takes a count parameter n and calls
`uC_win_scroll_up_n()` internally.  Use n = 1 to scroll by one line.

```c
static void dn(void)               %dn(n)
```

This static function handles the %dn format specifier which will
scroll a window down n lines, erasing the top n lines.

This specifier takes a count parameter n and calls
`uC_win_scroll_dn_n()` internally.  Use n = 1 to scroll by one line.

```c
static void lt(void)               %lt(n)
```

This static function handles the %lt format specifier which will
pan a window left n columns, erasing the n columns on the far
right.

This specifier takes a count parameter n and calls
`uC_win_scroll_lt_n()` internally.  Use n=1 to pan by one column.

```c
static void rt(void)               %rt(n)
```

This static function handles the %rt format specifier which will
pan a window right n columns, erasing the n columns on the far
left.

This specifier takes a count parameter n and calls
`uC_win_scroll_rt_n()` internally.  Use n=1 to pan by one column.

```c
static void r(void)                %rf or %rb
```

This static function handles both the %rf and %rb specifiers and
will modify either the windows foreground color or background
color, setting them to a 24 bit RGB color.  This format specifier
therefore expects three parameters, the red, the green and the
blue values to be set.

```c
static void f(void)                %fc or %fs
```

This static function handles both the %fc and %fs specifiers and
will modify a windows foreground color setting it to either a
specific 8 bit (palette-indexed) color or to a gray scale value.

```c
static void b(void)                %bc or %bs
```

This static function handles both the %bc and %bs specifiers and
will modify a windows background color setting it to either a
specific 8 bit (palette-indexed) color or to a gray scale value.

```c
static void xy(void)               %@
```

This static function handles the %@ specifier and will move the
windows cursor to the specified X, Y coordinates within that
window.  This specifier therefore expects two parameters being
both the X and the Y coordinates.

This call will not move the cursor outside the bounds of the
window.

```c
static void P(void)                %P
```

This static function handles the %P specifier and will move a
windows location within its parent screen to the specified X / Y
coordinates.   This specifier therefore expects two parameters
being both the X and Y coordinates.

This call will not move any part of the window outside the bounds
of its parent screen.

```c
static void x(void)                %x
```

This static function handles the %x specifier which will move a
windows cursor to the specified X coordinate within its current
line.

This call will not move the cursor outside the bounds of the
window.

```c
static void y(void)                %y
```

This static function handles the %y specifier which will move a
windows cursor to the specified Y coordinate within its current
column.

This call will not move the cursor outside the bounds of the
window.

```c
static void utf8(void)             %8
```

This static function handles the %8 specifier which will write a
single UTF-8 character into a window.

```c
static void c(void)                %cu or %cd or %cl or %cr
```

This static function handles the %cu, %cd, %cl and %cr specifiers
and will move a windows cursor either up, down, left or right.

Moving the cursor up or down will not move it outside the bounds
of the window but will scroll that window up one line if the
cursor was on the bottom line *unless* that window is scroll
locked in which case it will not move the cursor at all.

Moving the cursor left when it is already at the left edge of the
window will move the cursor to the far right column of the window
line above its current line unless it is on the top line already.

Moving the cursor right when it is already at the far right edge
of the window will move the cursor to the far left column of the
line below its current position unless it is already on the
bottom line in which case it will not move the cursor but can
scroll the window up one line if it is not scroll locked.

```c
static void wclear(void)           %0
```

This static function handles the %0 specifier which will clear a
window and position its cursor to the top left thereof.

```c
static void u_puts(void)           %s
```

This static function handles the %s specifier which will write
a string out to the window at its current cursor location.  It is
simply a wrapper for the above documented `uC_win_puts()` function.

```c
static void bold(void)             %B+ or %B-
```

This static function which handles both the %B+ and %B- specifiers
will either enable or disable the bold attribute for a window.

```c
static void uline(void)            %U+ or %U-
```

This static function which handles both the %U+ and %U- specifiers
will either enable or disable the underline attribute for a
window.

```c
static void rev(void)             %R+ or %R-
```

This static function which handles both the %R+ and %R- specifiers
will either enable or disable the reverse video attribute for a
window.

```c
static void e(void)                %e
```

This static function which handles the %e specifier will write an
EOL character to a window (a cr/lf).  If the windows cursor is on
the bottom line of the window then this call will place the cursor
on the left edge of the window and will scroll the window unless
it is scroll locked.

```c
static void star(void)             %*
```

This static function which handles the %* specifier will write a
specified character a specified number of times to the window at
its current cursor location.

Each character written will move the windows cursor to the right
which will potentially wrap round to the start of the next line
which could potentially scroll the window unless it is scroll
locked.

```c
static void specifier(void)
```

This static function executes one of the above handlers based on
a selection made from a `uC_switch_t` structure.

```c
API void uC_win_printf(uC_window_t *win, const char *format, ...)
```

This public API call works in a way that is similar to C's printf
function but none of C's printf specifiers are valid here.  The
format string can contain any number of specifiers as documented
above or can contain characters to be written directly to the
specified window.

Parameter grouping macros

Three macros are defined in `uCurses.h` to visually associate
parameters that logically belong together.  Using the C comma
operator to group them (e.g. (x, y)) would silently discard all
but the last value, so these macros expand to the correct number
of separate arguments instead.  They work equally well with
`uC_win_printf` format specifiers and with direct API calls.

```c
#define UC_XY(x, y)      (x), (y)    // position
#define UC_WH(w, h)      (w), (h)    // dimensions
#define UC_RGB(r, g, b)  (r), (g), (b)
```

Examples with `uC_win_printf`:

```c
uC_win_printf(win, "%@%rf",
    UC_XY(cx, cy),
    UC_RGB(255, 0, 128));
```

Examples with direct API calls:

```c
uC_win_open(UC_WH(80, 24));
uC_scr_open(UC_WH(width, height));
uC_win_cup(win, UC_XY(x, y));
uC_win_set_rgb_fg(win, UC_RGB(255, 0, 128));
uC_widget_vg_create(name,
    UC_WH(w, h),
    UC_XY(x, y),
    attrs);
```


### 10.10. `uC_winch.c`
This module handles the `SIGWINCH` signal within uCurses.  This signal is
sent to a process when the terminal window it is running within is
resized.  uCurses does not try to magically resize an existing interface.
Instead it records that a resize happened, allows application code to
detect that state, and expects the application to rebuild its layout for
the new terminal dimensions.

```c
static void winch_handler(int sig)
```

The signal handler installed by `init_winch()`.  It does the minimum
async-signal-safe work required here: ignore the signal argument
and set the global winch flag.

```c
void init_winch(void)
```

Internal initialization function called by `uCurses_init()`.  It
saves the previous `SIGWINCH` action when possible and installs the
uCurses handler unless the previous action explicitly ignored
`SIGWINCH`.

```c
void de_init_winch(void)
```

Internal shutdown function called by `uCurses_deInit()`.  It
restores the previous `SIGWINCH` action when one was saved, clears
the saved-state marker, and clears the pending winch flag.

```c
API bool uC_winch_pending(void)
```

Returns true when `SIGWINCH` has been received and not yet
acknowledged.

```c
API void uC_winch_ack(void)
```

Clears the pending winch flag.  Application code should call this
only when it has intentionally consumed the resize event.  The
resize hold helper also acknowledges the flag while waiting for
the resize stream to go quiet.

```c
API bool uC_winch_dispatch(void)
```

Checks for a pending `WINCH` event, acknowledges it if present, and
calls the registered user handler when one exists.  Returns true
when a pending event was dispatched and false otherwise.

```c
API void uC_register_winch(user_winch_t handler)
API void uC_deregister_winch(user_winch_t handler)
```

Register or deregister the optional user `WINCH` callback used by
`uC_winch_dispatch()`.  Only one callback is stored.


## 11. JSON
This extension adds a JSON parser allowing application developers to write
an almost complete description of their user interface in JSON.  These
sources will parse that description and construct the interface based on
elements found therein.   This is not exactly a conformant implementation
of the JSON specification but it works well for its intended purpose.

It is probably only second to the widgets sources in complexity and it
significantly increases the size of the library when it is included in the
build.  Everything handled herein can be performed programmatically,
independently of the JSON parser.


### 11.1. `json.c`
This source file implements the JSON parsers state machine which uses five
active states to parse the JSON source file.   There is also one inactive
state which indicates that the JSON state machine has not yet started
parsing anything.  These states are as follows...

- `JSON_STATE_NONE`
- `JSON_STATE_L_BRACE`
- `JSON_STATE_KEY`
- `JSON_STATE_VALUE`
- `JSON_STATE_R_BRACE`
- `JSON_STATE_DONE`

As the JSON text is parsed the text does not define the state.  Rather,
the state defines what the next character must be.  All parsing is done on
a space delimited token by token basis.

The initial active state is `JSON_STATE_L_BRACE` which expects the next
token parsed to be the left brace '{' char.  If it is then the state is
set to `JSON_STATE_KEY`.   If not... oops!

The handler for `JSON_STATE_KEY` expect to see one of several known tokens
which will either be a `key` token or an `object` token.  Only recognized
tokens are valid, unrecognized tokens will cause the state machine to
puke.

The following object tokens are valid

- `STRUCT_SCREEN`       * `STRUCT_WINDOW`      * `STRUCT_BACKDROP`
- `STRUCT_MENU_BAR`     * `STRUCT_PULLDOWN`    * `STRUCT_MENU_ITEM`
- `STRUCT_ATTRIBS`      * `STRUCT_B_ATTRIBS`   * `STRUCT_S_ATTRIBS`
- `STRUCT_D_ATTRIBS`    * `STRUCT_F_ATTRIBS`   * `STRUCT_FLAGS`
- `STRUCT_RGB_FG`       * `STRUCT_RGB_BG`      * `STRUCT_WINDOWS`
- `STRUCT_PULLDOWNS`    * `STRUCT_MENU_ITEMS`

And the following key tokens are valid...

- `KEY_FG`              * `KEY_BG`             * `KEY_GRAY_BG`
- `KEY_GRAY_FG`         * `KEY_RED`            * `KEY_GREEN`
- `KEY_BLUE`            * `KEY_XCO`            * `KEY_YCO`
- `KEY_WIDTH`           * `KEY_HEIGHT`         * `KEY_NAME`
- `KEY_FLAGS`           * `KEY_BORDER_TYPE`    * `KEY_VECTOR`
- `KEY_SHORTCUT`        * `KEY_FLAG`           * `KEY_BLANK`
- `KEY_ORDER`

All tokens are recognized by their 32 bit FNV-1 hash value.  No string
compare functions are called.

If the parsed token is recognized as a key then that specific key is
handled (see below) and the next state is set to `JSON_STATE_VALUE` where we
extract the value to be stored in the specified key.

Note: while the ':' character between a key and its associated value
is required there is no state ':'

If the parsed token is one of the recognized objects the state is reset
back to `JSON_STATE_L_BRACE`.

Almost every `JSON_STATE_L_BRACE` indicates that a new object and thereby
an associated C structure is being created.

For every object state a new object is allocated for its associated C
structure but key states do not create any new objects because they are
used to assign values to the current object.  A new JSON state is however
always created and object state structure for the previous state is pushed
onto a state stack.

Any time a new object or key token is parsed we check to see if there is a
comma on it.  If there is no comma the next state will instead be
`JSON_STATE_R_BRACE` which pops the previous state off the stack.

As key values are parsed we immediately set the specified item in their
parents C structure.  When an object is completed we store the associated
C structure within its parents C structure.  The structure types of both
specify how and where.

When all states have been popped off the stack we transition into
`JSON_STATE_DONE` and the state machine terminates.

This custom JSON parser was implemented rather than using an off the shelf
parser because it not only builds the user interface but also verifies its
validity.   It will not for example, allow you to define a screen
structure within a color attributes structure.

As stated above, some object states do not have an associated C structure
so parsing them creates a pseudo object.  Any keys specified within
these pseudo object will be assigned to this pseudo objects parent C
structure. I.E. The keys grandparent.

I did not implement a way to define arrays in this parser and I have also
allowed for values to be expressed as a percentage.  For example you can
set a window width equal to a percentage of its parent screens width.

```c
static void json_push(json_state_t *j)
```

This static function pushes the current state variables address
onto a stack so we can initiate a new state which has been passed
in as a pointer to a new state structure.

```c
void json_pop(void)
```

This function pops the previous state back off the state stack and
makes it once again the current state.

```c
__attribute__((noreturn)) void json_error(const char *s)
```

This function is the JSON state machines abort method.  Any errors
in the JSON source file is considered a fatal error here and the
application will be aborted with an error message which has been
passed in.

```c
void *json_alloc(uC_mem_zone_t zone, uint32_t size)
```

This function is used to allocate a user interface structure such
as a screen or a window etc which is to be populated with key
values and other sub structures defined in the JSON sources.

It is also used to allocate the JSON parsers state structures when
the state machine is transitioned to a new state.

```c
static void json_state_l_brace(void)
```

This static function handles the initial state of the JSON parser
and verifies that the next character parsed out of the sources is
the left brace '{' character.   If it is the expected character
then the state is set to `JSON_STATE_KEY`.  Otherwise it is an error
which is always fatal.

```c
void json_new_state_struct(int struct_size, int32_t struct_type)
```

This function is used to transition the JSON parsers state machine
to a new state.  It allocates and initializes a new state
structure and then calls a function which pushes the old state
structure onto a stack.  Finally it sets the current state equal
to the new state.

```c
static bool check_comma(void)
```

This static function verifies that the most recently parsed space
delimited token has a comma ',' as its last character.  As parsed
tokens are never compared directly but are hashed, the hash value
calculated for this token will be incorrect (because of the
comma).

If the token does have a comma then that comma is stripped off and
the tokens FNV hash value is recalculated.

This function then returns either true (there was a comma) or
false (there was no comma).   There being no comma is not always
an error.

```c
void json_state_r_brace(void)
```

This function verifies that the most recently parsed space
delimited token was the right brace '}' character.  Prior to this
test it must first strip any comma from the token if there was one
and must recalculate the tokens hash value with the comma removed.

It is assumed that the next state will be `JSON_STATE_DONE` but if
the current structure being populated is not the "one and only"
screen structure we populate that structures parent (the screen
has no parent as it is the root structure of the UI).

In this case we must then pop the previous state off the state
machines stack and if that state is not `NULL` then we set the new
state to either `JSON_STATE_KEY` or to `JSON_STATE_R_BRACE` depending
on whether original '}' token had a comma on it or not.

If, after popping the previous state off the stack results in a
`NULL` json_state variable then the above assumption is correct and
we have successfully parsed the entire JSON source file.

```c
static void run_state_machine(void)
```

This static function contains the main loop of the JSON parsers
state machine.  It parses the next space delimited token out of
the source JSON file, computes its hash and executes the function
associated with the current state which will verify that the token
which was parsed was the correct one and will advance the state
machine to the next appropriate state if it was.

If the token was incorrect for the current state then that is
always treated as a fatal error.

This function will repeat in a loop until the state handler which
was executed in the most recent iteration sets the state to
`JSON_STATE_DONE`.

```c
static void open_json_file(char *path)
```

This function will attempt to open and memory map a JSON source
file whose path name has been passed in.  Being unable to open
said file is a fatal error.

```c
static void parse_json_data(void)
```

This function simply runs the JSON parsers state machine on a
memory mapped instance of a JSON file.   If this file was valid
then *somewhere* within that parsing the library-internal screen
pointer will have been set.

If the screen was successfully created then fixup code is run on
the structures built during parsing so that the interface is
complete.

```c
void json_file_create_ui(char *path, fp_finder_t fp)
```

This function will run the state machine parser on a file which is
in the applications current directory.

The fp_finder_t function pointer passed in here is used by the
user interface builder functions to assign function address to
every pull-down menu item parsed by the state machine.

This function is generated by executing the libraries .so file
directly while specifying a file containing a list of function
names to be associated with menu items.   The library will at
this time generate sources which, when included in the
applications build will allow the JSON parser to create the
desired associations.

It is not possible for the JSON parser to know the address of a
function whose name has been parsed but by computing a hash value
of that name string and assigning that association at build time
it enables the JSON parser to make that association as it builds
the menu user interface.

```c
void json_mem_create_ui(char *json_data, int len, fp_finder_t fp)
```

This function will run the state machine parser on a file which
has been built into the applications executable.

The fp_finder_t function pointer passed in here is used by the
user interface builder functions to assign function address to
every pull-down menu item parsed by the state machine.

This function is generated by executing the libraries .so file
directly while specifying a file containing a list of function
names to be associated with menu items.   The library will at
this time generate sources which, when included in the
applications build will allow the JSON parser to define the
desired associations.

It is not possible for the JSON parser to know the address of a
function whose name has been parsed but by computing a hash value
of that name string and assigning that association at build time
it enables the JSON parser to make that association as it builds
the menu user interface.


### 11.2. `json_token.c`
The functions in this file are where this parser does its actual parsing.
Tokens are parsed out of the source JSON file based on white space.  All
tokens must therefore be space delimited.  This greatly simplified the
complexity of the parser!

The following functions are contained within this file...

```c
void strip_quotes(int16_t len)
```

All key values must be wrapped within double quotation marks.
These must be removed from the space delimited token so prior to
computing that tokens FNV hash value.

```c
static void refill(void)
```

This parser parses tokens out of the source JSON file one line at
a time.  When the current line of the source file has been parsed
this function will 'refill' which copies the next line of the
JSON file into a separate line buffer.

Blank lines are skipped here and overly long lines will cause this
function to puke.

```c
static void skip_white(void)
```

When parsing the next token out of the line buffer this function
is called to skip any leading white space prior to parsing that
token. If there is no more data within the line buffer to be
parsed then the refill function is called here.

This is also where comments are skipped.  JSON does not have a
line comment mechanism defined as part of its specification but
most custom parsers add extensions to allow them.  I will not
comment on how not being able to comment in JSON is DUMB!

```c
void json_de_tab(char *s, int len)
```

This function overwrites every tab character in the JSON source
file with a space.  Tabs are evil kthxbai!

```c
void token(void)
```

This function is where each space delimited token is parsed out of
the source JSON file.  These tokens are copied into a json_token[]
array within the json_vars structure.  There is therefore a max
length of any token.


### 11.3. `json_schema.c`
This source file replaces the older `json_key.c` implementation.  The
parent / child constraint rules that were previously spread across many
small object and key handler functions are now encoded in one schema
array.  The parser still recognizes the same JSON objects and scalar
keys, but dispatch is table driven instead of function driven.

Each parsed JSON key is hashed with the same FNV-1 hash used elsewhere in
uCurses.  `json_schema.c` then checks that hash against the current parser
context and either creates the next JSON state structure, consumes a
special key, or reports a fatal JSON error.


#### 11.3.1. Schema entries
```c
typedef struct
{
    int32_t  hash;
    uint32_t valid_parents;
    uint16_t child_type;
    uint16_t child_size;
    uint8_t  flags;
} json_schema_t;
```

This structure describes one JSON object or scalar key.  The `hash`
field is the FNV-1 hash of the token name.  `valid_parents` is a
bitmask of parent `STRUCT_*` values where that key is legal.  A zero
parent mask means the entry is valid only at the root of the JSON
document.

`child_type` is the `STRUCT_*` or `KEY_*` state type to create, and
`child_size` is the number of bytes to allocate for the associated
structure.  Pseudo objects and scalar keys use a zero child size.

`SF_OBJ` marks an entry as an object key.  Object keys transition the parser
to `JSON_STATE_L_BRACE` after the new state is created.

`SF_ROOT` marks the root screen entry.  The screen must be the first object
in the JSON document, only one screen is allowed, and creating it
also initializes `active_screen` and the screen dimensions.

`SF_SKIP` marks an entry that may be consumed without applying a value in
specific contexts.  The current use is the `order` key inside a
backdrop.  A throwaway state is pushed so `json_state_value()` can
pop the state stack normally.

`SF_BREAK` is a debug helper entry.  It calls `uC_noop()`, creates no new object,
and consumes the following scalar value.

```c
static const json_schema_t schema[]
```

This table contains the complete key / object schema.  Object
entries are listed before scalar entries so ambiguous keys such as
`flags` can resolve based on the current parent context.

The table currently recognizes `screen`, `windows`, `window`, `backdrop`,
`menu_bar`, `pulldowns`, `pulldown`, `menu_items`, `menu_item`, the
various attribute containers, RGB color containers, `flags`, color scalar
keys, geometry keys, `name`, `border_type`, `vector`, `shortcut`, `flag`,
`order`, `blank`, and the debug breakpoint key.


#### 11.3.2. Schema helpers
```c
PM(x)
```

Builds a parent bit for a `STRUCT_*` value.

```c
PM_ATTRIB_TYPES
PM_ATTRIB_HOSTS
```

Convenience masks for the groups of attribute container types and
object types that may contain attributes.

```c
static const json_schema_t *find_schema(int32_t hash, uint32_t pmask)
```

Scans `schema[]` for the first row whose hash matches the parsed key
and whose valid parent mask accepts the current parser context.
Root-only entries match only when the parser is at the root.
`SF_BREAK` entries match regardless of parent.

```c
static void must_quote(int16_t len)
```

Verifies that JSON key tokens are wrapped in double quotes.

```c
static void check_colon(void)
```

Reads the token following a key and verifies that it is the colon
separator.


#### 11.3.3. Key dispatch
```c
static void json_schema_dispatch(void)
```

Converts the current parser context into a parent mask, finds the
matching schema row, and performs the state transition described by
that row.

Root entries create and initialize the active screen.  Skipped
entries create a balanced throwaway state.  Normal object entries
create a child state and transition to `JSON_STATE_L_BRACE`.  Normal
scalar key entries create a child state and transition to
`JSON_STATE_VALUE`.

```c
void json_state_key(void)
```

This function is called when the parser is in `JSON_STATE_KEY`.  It
handles the special case where a closing brace is read, verifies and
strips key quotes, dispatches through `schema[]`, and verifies the colon
following the key token.


### 11.4. `json_value.c`
After a scalar key token has been matched by `json_schema.c` the JSON state
is transitioned into `JSON_STATE_VALUE`.  These functions handle each value
type and set the appropriate item within the associated object structure
held by the JSON state machine.

```c
static void value_fgbg(void)
```

This function is called to set either a foreground or a background
color within one of the various `uC_attribs_t` structures.  These
'various' structures are all used to assign attributes to
different elements within the user interface.

```c
static void value_gray_fgbg(void)
```

This function is called to set either a gray scale foreground or a
gray scale background within one of the various `uC_attrib_t`
structures.  These 'various' structures are used to assign
attributes to different elements within the user interface.

```c
static void value_rgb_fg(uC_attribs_t *gstruct)
```

This function is called to set one element of a RGB foreground
color one of the various `uC_attribs_t` structures. These 'various'
structures are used to assign attributes to different elements
within the user interface.

The *gstruct attributes structure passed in here is the keys
grandparent because its parent is a pseudo structure.  The key
will be one of the following items,

- `KEY_RED`
- `KEY_GREEN`
- `KEY_BLUE`

```c
static void value_rgb_bg(uC_attribs_t *gstruct)
```

This function is called to set one element of a RGB background
color one of the various `uC_attribs_t` structures. These 'various'
structures are used to assign attributes to different elements
within the user interface.

The *gstruct attributes structure passed in here is the keys
grandparent because its parent is a pseudo structure.  The key
will be one of the following items,

- `KEY_RED`
- `KEY_GREEN`
- `KEY_BLUE`

```c
static void value_rgb(void)
```

This function is used to set either a foreground attribute or a
background attribute to one element of a RGB color.  The attribute
will be set in the keys grandparent object structure and the value
assigned must be in the range 0 to 255.

```c
static void value_xy(void)
```

This function is used to set either the X coordinate or the Y
coordinate of a window structure within its parent screen.  The
coordinates can be specified as a percentage of the screens
overall dimensions.

```c
static void value_wh(void)
```

This function is used to set either the width or the height of a
window structure.  The dimensions may be expressed as a percentage
of the parent screens overall dimensions.

```c
static void value_name(void)
```

This function will set the name string of the keys parent object
structure which will be one of the following object structures.

- `STRUCT_WINDOW`
- `STRUCT_MENU_ITEM`
- `STRUCT_PULLDOWN`

In the case of a window this function sets both its name and its
display name.  The name of a window structure is a hash value of
its display name.

```c
static void val_m_item_flag(menu_item_t *item)
```

This function will set the flags element of a menu structure.
Only one flag type is supported which is `uC_MENU_DISABLED`.

```c
static void val_pd_flag(pulldown_t *pd)
```

This function will set the flags element of a pulldown menu
structure.  Only one flag type is supported which is
`uC_MENU_DISABLED`.

```c
static void val_win_flag(uC_window_t *win)
```

This function will set the flags element of a window structure.
If the flags being set contains a `uC_WIN_BOXED` then the windows
width value is flagged as needing to be adjusted once the JSON
parser has completed building the user interface.

```c
static void value_flag(void)
```

This function will set a flags element of the grand parent
object structure of the key.  The grand parent will be one of

- `STRUCT_MENU_ITEM`
- `STRUCT_PULLDOWN`
- `STRUCT_WINDOW`

```c
static void value_border_type(void)
```

This function will give a window structure a border which may be
one of the following border types.

- `uC_BDR_SINGLE`
- `uC_BDR_DOUBLE`
- `uC_BDR_CURVED`

```c
static void value_blank(void)
```

This function sets the blank character for a window which is by
default the space character.  When the window is cleared it will
be filled with this character which may be any UTF-8 codepoint.

```c
static void value_order(void)
```

This function sets the tab selection order of a window.

```c
static void value_vector(void)
```

This function sets a pulldown menu items function vector address.
This address is set by calling a fp finder function and passing it
a hash of the name of the function whose address we need.

This fp finder call is generated by executing the libraries .so
file directly and passing it a file containing the names of every
function we need the address of at run time.  The .so file will
output sources to be added to your build which associate each name
hash value with its associated function.

The function supplied by the .so file will return a menu functions
address given that functions name hash.

```c
static void value_shortcut(void)
```

This function sets a pulldown menu item's shortcut key.  The JSON
value must be quoted and non-empty; the first character in that
quoted value is stored as the menu item's shortcut.  The menu item
creation path later registers that shortcut with the screen-level
shortcut registry when the item is enabled.

```c
static const uC_switch_t value_types[] =
```

This `uC_switch_t` array associates each known key type with one of
the key handling functions documented above.

```c
static int32_t constant_hash[]
```

This array contains a list of hash values for known named
constants which can be added to the applications JSON data.  Each
of these hash values is associated with a constant value to be
assigned to a key when parsing the JSON data.

```c
static int32_t constant_val[]
```

This array contains a list of constant values in the same order as
the above constant key hash values.

```c
static void parse_number(void)
```

This function will parse a numerical value out of the JSON data
which may be expressed in hexadecimal by prefixing the values
string with the usual 0x prefix.

It can also be expressed as a percentage of some other value based
on JSON context.  For example, windows can be sized and located
based on a percentage of the screen size.

Values are assumed to be expressed in decimal if the 0x prefix
is not specified.

```c
static void is_constant(void)
```

This function will set the current key value to one of the known
named constants values.

```c
static bool chk_quotes(uint16_t len)
```

This function verifies that the most recently parsed JSON token is
quoted.  If it is then those quotes are stripped and a true result
is returned.  Otherwise a false result is returned.

```c
void json_state_value(void)
```

This function is called by the JSON state machine when the current
JSON state is `STATE_VALUE`.   It initializes the current key value
to a special magic "not a number" value and then evaluates the
JSON data and overrides that `NAN` with an actual value.

It first tests to see if the value token is a known named constant
and if, after doing this the key value is still `NAN` it will then
attempt to interpret the value token as a numerical string.

It must be one or other of these!

It then `uC_switch_t` selects a function based on the key name so
the function which knows how to set the specified key to the
specified value will be executed.

At this time, if the JSON state stack is not empty the previous
JSON state is popped off the state stack which will be one of the
following states.

- `JSON_STATE_KEY`
- `JSON_STATE_R_BRACE`

If the JSON state stack was empty here that is an error condition
that I do not actually believe is possible (but then again, this
IS C!).


### 11.5. `json_populate.c`
As the JSON sources are parsed and the state machine is executed, numerous
object structures will be created and populated with key values.  However,
when an object structure is defined as part of some other parent object
structure the state machine will not make that assignment.

Every object other than the root screen object will have a parent object
and this function will populate each parent object with whatever child
objects were defined within it.

This function is called by the JSON state machine for each object
structure except the screen (which has no parent) and populates that
objects parent with that object.

```c
static void populate_attribs(void *pstruct, int32_t ptype)
```

This function will populate a parent structure with its child
`uC_attribs_t` object.  The parent object will be one of the
following...

- `STRUCT_WINDOW`
- `STRUCT_BACKDROP`
- `STRUCT_PULLDOWN`
- `STRUCT_MENU_BAR`

When the parent object has been populated with the child object
the child objects JSON state structure is freed.

```c
static void populate_b_attribs(uC_window_t *pstruct)
```

This function populates a parent window structure with its child
border attributes structure.  Once this is completed the child's
JSON state structure is freed.

```c
static void populate_s_attribs(void *pstruct, int32_t ptype)
```

This function will populate a parent structures selected
attributes with its child attributes structure.   The parent
structure will be one of the following types..

- `STRUCT_PULLDOWN`
- `STRUCT_MENU_BAR`

Once completed the child structures JSON state structure is freed.

```c
static void populate_d_attribs(void *pstruct, int32_t ptype)
```

This function will populate a parent structures disabled
attributes with its child attributes structure.  The parent
structure will be one of the following types.

- `STRUCT_PULLDOWN`
- `STRUCT_MENU_BAR`

Once completed the child structures JSON state structure is freed.

```c
static void populate_pulldown(menu_bar_t *pstruct)
```

This function will populate a parent menu bar structure with
its child pulldown menu structure.  The child is added to the
parents array of pulldown menus and the parents item count is
incremented.

```c
static void populate_menu_item(pulldown_t *gstruct)
```

This function will populate a grandparent pulldown menu structure
with its grandchild pulldown menu item structure.  The grandchild
is added to the grandparents array of menu items and its item
count is incremented.

```c
static void populate_window(json_state_t *parent)
```

This function populates the root screen structure with a child
window structure.   The `uC_scr_win_attach()` API is called to
add the child window to its parent screen.

```c
static void populate_backdrop(uC_screen_t *pstruct)
```

This function populates the root screen structure with a child
backdrop window structure.

```c
static void populate_bar(uC_screen_t *scr)
```

This function populates the root screen structure with a child
menu bar structure.

```c
void populate_parent(void)
```

For each object structure created by the JSON state machine this
function is called to add that object structure to its parent
object structure using one of the above documented functions.


### 11.6. `json_build.c`
This module will build the applications user interface, allocating various
structures and fixing values which could not be known while the state
machine built the user interface objects until all the objects had been
created.

At this point all objects have been created and we can finalize the
initialization of the applications user interface.

```c
static void bounds_check(uC_window_t *win)
```

When a window object is created by the JSON state machine it will
have its position and dimensions specified in the JSON data.  This
function verifies that those values are valid and will puke if
they are not.

When checking a windows coordinates against the known width and
height of the console (and thus the width and height which will
be taken up by the screen) we must account for any borders that
have been assigned to that window.

If there is a border then the window is actually two characters
taller and two characters wider that what is specified in that
windows dimensions.

If any part of the window, including its borders would be situated
outside the bounds of its parent screen structure then this is a
critical error.

```c
static void fix_win(uC_screen_t *scr, uC_window_t *win)
```

Window positional coordinates can be expressed as "`uC_WIN_FAR`" in the
JSON data.  This means the window is meant to be situated either
as far right or as far down the screen as it can go and still fit
within the bounds of that screen.

This function moves a window thusly defined into its requested
position within the screen.

```c
static void fix_windows(uC_screen_t *scr)
```

This function performs a "fixup" procedure on all windows attached
to their parent screen structure.

If the screen has a backdrop window then the backdrop structure
will need to be finalized by setting its width, height and
position within the screen.  The windows backing store will also
need to be allocated.

This code will then scan through the active screens list of
attached windows and will "fix" that window using the above
documented functions, will verify that its position is valid,
will allocate its backing store, set its blank character to
a space (which it might not be supposed to do?).

```c
static void fix_menus(uC_screen_t *scr)
```

This function will perform a fixup procedure on any menu bar that
is attached to the specified screen.

It will open a window for that menu bar, set that window as
scroll locked, reset the child pulldown menus X coordinate two
two (where the first pulldown menu item will be positioned) and
set its blank character.

For each pulldown menu associated with that menu bar this function
will then assign that pulldown menus X coordinate, adjust the
X coordinate for the next pulldown based on this pulldown menus
name length then set the pulldown menus width equal to the width
of the widest menu items name contained therein.

```c
void json_build_ui(void)
```

This function is called as the final operation of the JSON state
machine, once the `JSON_STATE_DONE` state is reached.

It allocates the backing store for the newly created screen
structure and fixes all the windows and any menu bar attached to
that screen using the above functions.

At this time the entire user interface has been defined.

Job complete!


## 12. Function coverage appendix
This appendix covers small helper functions that are not already described
by name in the main source-file sections above.  It is intentionally short:
the goal is to identify what each function is for and what important side
effect it has, not to restate the source code.


### `src/json/json.c`
`json_state_machine()` drives the parser state machine until the JSON
input is consumed or a terminal parser state is reached.  It dispatches
token handling according to the current JSON parser state.


### `src/json/json_populate.c`
`copy_attribs_to()` copies the currently parsed attribute values into a
destination `uC_attribs_t`.  It is the local helper used when a JSON object
has completed an attribute description.


### `src/json/json_value.c`
`value_attrib()` interprets one attribute value from JSON and stores it in
the active parser target.

`value_window_dim()` interprets a window dimension value.  It accepts the
numeric case and the special window-dimension tokens used by the JSON UI
description.


### `src/keys/key_sequence.c`
`kLFT()` emits the terminal capability for shifted cursor-left.

`kRIT()` emits the terminal capability for shifted cursor-right.


### `src/keys/uC_key_table.c`
`k_sleft()` maps the shifted left-arrow escape sequence onto
`UC_KEY_SLEFT`.

`k_sright()` maps the shifted right-arrow escape sequence onto
`UC_KEY_SRIGHT`.

`uC_restore_default_key_action()` restores one default key action only if
the current slot still contains the expected action.  This keeps feature
cleanup from trampling a later override.

`uC_restore_key_action()` performs the same guarded restore operation for
the currently active key handler table.

`key_context()` captures the popup/widget-popup state used by the
consolidated `uC_key()` dispatcher.

`key_shortcuts_blocked()` reports whether global shortcuts should be
blocked.  Raw informational popups and active textbox editing both block
shortcut dispatch.

`key_shortcut_run()` gives the shortcut registry a chance to consume a
key.  Popup widget groups use the popup-only shortcut path.

`key_menu_handle()` gives the menu system a chance to handle a key when a
popup is not owning the interface.

`key_widget_handle()` gives the widget system a chance to handle a key
when widgets are active and no raw-only popup is blocking them.


### `src/keys/uC_shortcuts.c`
`shortcut_key()` extracts the base key from a shortcut value.

`shortcut_alpha()` tests whether a key is alphabetic.

`shortcut_upper()` converts an alphabetic key to upper case.

`shortcut_normalize()` normalizes plain alphabetic shortcuts so upper and
lower case compare consistently.

`shortcut_plain_match()` compares a plain shortcut key against an input
key, including the case-folding rule for alphabetic shortcuts.

`shortcut_ctrl_code()` converts a control-key input byte into the matching
alphabetic key when possible.

`uC_shortcut_matches()` compares a registered shortcut against a key
returned by the keyboard input path.


### `src/terminfo/uC_attribs.c`
`uC_set_fg()` sets the foreground color field of an attribute object.

`uC_set_bg()` sets the background color field of an attribute object.

`uC_set_gray_fg()` sets a grayscale foreground value.

`uC_set_gray_bg()` sets a grayscale background value.

`uC_set_rgb_fg()` sets a full RGB foreground color.

`uC_set_rgb_bg()` sets a full RGB background color.


### `src/terminfo/uC_terminfo.c`
`ti_set_screen()` binds terminfo output to the active screen.  Terminal
cursor tracking is screen-relative, so terminfo needs this pointer before
emitting cursor movement.


### `src/terminfo/uC_ti_parse.c`
`_div()` implements the division operation used by the terminfo format
string evaluator.

`fs_reset()` clears the scratch format stack before parsing one terminfo
format string.

`fs_drop_leftovers()` discards unused scratch stack values before returning
from one terminfo format string.


### `src/ui/menus/uC_menu_bar.c`
`pd_label_width()` returns the widest visible label width in a pulldown.
The menu renderer uses this to align shortcut text.

`draw_item_shortcut()` draws one menu item's shortcut in the aligned
shortcut column.


### `src/ui/menus/uC_menu_item.c`
`menu_shortcut_action()` is the shortcut registry callback for menu items.
It invokes the associated menu item's action when the item is still live.

`menu_item_shortcut_register()` registers a menu item shortcut against the
screen that owns its pulldown.

`menu_item_shortcut_remove()` removes a menu item's registered shortcut.

`menu_pd_shortcuts_register()` registers every shortcut owned by a
pulldown.

`menu_pd_shortcuts_remove()` removes every shortcut owned by a pulldown.


### `src/ui/menus/uC_menu_key.c`
`menu_set_screen()` stores the screen currently being driven by menu key
handling.

`pd_enabled()` reports whether a pulldown exists and is not disabled.

`item_enabled()` reports whether a menu item exists and is not disabled.

`menu_key_return()` converts the internal no-key marker into the public
zero return value expected by menu callers.

`select_enabled_pd()` advances to an enabled pulldown, wrapping as needed
and skipping disabled pulldowns.

`select_enabled_item()` advances to an enabled item within a pulldown,
wrapping as needed and skipping disabled menu items.

`menu_normalize_selection()` repairs the selected menu and item after menu
entries are disabled or enabled.  It also closes an invalid open pulldown.

`menu_action_key()` reads the key produced by an activated menu item and
normalizes the menu return value.

`menu_key_f10()` injects `UC_KEY_F10` into the key buffer for the menu
handler.

`uC_menu_deinit_keys()` restores key handlers that were temporarily
installed by the menu system.


### `src/ui/menus/uC_menu_shortcut.c`
The local `shortcut_key()` extracts the base key from a menu shortcut.

The local `shortcut_upper()` converts a menu shortcut key to upper case
for display.

`shortcut_base_display()` formats the base key portion of a menu shortcut.

`uC_menu_shortcut_display()` formats a full menu shortcut, including
supported modifier prefixes.

`uC_menu_shortcut_width()` returns the display width needed for a menu
shortcut string.

`uC_menu_shortcut_matches()` tests whether a key activates a menu
shortcut.


### `src/ui/uC_border.c`
`uC_window_clear_box()` clears a rectangular area inside a window.  It is
used by border and box drawing paths that need to erase an interior before
redrawing.


### `src/ui/uC_screen_draw.c`
`scr_is_too_small()` reports whether a screen is smaller than its declared
minimum dimensions.

`too_small_write()` writes clipped text into the automatic "Too small"
popup without changing the window's permanent attributes.

`text_len_i16()` returns a small signed length for literal UI text used by
the popup layout code.

`too_small_popup_prepare()` creates or refreshes the automatic "Too small"
popup window.

`scr_draw_too_small_popup()` draws the automatic size-warning popup or
tears it down once the screen is large enough again.

`scr_cell_width()` returns the display width of a composed screen cell.
The `DEADC0DE` sentinel is treated as a continuation cell with zero
standalone width.

`scr_blank_cell()` replaces a composed screen cell with a space.

`scr_normalize_wide_row()` cleans one composed screen row so wide glyphs
cannot leave orphaned `DEADC0DE` continuation cells or partial wide
characters after overlapping windows have been painted.

`monotonic_ns()` returns a monotonic timestamp in nanoseconds for draw
timing.

`resize_fill_shadow_view()` builds a shadow copy of a screen view during
resize handling.

`resize_draw_shadow()` draws the resize shadow view used to avoid stale
terminal contents after a `WINCH`.


### `src/ui/uC_window_draw.c`
`win_cell_width()` returns the display width of one window cell and treats
`DEADC0DE` as a continuation marker.

`win_blank_cell()` fills a window cell with the window's current blank
character and attributes.

`win_blank_range()` blanks a half-open range of cells in one row.

`win_wide_owner_at()` finds the owning cell of a wide glyph that covers a
given column.

`win_clear_wide_overlap()` blanks any existing wide glyph that overlaps a
new write range.

`win_normalize_wide_row()` removes invalid partial wide glyphs from a
window row after clipping or horizontal motion.

`win_copy_line()` copies one line of a window backing store to another
line in the same window.

`uC_win_set_cx()` sets a window's current X cursor position if the value
is inside the window.

`uC_win_set_cy()` sets a window's current Y cursor position if the value
is inside the window.

`uC_win_crsr_up()` moves the window cursor up one row when possible.

`uC_win_crsr_dn()` moves the window cursor down one row when possible.

`uC_win_crsr_lt()` moves the window cursor left one column when possible.

`uC_win_crsr_rt()` moves the window cursor right one column when possible.

`uC_win_cr()` performs a carriage return inside a window by moving the
window cursor to column zero.


### `src/ui/widgets/uC_widget.c`
`auto_sequence()` returns the next automatically assigned widget sequence
number.

`sync_seq()` advances the automatic sequence counter past an explicitly
assigned sequence number.

`uC_widget_reset_sequence()` resets automatic widget sequence assignment.

`widget_clear_focus()` removes focus from a widget and clears the global
widget focus state when that widget was active.

`widget_detach_widget()` removes a widget from its view, unregisters its
shortcut and clears focus if needed.


### `src/ui/widgets/uC_widget_button.c`
`button_letter_alpha()` tests whether a candidate button shortcut
character is alphabetic.

`button_letter_upper()` converts a button shortcut character to upper
case.

`button_letter_from_name()` chooses a usable button shortcut character
from a requested letter or from the button name.


### `src/ui/widgets/uC_widget_draw.c`
`widget_set_attrs()` selects the attributes used to draw a widget,
including focus and inactive-view styling.

`draw_widget_view_group()` draws all views and widgets contained in one
view group window.

`draw_popup_view_group()` draws a popup view group and then paints its
window onto the screen as the topmost popup layer.


### `src/ui/widgets/uC_widget_keys.c`
`uC_widget_reset_state()` clears the global widget input state and resets
automatic widget sequencing.

`widget_key_f10()` injects the widget-loop `F10` marker for the legacy
widget key path.

`uC_widget_current_sequence()` returns the sequence number of the
currently selected widget.

`widget_input_active()` reports whether a screen currently has an active
widget context that can receive keys.


### `src/ui/widgets/uC_widget_mouse.c`
`widget_activate_mouse_hit()` activates the widget identified by a mouse
hit and returns the key/action produced by that activation.

`widget_hit_test()` maps a screen coordinate to a widget hit record when
the coordinate lies inside an active widget.

`uC_widget_mouse_handle()` handles one decoded mouse event for the widget
system.


### `src/ui/widgets/uC_widget_scan.c`
`scan_tab_candidate_in_vg()` scans a view group once to find the next or
previous selectable widget sequence, including the wrap candidate.

`tab_candidate()` chooses the next or previous sequence for tab
navigation.  Sparse sequence numbers are supported without probing every
integer in the gap.


### `src/ui/widgets/uC_widget_view.c`
`uC_widget_to_view_index()` moves a scrollable view to a specific item
index.

`uC_widget_view_set_position()` changes a view's position inside its
parent view group.

`uC_widget_view_index()` returns the current top item index of a
scrollable view.


### `src/ui/widgets/uC_widget_view_group.c`
`widget_shortcut_attached_active()` checks whether a widget shortcut still
belongs to an attached, active widget group.

`widget_shortcut_action()` is the shortcut registry callback for button
widgets.  It selects the button and injects the button's action key.

`widget_register_shortcuts()` registers button shortcuts for all widgets
inside a view group.

`widget_remove_shortcuts()` unregisters shortcuts owned by all widgets in
a view group.

`widget_set_vg_inactive()` sets or clears the inactive flag on a view
group.

`widget_modal_deactivate_others()` marks non-popup view groups inactive
while a widget popup owns the interface.

`widget_modal_restore_others()` restores view-group inactive state after
the widget popup is closed.

`widget_clear_focus_for_vg()` clears global widget focus when the focused
widget belongs to a specified view group.

`widget_vg_release_window()` releases the backing store owned by a view
group window.

`widget_view_fits_group()` validates that a view fits completely inside
its parent view group.


### `src/utils/uC_win_printf.c`
`do_win_printf()` executes one parsed `uC_win_printf()` format operation.


## 13. Final words
This library has been in semi active development for a number of years.
It is far from complete, has known issues (with certain terminal types)
and is significantly larger than I intended it to be (and will probably
still grow some more).

I am in no way, shape or form an expert at developing for *nix terminals
and if I am totally honest I must admit to a certain level of ABSOLUTE
FREEKING INSANITY for even attempting this.

I do not know if this code is safe and as it currently stands I am certain
it is neither as consistent nor as reliable as ncurses (tyvm TED!).

BUT!

It actually performs amazingly well considering and was a joyful challenge
to implement.

Writing this document however was certainly not fun and I am certain there
is VAST room for improvements herein.  It is also only half the job so
there is a lot more documentation incoming (soon ish maybe kinda).

If you read something here that makes absolutely no sense to you or is
known by you to be utterly incorrect I can be contacted via email or
through the projects github page which is listed on this documents cover
sheet at the top.

My email is mark4th at 'g' you know where dot com.

---
