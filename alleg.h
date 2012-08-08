#ifndef __MANIPULATED_ALLEGRO_H__
#define __MANIPULATED_ALLEGRO_H__

#undef BITMAP



#include <allegro.h>

#undef MAX
#undef MIN

template <class T> inline T MAX (T x1, T x2)
{
  if (x1 > x2)
    return x1;
  else
    return x2;
}

template <class T> inline T MIN (T x1, T x2)
{
  if (x1 < x2)
    return x1;
  else
    return x2;
}

#endif
