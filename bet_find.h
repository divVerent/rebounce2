#ifndef __RP__BETTER_FIND_H__
#define __RP__BETTER_FIND_H__

#include "entity.h"

namespace Collision
{
  inline bool EntityCompare (nsEntity::Entity *e1, nsEntity::Entity *e2)
  {
    if (!e1)
      return 0;
    if (!e2)
      return 1;
    return e1->x < e2->x;
  }

  template <class C, class F> void for_each_sorted (const C& cont, F f, int x, int y, int w, int h)
  {
    typedef typename C::const_iterator CI;
    static nsEntity::ConfigMap m;
    static nsEntity::Entity e1(m);
    static nsEntity::Entity e2(m);
    e1.x = x - MAXWIDTH;
    e2.x = x + w + MAXWIDTH;
    CI begin = std::lower_bound (cont.begin(), cont.end(), &e1, EntityCompare);
    CI end = std::upper_bound (cont.begin(), cont.end(), &e2, EntityCompare);
    for (CI p = begin; p != end; ++p)
      if (check_box_coll (x, y, w, h, (*p)->x, (*p)->y, (*p)->w, (*p)->h))
	f (*p);
  }

  template <class C> void PrepareEntityContainer (C& cont)
  {
    // sort by X coord
    std::sort (cont.begin(), cont.end(), EntityCompare);
  }
}

#endif
