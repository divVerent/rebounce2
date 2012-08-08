#include "map.h"

#include "coll.h"
#include "milli.h"
#include "sprite.h"

void Level::Map::InitCache (int CWidth, int CHeight)
{
  if (Cache)
    ShutdownCache ();
  CacheWidth = CWidth;
  CacheHeight = CHeight;
  Cache = create_optimized_bitmap (CacheWidth, CacheHeight);
  CacheXPos = scrxpos;
  CacheYPos = scrypos;
  InvalidateRect (CacheXPos, CacheYPos, CacheWidth, CacheHeight);
}

void Level::Map::ShutdownCache ()
{
  if (!Cache)
    return;
  free_optimized_bitmap (Cache);
  Cache = 0;
}

void Level::Map::InvalidateRect (int xx, int yy, int ww, int hh)
{
  int x1, x2, y1, y2;
  xx = MAX (xx, CacheXPos);
  yy = MAX (yy, CacheYPos);
  ww = MIN (ww, CacheWidth);
  hh = MIN (hh, CacheHeight);
  Collision::get_map_range (xx, yy, ww, hh, SPRWIDTH, SPRHEIGHT, &x1, &y1, &x2, &y2);
  for (int y = y1; y <= y2; ++y)
    for (int x = x1; x <= x2; ++x)
    {
      Sprite::Sprites[operator()((x+mapwidth)%mapwidth, (y+mapheight)%mapheight).groupnum]
	[operator()((x+mapwidth)%mapwidth, (y+mapheight)%mapheight).sprnum]
	.draw(Cache, x * SPRWIDTH - CacheXPos, y * SPRHEIGHT - CacheYPos);
      if (cheat)
      {
	if (operator()((x+mapwidth)%mapwidth, (y+mapheight)%mapheight).flags & COLLIDES)
	  line (Cache, x * SPRWIDTH - CacheXPos, y * SPRHEIGHT - CacheYPos,
	      x * SPRWIDTH - CacheXPos + SPRWIDTH - 1, y * SPRHEIGHT - CacheYPos + SPRHEIGHT - 1, makecol (0, 0, 255));
	if (operator()((x+mapwidth)%mapwidth, (y+mapheight)%mapheight).flags & DEADLY)
	  line (Cache, x * SPRWIDTH - CacheXPos, y * SPRHEIGHT - CacheYPos,
	      x * SPRWIDTH - CacheXPos + SPRWIDTH - 1, y * SPRHEIGHT - CacheYPos + SPRHEIGHT - 1, makecol (255, 0, 0));
	if (operator()((x+mapwidth)%mapwidth, (y+mapheight)%mapheight).flags & SWITCHABLE)
	  line (Cache, x * SPRWIDTH - CacheXPos + SPRWIDTH - 1, y * SPRHEIGHT - CacheYPos,
	      x * SPRWIDTH - CacheXPos, y * SPRHEIGHT - CacheYPos + SPRHEIGHT - 1, makecol (0, 255, 0));
      }
    }
}

void Level::Map::InvalidateCache ()
{
  InvalidateRect (CacheXPos, CacheYPos, CacheWidth, CacheHeight);
}

void Level::Map::draw (bitmap screen)
{
  if ((scrxpos < CacheXPos)
      ||
      (scrypos < CacheYPos)
      ||
      (scrxpos > (CacheXPos + CacheWidth - screen->w))
      ||
      (scrypos > (CacheYPos + CacheHeight - screen->h))
     )
    ScrollCache (scrxpos - (CacheWidth - screen->w) / 2,
	scrypos - (CacheHeight - screen->h) / 2);

  blit (Cache, screen, scrxpos - CacheXPos, scrypos - CacheYPos, 0, 0, screen->w, screen->h);
}

void Level::Map::ScrollCacheByX (int dx)
{
  if (!dx)
    return;
  if (dx < 0)
  {
    // blit it
    blit (Cache, Cache, 0, 0, -dx, 0, CacheWidth + dx, CacheHeight);
    // invalidate everything [else]
    CacheXPos += dx;
    InvalidateRect (CacheXPos, CacheYPos, -dx, CacheHeight);
  }
  else
  {
    // blit it
    blit (Cache, Cache, dx, 0, 0, 0, CacheWidth - dx, CacheHeight);
    // invalidate everything [else]
    CacheXPos += dx;
    InvalidateRect (CacheXPos + CacheWidth - dx, CacheYPos, dx, CacheHeight);
  }
}

void Level::Map::ScrollCacheByY (int dy)
{
  if (!dy)
    return;
  if (dy < 0)
  {
    // blit it
    blit (Cache, Cache, 0, 0, 0, -dy, CacheWidth, CacheHeight + dy);
    // invalidate everything [else]
    CacheYPos += dy;
    InvalidateRect (CacheXPos, CacheYPos, CacheWidth, -dy);
  }
  else
  {
    // blit it
    blit (Cache, Cache, 0, dy, 0, 0, CacheWidth, CacheHeight - dy);
    // invalidate everything [else]
    CacheYPos += dy;
    InvalidateRect (CacheXPos, CacheYPos + CacheHeight - dy, CacheWidth, dy);
  }
}

void Level::Map::ScrollCache (int x, int y)
{
  ScrollCacheByX (x - CacheXPos);
  ScrollCacheByY (y - CacheYPos);
}

Level::Map::Map(char *ptr, unsigned int n): Cache(0), mapwidth(0), mapheight(0), scrxpos(0), scrypos(0), selxpos(0), selypos(0), blockxpos(0), blockypos(0), cheat(0)
{
  Map_IO_header *h = reinterpret_cast<Map_IO_header *>(ptr);
  if (h->version != sizeof (Map_IO_header))
    throw "invalid map format: conversion needed";
  if (sizeof (Map_IO_tile) * h->width * h->height + sizeof (Map_IO_header) != n)
    throw "invalid map format: file is corrupted";
  mapwidth = h->width;
  mapheight = h->height;
  m.resize (mapwidth * mapheight);
  Map_IO_tile *tile = reinterpret_cast<Map_IO_tile *> (h + 1);
  for (int i = 0; i < mapwidth * mapheight; ++i)
  {
    m[i].groupnum = tile[i].groupnum;
    m[i].sprnum = tile[i].sprnum;
    m[i].sprswitchnum = tile[i].sprswitchnum;
    m[i].flags = tile[i].flags;
    m[i].swflags = tile[i].swflags;
  }
  InitCache (CACHEWIDTH, CACHEHEIGHT);
}

char *Level::Map::OutputMap ()
{
  char *ptr = new char[OutputMap_Size()];
  Map_IO_header *h = reinterpret_cast<Map_IO_header *>(ptr);
  h->version = sizeof (Map_IO_header);
  h->width = mapwidth;
  h->height = mapheight;
  Map_IO_tile *tile = reinterpret_cast<Map_IO_tile *>(h + 1);
  for (int i = 0; i < mapwidth * mapheight; ++i)
  {
    tile[i].groupnum = m[i].groupnum;
    tile[i].sprnum = m[i].sprnum;
    tile[i].sprswitchnum = m[i].sprswitchnum;
    tile[i].flags = m[i].flags;
    tile[i].swflags = m[i].swflags;
  }
  return ptr;
}

Level::Map::Map(int w, int h): Cache(0), mapwidth(w), mapheight(h), scrxpos(0), scrypos(0), selxpos(0), selypos(0), blockxpos(0), blockypos(0), cheat(0)
{
  m.resize (mapwidth * mapheight);
  for (int x = 0; x < mapwidth * mapheight; ++x)
    m[x] = Level::Maptile ();
  InitCache (CACHEWIDTH, CACHEHEIGHT);
}

Level::Map& Level::Map::operator= (const Level::Map &in)
{
  ShutdownCache ();
  m = in.m;
  Cache = 0;
  mapwidth = in.mapwidth;
  mapheight = in.mapheight;
  scrxpos = in.scrxpos;
  scrypos = in.scrypos;
  selxpos = in.selxpos;
  selypos = in.selypos;
  blockxpos = in.blockxpos;
  blockypos = in.blockypos;
  cheat = in.cheat;
  InitCache (CACHEWIDTH, CACHEHEIGHT);
  return *this;
}

Level::Map::Map (const Level::Map &in) : m(in.m), Cache(0), mapwidth(in.mapwidth), mapheight(in.mapheight), scrxpos(in.scrxpos), scrypos(in.scrypos), selxpos(in.selxpos), selypos(in.selypos), blockxpos(in.blockxpos), blockypos(in.blockypos), cheat(in.cheat)
{
  InitCache (CACHEWIDTH, CACHEHEIGHT);
}

void Level::Map::do_scroll (bool b, bitmap scr)
{
  if (b)
  {
    while (scrxpos < 0)
      scrxpos += SPRWIDTH * mapwidth;
    while (scrypos < 0)
      scrypos += SPRWIDTH * mapheight;
    scrxpos %= (SPRWIDTH * mapwidth);
    scrypos %= (SPRHEIGHT * mapheight);
  }
  else
  {
    scrxpos = scrxpos < 0 ? 0 : (scrxpos + scr->w > SPRWIDTH * mapwidth ? SPRWIDTH * mapwidth - scr->w : scrxpos);
    scrypos = scrypos < 0 ? 0 : (scrypos + scr->h > SPRHEIGHT * mapheight ? SPRHEIGHT * mapheight - scr->h : scrypos);
  }
}
