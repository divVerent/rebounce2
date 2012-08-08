#include "border.h"
#include "alleg.h"

void Border (int r, int g, int b)
{
  static RGB rgb;
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  set_color (0, &rgb);
}

void GetBorder (int &r, int &g, int &b)
{
  static RGB rgb;
  get_color (0, &rgb);
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
}

void Border ()
{
  Border (255, 0, 255);
}
