#pragma warning (disable: 4786)

#ifndef __RP__DEFINITIONS_H__
#define __RP__DEFINITIONS_H__

// #define NDEBUG

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>

#ifdef WIN32
# define SAVEPREFIX std::string("")
#else
# define SAVEPREFIX (getenv("HOME") ? std::string(getenv("HOME")) + "/.rebounce2/" : std::string(""))
#endif

typedef struct BITMAP *bitmap;

int const SPRWIDTH = 16;
int const SPRHEIGHT = 16;
extern bool EnableVSync;
extern bool EnableSound;
extern bitmap savescreen;

extern int CACHEWIDTH;
extern int CACHEHEIGHT;
extern int HUD;
extern int SCRWIDTH;
extern int SCRHEIGHT;
extern int FRAMERATE;
extern int SOUND_LIMIT;
extern int MAXWIDTH;
extern int MAXHEIGHT;

extern std::string startlevel;

bitmap create_optimized_bitmap (int w, int h);
void free_optimized_bitmap (bitmap bmp);
void check_bitmap_memory ();

bool graphics_init (bool UseColor24 = 0, bool EnableFullscreen = 1);
void graphics_done (void);
bool sound_init ();

inline int str2int (std::string s)
{
  return atoi(s.c_str());
}

inline std::string int2str (int i)
{
  static char buf[40];
  // itoa (i, buf, 10);
  sprintf (buf, "%d", i);
  return std::string (buf);
}

template <class T> inline void destroy_ptr_container (T &cont)
{
  for (typename T::iterator i = cont.begin(); i != cont.end(); ++i)
    delete *i;
}

std::string Filename (const std::string &s);

void fade_screen (bitmap buffer);

// map flags

enum { NONE = 0, COLLIDES = 1, DEADLY = 2, SWITCHABLE = 4, FLOOR = 256, AIR = 512 };
// DO NOT USE FLOOR


int Random (int n);

#ifndef NDEBUG
void INIT_PROFILE ();
void DONE_PROFILE ();
void BEGIN_PROFILE (const std::string &s);
void END_PROFILE ();
void VIEW_PROFILE (bitmap bmp);
void RESET_PROFILE ();
void WAIT_IF_PROFILE ();
void ADD_PROFILE_DATA (std::string, int);
#else
#define INIT_PROFILE()
#define DONE_PROFILE()
#define BEGIN_PROFILE(s)
#define END_PROFILE()
#define VIEW_PROFILE(bmp)
#define RESET_PROFILE()
#define WAIT_IF_PROFILE()
#define ADD_PROFILE_DATA(x,y)
#endif


void update_screen (bitmap buffer);

#endif
