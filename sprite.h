#ifndef __RP__SPRITE_H__
#define __RP__SPRITE_H__

#include "defs.h"

#include "alleg.h"
#include <map>
#include <vector>

#define LIGHTPINK makecol(255, 0, 255)

namespace Sprite
{
  inline bitmap duplicate_bitmap (bitmap bmp, int x = 0, int y = 0, int w = 0, int h = 0)
  {
    if (!w)
      w = bmp->w;
    if (!h)
      h = bmp->h;
    bitmap b = create_optimized_bitmap (w, h);
    blit (bmp, b, x, y, 0, 0, w, h);
    return b;
  }

  class Sprite
  {
   private:
    bitmap bmp;
    RLE_SPRITE *rle;
    int w, h;

    void rle2bmp()
    {
      if (!rle) throw "NO SPRITE IN rle2bmp!";
      bmp = create_optimized_bitmap (w, h);
      clear_to_color (bmp, LIGHTPINK);
      draw_rle_sprite (bmp, rle, 0, 0);
    }

    void bmp2rle()
    {
      if (!bmp) throw "NO SPRITE IN bmp2rle!";
      rle = get_rle_sprite (bmp);
    }

    void rleclear()
    {
      if (rle) destroy_rle_sprite (rle);
      rle = 0;
    }

   public:
    Sprite(): bmp(0), rle(0), w(-1), h(-1)
    {
    }
    Sprite(bitmap b, int rip = 0): bmp(b), rle(0), w(b->w), h(b->h)
    {
      if (!rip)
	bmp = duplicate_bitmap (bmp);
    }
    Sprite(bitmap b, int x, int y, int _w, int _h): rle(0), w(_w), h(_h)
    {
      if (!b)
	throw "Sprite(bitmap,int) : no bitmap";
      // b = create_sub_bitmap (b, x, y, w, h);
      // seems to be buggy
      bmp = duplicate_bitmap (b, x, y, _w, _h);
      w = _w;
      h = _h;
    }
    Sprite(const Sprite &spr): rle(0)
    {
      if (!spr.bmp)
      {
	bmp = 0;
	w = -1;
	h = -1;
      }
      else
      {
	bmp = duplicate_bitmap (static_cast<bitmap>(const_cast<Sprite &>(spr)));
	w = bmp->w;
	h = bmp->h;
      }
    }
    ~Sprite()
    {
      if (bmp) free_optimized_bitmap (bmp);
      if (rle) destroy_rle_sprite (rle);
    }

    Sprite &operator= (const Sprite &spr)
    {
      if (bmp) free_optimized_bitmap (bmp);
      if (rle) destroy_rle_sprite (rle);
      rle = 0;
      if (!spr.bmp)
      {
	bmp = 0;
	w = -1;
	h = -1;
      }
      else
      {
	bmp = duplicate_bitmap (static_cast<bitmap >(const_cast<Sprite &>(spr)));
	w = bmp->w;
	h = bmp->h;
      }
      return *this;
    }

    // now the important ones:

    operator bitmap ()
    {
      if (!bmp)
	rle2bmp();
      rleclear();
      return bmp;
    }

    operator RLE_SPRITE* ()
    {
      if (!rle)
	bmp2rle();
      return rle;
    }

    int width ()
    {
      return w;
    }

    int height ()
    {
      return h;
    }

    void draw (bitmap screen, int x, int y)
    {
      draw_rle_sprite (screen, *this, x, y);
      // draw_sprite (screen, *this, x, y);
    }
  };

  extern std::map <int, std::vector<Sprite> > Sprites;
}


#endif
