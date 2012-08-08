#ifndef __RP__COLLSION_H__
#define __RP__COLLSION_H__

#include "defs.h"
#include <algorithm>
#include "map.h"

namespace Collision
{
  inline void get_map_range (int x, int y, int w, int h, int grid_w, int grid_h, int *xmap, int *ymap, int *x2map, int *y2map)
  {
    *xmap = x / grid_w;
    *ymap = y / grid_h;
    *x2map = (x + w - 1) / grid_w;
    *y2map = (y + h - 1) / grid_h;
  }
  inline bool check_box_coll (int x, int y, int w, int h, int x2, int y2, int w2, int h2)
  {
    // b above b2
    if (y + h <= y2)
      return 0;
    // b2 above b
    if (y2 + h2 <= y)
      return 0;
    // b left to b2
    if (x + w <= x2)
      return 0;
    // b2 left to b
    if (x2 + w2 <= x)
      return 0;
    // or they touch
    return 1;
  }
  template <class C> class _iter_func
  {
    C iter;
   public:
    explicit _iter_func (const C &it) : iter(it)
    {
    }
    void operator () (const typename C::container_type::value_type &val)
    {
      *(iter++) = val;
    }
  };
  template <class C> _iter_func<C> iter_func (const C &iter)
  {
    return _iter_func<C>(iter);
  }

  template <class C, class F> void for_each (const C& cont, F f, int x, int y, int w, int h)
  {
    for (typename C::const_iterator p = cont.begin(); p != cont.end(); ++p)
      if (check_box_coll (x, y, w, h, (*p)->x, (*p)->y, (*p)->w, (*p)->h))
	f (*p);
  }
  template <class C, class C2> void FindCollidingEntities (int x, int y, int w, int h, const C &in, C2 &out)
  {
    out.clear ();
    for_each (in, iter_func(back_inserter(out)), x, y, w, h);
  }
  template <class F> class ref_func
  {
    F f;
   public:
    ref_func (const F &_f) : f(_f) { }
    template <class I> I operator () (I &item)
    {
      f (item);
      return item;
    }
  };
  template <class C, class F> void for_each (C& cont, F f)
  {
    std::transform (cont.begin(), cont.end(), cont.begin(), ref_func<F>(f));
  }
  inline int RectFlags (::Level::Map &current, int x, int y, int w, int h)
  {
    int flags = 0;
    int xm, ym, x2m, y2m;
    get_map_range (x, y, w, h, SPRWIDTH, SPRHEIGHT, &xm, &ym, &x2m, &y2m);
    for (int yy = ym; yy <= y2m; ++yy)
      for (int xx = xm; xx <= x2m; ++xx)
	flags |= current(xx, yy).flags;
    return flags;
  }
}


#endif
