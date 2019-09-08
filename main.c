
#include <inttypes.h>
#include <stdio.h>

#include "h/color.h"
#include "h/uCurses.h"

int main(void)
{
  uCurses_init();      printf("\r\n\r\n");

  set_fg(WHITE);       printf("This is normal text\r\n");
  set_fg(BLUE);        printf("This is blue text\r\n");
  set_fg(WHITE);
  set_bold();          printf("This is bold text\r\n");
  set_fg(BLUE);        printf("This is bold blue text\r\n");
  set_fg(WHITE);
  clr_bold();
  set_rev();           printf("This is reverse text\r\n");
  clr_rev();
  set_ul();            printf("This is underlined text\r\n");
  clr_ul();
  set_blink();         printf("This is sometimes blinking text\r\n");
  clr_blink();
  set_dim();           printf("This is dim text\r\n");
  clr_dim();
  set_acs();           printf("qwertyuiopasdfghjklzxcvbnm\r\n");
  clr_acs();
  set_norm();

  return 0;
}
