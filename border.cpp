#include "border.h"
#include "alleg.h"

static RGB rgb;

void Border (int r, int g, int b)
{
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  set_color (0, &rgb);
}

void GetBorder (int &r, int &g, int &b)
{
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
}

void Border ()
{
  Border (255, 0, 255);
}
