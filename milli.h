#ifndef __RP__MILLISECONDS_H__
#define __RP__MILLISECONDS_H__

#include "defs.h"

namespace Timer
{
  void init_timer ();
  void done_timer ();
  int synchronize (int framerate, bool video = 1, bool clear = 0);
  volatile extern int milliseconds;
  bool did_vsync ();
}
using Timer::milliseconds;

#endif
