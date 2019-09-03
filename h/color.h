// color.h
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

typedef enum
{
  BLACK, RED, GREEN, YELLOW,
  BLUE, MAGENTA, CYAN, WHITE,
} ti_color_t;

// -----------------------------------------------------------------------

typedef enum
{
  TI_STANDOUT, TI_UNDERLINE, TI_REVERSE, TI_BLINK,
  TI_DIM, TI_BOLD, TI_BLANK, TI_PROT, TI_ACS
} ti_attrib_t;

// -----------------------------------------------------------------------

#define STANDOUT  (1 << TI_STANDOUT)
#define UNDERLINE (1 << TI_UNDERLINE)
#define REVERSE   (1 << TI_REVERSE)
#define BOLD      (1 << TI_BOLD)
#define BLINK     (1 << TI_BLINK)
#define DIM       (1 << TI_DIM)
#define ACS       (1 << TI_ACS)

// -----------------------------------------------------------------------

void set_so(void);          // set standout
void set_ul(void);          // set underline
void set_rev(void);         // set reverse video
void set_bold(void);        // set bold
void set_blink(void);       // set blinking (sometimes)
void set_dim(void);
void set_acs(void);

void clr_so(void);          // clear ...
void clr_ul(void);
void clr_rev(void);
void clr_bold(void);
void clr_blink(void);
void clr_dim(void);
void clr_acs(void);

void set_norm(void);        // clear all atrributes etc

void set_bg(uint8_t c);     // set background/foreground colors
void set_fg(uint8_t c);

// =======================================================================
