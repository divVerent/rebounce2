#ifndef __RP__MAP_H__
#define __RP__MAP_H__

#include "defs.h"

#include <vector>

namespace Level
{
  struct Maptile
  {
    int groupnum, sprnum, sprswitchnum;
    int flags, swflags;
    Maptile (int gnum, int num, int snum, int flg, int sflg): groupnum(gnum), sprnum(num), sprswitchnum(snum), flags(flg), swflags(sflg)
    {
    }
    Maptile (): groupnum(0), sprnum(0), sprswitchnum(0), flags(1), swflags(1)
    {
    }
  };
  inline bool operator == (const Maptile &n, const Maptile &m)
  {
    return ((n.groupnum == m.groupnum)
	&& (n.sprnum == m.sprnum)
	&& (n.sprswitchnum == m.sprswitchnum)
	&& (n.flags == m.flags)
	&& (n.swflags == m.swflags));
  }
  inline bool operator != (const Maptile &n, const Maptile &m)
  {
    return ((n.groupnum != m.groupnum)
	|| (n.sprnum != m.sprnum)
	|| (n.sprswitchnum != m.sprswitchnum)
	|| (n.flags != m.flags)
	|| (n.swflags != m.swflags));
  }

  struct Map_IO_tile
  {
    short groupnum, sprnum, sprswitchnum;
    char flags, swflags;
  };
  struct Map_IO_header
  {
    int version;
    short width, height;
  };

  class Map
  {
    std::vector<Maptile> m;
    int CacheWidth, CacheHeight, CacheXPos, CacheYPos;
    bitmap Cache;
    void ScrollCache (int x, int y);
    void ScrollCacheByX (int dx);
    void ScrollCacheByY (int dy);
    void InvalidateRect (int x, int y, int w, int h);
    void InitCache (int, int);
    void ShutdownCache ();
    Map() : mapwidth(1), mapheight(1) { }           // remains valid, needed by WorldEnt!
   public:
    int mapwidth, mapheight;
    int scrxpos, scrypos;
    int selxpos, selypos, blockxpos, blockypos, cheat;
    Map(int w, int h);
    Map(char *ptr, unsigned int n);
    Map (const Map& in);
    ~Map ()
    {
      ShutdownCache ();
    }
    Map& operator= (const Map &in);
    char *OutputMap ();
    int OutputMap_Size ()
    {
      return sizeof (Map_IO_header) + mapwidth * mapheight * sizeof (Map_IO_tile);
    }
    inline const Maptile &operator () (int x, int y)
    {
      static Maptile dummy = Maptile();
      if ((x < 0) || (x >= mapwidth) || (y < 0) || (y >= mapheight))
	return dummy;
      return m[x + y * mapwidth];
    }
    inline void set (int x, int y, const Maptile &mt, bool DoInvalidate = 1)
    {
      if ((x < 0) || (x >= mapwidth) || (y < 0) || (y >= mapheight))
	return;
      m[x + y * mapwidth] = mt;
      if (DoInvalidate)
	InvalidateRect (x * SPRWIDTH, y * SPRHEIGHT, SPRWIDTH, SPRHEIGHT);
    }
    void draw(bitmap screen);
    void do_scroll (bool b = 0, bitmap scr = screen);
    inline void scrollto(int x, int y, bool b = 0, bitmap bm = screen)
    {
      scrxpos = x;
      scrypos = y;
      do_scroll (b, bm);
    }
    inline void scrollby(int x, int y, bool b = 0, bitmap bm = screen)
    {
      scrxpos += x;
      scrypos += y;
      do_scroll (b, bm);
    }
    inline void fix_selpos()
    {
      selxpos += mapwidth;
      selxpos %= mapwidth;
      selypos += mapheight;
      selypos %= mapheight;
      blockxpos += mapwidth;
      blockxpos %= mapwidth;
      blockypos += mapheight;
      blockypos %= mapheight;
    }
    void InvalidateCache ();
  };
};

#endif

