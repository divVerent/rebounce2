#include "defs.h"

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <map>
#include "alleg.h"
#include "milli.h"

// #define PROFILE
// #define ALLINONE
// #define INEXE
// #define USE_VIDEO_BITMAPS
// #define FAVOR_VESA

#ifdef INEXE
#define ALLINONE
#endif

std::string startlevel = "01S2;11S2;21S2;31S2;41S2;51S2;61S2";

int HUD = 20;

#ifdef DJGPP
BEGIN_GFX_DRIVER_LIST
#ifdef FAVOR_VESA
GFX_DRIVER_VESA1
GFX_DRIVER_MODEX
#else
GFX_DRIVER_MODEX
GFX_DRIVER_VESA1
#endif
END_GFX_DRIVER_LIST

BEGIN_COLOR_DEPTH_LIST
COLOR_DEPTH_8
COLOR_DEPTH_15
COLOR_DEPTH_16
COLOR_DEPTH_24
END_COLOR_DEPTH_LIST

BEGIN_MIDI_DRIVER_LIST
MIDI_DRIVER_DIGMID
MIDI_DRIVER_ADLIB
END_MIDI_DRIVER_LIST

BEGIN_DIGI_DRIVER_LIST
DIGI_DRIVER_SB
END_DIGI_DRIVER_LIST
#endif

int SCRWIDTH = 320;

#ifdef EDITOR
int SCRHEIGHT = 200;
#else
int SCRHEIGHT = 240;
#endif

int FRAMERATE = 40;
int SOUND_LIMIT = 8;
int CACHEWIDTH = 640;
int CACHEHEIGHT = 400;
int MAXWIDTH = 64;
int MAXHEIGHT = 64;

bool EnableVSync = 1;
bool EnableSound = 1;



#ifdef INEXE
#define DatFile ""
#else
#define DatFile "rebounce2.dat"
#endif

std::string LowerCase (std::string s)
{
  for (int i = 0; i < s.length (); ++i)
    s[i] = tolower (s[i]);
  return s;
}

std::string Filename (const std::string &s)
{
  std::string ss = LowerCase (s);
#ifdef ALLINONE
  ss = std::string(DatFile) + std::string("#") + ss;
#endif
  return ss;
}

#ifdef GCC
#define srand srandom
#define rand random
#endif

void Randomize ()
{
  static bool ok = 0;
  if (ok)
    return;
  ok = 1;
  srand (time (0));
}

int Random (int n)
{
  Randomize ();
  return rand() % n;
}


int const FADE_SPEED = 16;


void fade_screen (bitmap buf)
{
  int effect = Random (14) + 1;
  switch (effect)
  {
   case 1:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);
      for (int sum = 0; sum <= buf->w + buf->h - 2; ++sum)
      {
	for (int diff = MAX (-sum, sum - 2 * buf->h + 2);
	    diff <= MIN (sum, 2 * buf->w - sum - 2);
	    diff += 2)
	  putpixel (buf2, (diff + sum) / 2, (sum - diff) / 2,
	      getpixel (buf, (diff + sum) / 2, (sum - diff) / 2));
	if (!(sum % 20))
	  update_screen (buf2);
      }
      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 2:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);
      for (int n = 0; n < 36; ++n)
      {
	static int c[2*36] =
	{
	  0,   0,
	  1,   0,
	  1,   1,
	  0,   1,
	  0,   2,
	  1,   2,
	  2,   2,
	  2,   1,
	  2,   0,
	  3,   0,
	  3,   1,
	  3,   2,
	  3,   3,
	  2,   3,
	  1,   3,
	  0,   3,
	  0,   4,
	  1,   4,
	  2,   4,
	  3,   4,
	  4,   4,
	  4,   3,
	  4,   2,
	  4,   1,
	  4,   0,
	  5,   0,
	  5,   1,
	  5,   2,
	  5,   3,
	  5,   4,
	  5,   5,
	  4,   5,
	  3,   5,
	  2,   5,
	  1,   5,
	  0,   5
	};
	int xx = c[2 * n];
	int yy = c[2 * n + 1];
	for (int y = yy; y < buf->h; y += 6)
	  for (int x = xx; x < buf->w; x += 6)
	    putpixel (buf2, x, y, getpixel (buf, x, y));
	update_screen (buf2);
      }
      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 3:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int start = 0; start < 16; ++start)
      {
	for (int sum = start; sum <= buf->w + buf->h - 2; sum += 16)
	{
	  for (int diff = MAX (-sum, sum - 2 * buf->h + 2);
	      diff <= MIN (sum, 2 * buf->w - sum - 2);
	      diff += 2)
	    putpixel (buf2, (diff + sum) / 2, (sum - diff) / 2,
		getpixel (buf, (diff + sum) / 2, (sum - diff) / 2));
	}
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 4:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int start = 0; start < 16; ++start)
      {
	for (int y = start; y < buf->h; y += 16)
	  for (int x = 0; x < buf->w; ++x)
	    putpixel (buf2, x, y, getpixel (buf, x, y));
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 5:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int start = 0; start < 16; ++start)
      {
	for (int x = start; x < buf->w; x += 16)
	  for (int y = 0; y < buf->h; ++y)
	    putpixel (buf2, x, y, getpixel (buf, x, y));
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 6:
    {
      // GIF fade
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int interlace = 64; interlace != 1; interlace >>= 1)
      {
	for (int y = 0; y < buf->h; ++y)
	  for (int x = 0; x < buf->w; ++x)
	    putpixel (buf2, x, y, getpixel (buf, x, y - y % interlace));
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 7:
    {
      // Bidir interlace
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int interlace = 64; interlace != 1; interlace >>= 1)
      {
	for (int y = 0; y < buf->h; ++y)
	  for (int x = 0; x < buf->w; ++x)
	  {
	    if ((x / 64 + y / 64) & 1)
	      putpixel (buf2, x, y, getpixel (buf, x, y - y % interlace));
	    else
	      putpixel (buf2, x, y, getpixel (buf, x - x % interlace, y));
	  }
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 8:
    {
      // Interlace Blocks
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int interlace = 64; interlace != 1; interlace >>= 1)
      {
	for (int y = 0; y < buf->h; ++y)
	  for (int x = 0; x < buf->w; ++x)
	  {
	    putpixel (buf2, x, y, getpixel (buf, x - x % interlace, y - y % interlace));
	  }
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 9:
    {
      // GIF fade x <-> y
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);

      for (int interlace = 64; interlace != 1; interlace >>= 1)
      {
	for (int y = 0; y < buf->h; ++y)
	  for (int x = 0; x < buf->w; ++x)
	    putpixel (buf2, x, y, getpixel (buf, x - x % interlace, y));
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 10:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf2, 0, 0, 0, 0, buf->w, buf->h);
      int cx = Random (buf->w);
      int cy = Random (buf->h);

      for (int i = 0; i < FADE_SPEED; ++i)
      {
	int w = buf->w * i / (FADE_SPEED);
	int h = buf->h * i / (FADE_SPEED);
	int x = cx * (FADE_SPEED - i) / FADE_SPEED;
	int y = cy * (FADE_SPEED - i) / FADE_SPEED;
	stretch_blit (buf, buf2, 0, 0, buf->w, buf->h,
	    x, y, w, h);
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
    }
    break;
   case 11:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      bitmap buf3 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf3, 0, 0, 0, 0, buf->w, buf->h);
      int cx = Random (buf->w);
      int cy = Random (buf->h);

      for (int i = FADE_SPEED-1; i >= 0; --i)
      {
	int w = buf->w * i / (FADE_SPEED);
	int h = buf->h * i / (FADE_SPEED);
	int x = cx * (FADE_SPEED - i) / FADE_SPEED;
	int y = cy * (FADE_SPEED - i) / FADE_SPEED;
	blit (buf, buf2, 0, 0, 0, 0, buf->w, buf->h);
	stretch_blit (buf3, buf2, 0, 0, buf->w, buf->h,
	    x, y, w, h);
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
      free_optimized_bitmap (buf3);
    }
    break;
   case 12:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      bitmap buf3 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf3, 0, 0, 0, 0, buf->w, buf->h);

      for (int i = 0; i < FADE_SPEED; ++i)
      {
	int y = buf->h * i / FADE_SPEED;
	stretch_blit (buf, buf2, 0, 0, buf->w, buf->h,
	    0, 0, buf->w, y);
	stretch_blit (buf3, buf2, 0, 0, buf->w, buf->h,
	    0, y, buf->w, buf->h - y);
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
      free_optimized_bitmap (buf3);
    }
    break;
   case 13:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      bitmap buf3 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf3, 0, 0, 0, 0, buf->w, buf->h);

      for (int i = 0; i < FADE_SPEED; ++i)
      {
	int x = buf->w * i / FADE_SPEED;
	int y = buf->h * i / FADE_SPEED;
	clear_to_color (buf2, makecol (0, 0, 0));
	stretch_blit (buf, buf2, 0, 0, buf->w, buf->h,
	    0, 0, x, y);
	stretch_blit (buf3, buf2, 0, 0, buf->w, buf->h,
	    x, y, buf->w - x, buf->h - y);
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
      free_optimized_bitmap (buf3);
    }
    break;
   case 14:
    {
      bitmap buf2 = create_optimized_bitmap (buf->w, buf->h);
      bitmap buf3 = create_optimized_bitmap (buf->w, buf->h);
      blit (savescreen, buf3, 0, 0, 0, 0, buf->w, buf->h);

      for (int i = 0; i < FADE_SPEED; ++i)
      {
	int x = buf->w * i / FADE_SPEED;
	stretch_blit (buf, buf2, 0, 0, buf->w, buf->h,
	    0, 0, x, buf->h);
	stretch_blit (buf3, buf2, 0, 0, buf->w, buf->h,
	    x, 0, buf->w - x, buf->h);
	update_screen (buf2);
      }

      update_screen (buf);
      free_optimized_bitmap (buf2);
      free_optimized_bitmap (buf3);
    }
    break;
  }
}

namespace
{
  typedef std::map <bitmap, std::pair <int, int> > bitmaps_t;
  bitmaps_t bitmaps;
}

bitmap create_optimized_bitmap (int w, int h)
{
  bitmap bmp =
#ifdef USE_VIDEO_BITMAPS
    create_video_bitmap (w, h);
#else
  0
#endif
    ;
  if (!bmp)
    bmp = create_bitmap (w, h);
  if (!bmp)
    throw "Out of bitmap memory";
  bitmaps [bmp] = std::make_pair (w, h);
  return bmp;
}

void free_optimized_bitmap (bitmap bmp)
{
  if (!bmp)
    throw "Freeing NULL bitmap?";
  destroy_bitmap (bmp);
  if (!bitmaps.erase (bmp))
    throw "Freeing unknown bitmap?";
}

void check_bitmap_memory ()
{
  for (bitmaps_t::iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
    std::cerr << "Unfreed bitmap found, size: " << i->second.first << " x " << i->second.second << std::endl;
}

#ifdef PROFILE

#include <stack>
#include <string>

namespace
{
  map <std::string, int> profile_data;
  map <std::string, int> profile2_data;
  stack <std::string> current;
  int last;
  bool ok = 0;
}
void INIT_PROFILE ()
{
  current = stack<std::string>();
  current.push ("GLOBAL");
  profile_data.erase (profile_data.begin(), profile_data.end());
  last = milliseconds;
  ok = 1;
}
void DONE_PROFILE ()
{
  ok = 0;
}
void BEGIN_PROFILE (const std::string &s)
{
  if (!ok) return;
  profile_data [current.top()] += milliseconds - last;
  current.push (s);
  last = milliseconds;
}
void END_PROFILE ()
{
  if (!ok) return;
  profile_data [current.top()] += milliseconds - last;
  if (current.top() != "GLOBAL")
    current.pop ();
  else
    throw "PROFILE ERROR: CANNOT POP";
  last = milliseconds;
}
void VIEW_PROFILE (bitmap bmp)
{
  if (!ok) return;
  int const dy = 10;
  int lasty = dy * profile2_data.size();
  int y = (bmp->h - lasty) / 2;
  int sum = 0;
  for (map<std::string, int>::iterator i = profile2_data.begin(); i != profile2_data.end(); ++i)
    if (i->first[0] != '*')
      sum += i->second;
  if (!sum)
    sum = 1;
  for (map<std::string, int>::iterator i = profile2_data.begin(); i != profile2_data.end(); ++i, y += dy)
  {
    const char *s = (i->first + ": " + int2str((i->first[0] == '*') ? i->second : 1000 * i->second / sum)).c_str();
    textout_centre (bmp, font, s, bmp->w / 2, y + 1, makecol (0, 0, 0));
    textout_centre (bmp, font, s, bmp->w / 2 + 1, y, makecol (0, 0, 0));
    textout_centre (bmp, font, s, bmp->w / 2 - 1, y, makecol (0, 0, 0));
    textout_centre (bmp, font, s, bmp->w / 2, y - 1, makecol (0, 0, 0));
    textout_centre (bmp, font, s, bmp->w / 2, y, makecol (255, 255, 255));
  }
  last = milliseconds;
}
void WAIT_IF_PROFILE ()
{
  if (!ok) return;
  profile_data [current.top()] += milliseconds - last;
  int l = milliseconds + 100;
  while (milliseconds < l)
    ;
  last = milliseconds;
}
void RESET_PROFILE ()
{
  if (!ok) return;
  profile2_data = profile_data;
  current = stack<std::string>();
  current.push ("GLOBAL");
  profile_data.erase (profile_data.begin(), profile_data.end());
  last = milliseconds;
}
void ADD_PROFILE_DATA (std::string name, int value)
{
  profile_data[name] = value;
}
#else
#ifndef NDEBUG
void INIT_PROFILE ()
{
}
void ADD_PROFILE_DATA (std::string name, int value)
{
}
void DONE_PROFILE ()
{
}
void BEGIN_PROFILE (const std::string &s)
{
}
void END_PROFILE ()
{
}
void VIEW_PROFILE (bitmap bmp)
{
}
void RESET_PROFILE ()
{
}
void WAIT_IF_PROFILE ()
{
}
#endif
#endif

namespace
{
  bool doScale = false;
  int scaleX = 0;
  int scaleY = 0;
  int scaleW = 0;
  int scaleH = 0;
};
bitmap savescreen = NULL;
bool graphics_init (bool UseColor24, bool EnableFullscreen)
{
  int dw = 0, dh = 0;
  int w = SCRWIDTH;
  int h = SCRHEIGHT;
  if (EnableFullscreen && get_desktop_resolution(&dw, &dh) == 0)
  {
    int depth = 0;
    depth = desktop_color_depth();
    if(depth <= 0)
      depth = 32;
    set_color_depth (depth);
    if (!set_gfx_mode (GFX_AUTODETECT_FULLSCREEN, dw, dh, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      if(w != SCREEN_W || h != SCREEN_H)
      {
        doScale = true;
	if(w * SCREEN_H > h * SCREEN_W)
	{
          scaleW = (SCREEN_W / w) * w;
          scaleH = (scaleW * h) / w;
	}
	else
	{
          scaleH = (SCREEN_H / h) * h;
          scaleW = (scaleH * w) / h;
	}
        scaleX = (SCREEN_W - scaleW) / 2;
        scaleY = (SCREEN_H - scaleH) / 2;
	savescreen = create_optimized_bitmap(w, h);
      }
      return 1;
    }
  }

  if (UseColor24)
  {
    set_color_depth (32);
    if (!set_gfx_mode (EnableFullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (24);
    if (!set_gfx_mode (EnableFullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (16);
    if (!set_gfx_mode (EnableFullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (15);
    if (!set_gfx_mode (EnableFullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (32);
    if (!set_gfx_mode (GFX_AUTODETECT, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (24);
    if (!set_gfx_mode (GFX_AUTODETECT, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (16);
    if (!set_gfx_mode (GFX_AUTODETECT, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
    set_color_depth (15);
    if (!set_gfx_mode (GFX_AUTODETECT, w, h, 0, 0))
    {
      savescreen = screen;
      set_color_conversion (COLORCONV_NONE);
      return 1;
    }
  }
  set_color_depth (8);
  if (!set_gfx_mode (GFX_AUTODETECT, w, h, 0, 0))
  {
    PALETTE p;
    generate_332_palette (p);
    set_palette (p);
    savescreen = screen;
    set_color_conversion (COLORCONV_NONE);
    return 1;
  }
  std::cerr << "VGA required." << std::endl;
  throw std::string("Invalid gfx card - ") + allegro_error;
}

void graphics_done (void)
{
  if(savescreen != screen)
    free_optimized_bitmap(savescreen);
  savescreen = screen;
}

bool sound_init ()
{
  if(!EnableSound)
    return 1;
  reserve_voices (SOUND_LIMIT, 9);              // not more!
  if (!install_sound (DIGI_AUTODETECT, MIDI_AUTODETECT, 0))
    return 1;
  reserve_voices (-1, -1);
  return !install_sound (DIGI_AUTODETECT, MIDI_AUTODETECT, 0)
    || !install_sound (DIGI_AUTODETECT, 0, 0)
    || !install_sound (0, MIDI_AUTODETECT, 0)
    || ((EnableSound = 0));
}

void update_screen (bitmap buffer)
{
  if (EnableVSync && !Timer::did_vsync ())
  {
    BEGIN_PROFILE ("vsync");
    vsync();
    END_PROFILE ();
  }
  acquire_bitmap (screen);
  if(doScale)
  {
    blit (buffer, savescreen, 0, 0, 0, 0, buffer->w, buffer->h);
    stretch_blit (buffer, screen, 0, 0, buffer->w, buffer->h, scaleX, scaleY, scaleW, scaleH);
  }
  else
    blit (buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
  release_bitmap (screen);
}

