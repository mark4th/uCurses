// rgb_hsl.c
// -----------------------------------------------------------------------

// used to calculate contrasting RGB colors for a given set of RGB colors
// not perfect, sometimes does not produce good contrasting colors

// -----------------------------------------------------------------------

#include "uCurses.h"

// -----------------------------------------------------------------------

static inline double dmax(double a, double b)  { return (a > b) ? a : b; }
static inline double dmin(double a, double b)  { return (a < b) ? a : b; }

// -----------------------------------------------------------------------
// Convert RGB color to HSL.

static void rgb2hsl(double r, double g, double b, double *h,
    double *s, double *l)
{
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;

    double max = dmax(dmax(r, g), b);
    double min = dmin(dmin(r, g), b);

    *h = *s = *l = (max + min) / 2;

    if (max == min)
    {
        *h = 0;
    }
    else
    {
        double d = (max - min);

        *s = (*l > 0.5)
            ? d / (2 - d)
            : d / (max + min);

        if (max == r)
        {
            *h = (g - b) / d + (g < b ? 6 : 0);
        }
        else if (max == g)
        {
            *h = (b - r) / d + 2;
        }
        else
        {
            *h = (r - g) / d + 4;
        }

        *h /= 6;
    }
}

// -----------------------------------------------------------------------
// Converts an HUE to r, g or b.

static double hue2rgb(double p, double q, double t)
{
    if (t < 0)    t += 1;
    if (t > 1)    t -= 1;

    if (t < 1.0 / 6)
    {
        return p + (q - p) * 6 * t;
    }

    if (t < 1.0 / 2)  return q;

    if (t < 2.0 / 3)
    {
        return p + (q - p) * (2.0 / 3 - t) * 6;
    }

    return p;
}

// -----------------------------------------------------------------------
// Converts HSL to RGB

static void hsl2rgb(double h, double s, double l, double *r, double *g, double *b)
{
    if(0 == s)
    {
        *r = *g = *b = l; // achromatic
    }
    else
    {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;

        *r = hue2rgb(p, q, h + 1.0 / 3) * 255.0;
        *g = hue2rgb(p, q, h)           * 255.0;
        *b = hue2rgb(p, q, h - 1.0 / 3) * 255.0;
    }
}

// -----------------------------------------------------------------------

API void make_contrast(uint8_t *r, uint8_t *g, uint8_t *b)
{
   double h, s, l;

   double R = (double) *r;
   double G = (double) *g;
   double B = (double) *b;

   rgb2hsl(R, G, B, &h, &s, &l);

   h += 0.5;
   if (h > 1) h -= 1;

   hsl2rgb(h, s, l, &R, &G, &B);
   *r = (uint8_t)R;
   *g = (uint8_t)G;
   *b = (uint8_t)B;
}

// =======================================================================
