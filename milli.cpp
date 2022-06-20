#include "milli.h"
#include "alleg.h"

volatile int Timer::milliseconds = 0;

namespace
{
  void TimerProc()
  {
    ++Timer::milliseconds;
  }
  END_OF_FUNCTION (TimerProc);
  bool _did_vsync;
}

void Timer::init_timer()
{
  LOCK_FUNCTION (TimerProc);
  LOCK_VARIABLE (milliseconds);
  install_timer ();
  install_int (TimerProc, 1);
  _did_vsync = 0;
}

void Timer::done_timer ()
{
  remove_int (TimerProc);
}

int Timer::synchronize (int framerate, bool video, bool clear)
{
  static int last = 0;

  if (clear)
    last = milliseconds;

  int next = last + 1000 / framerate;
  int delta = next - milliseconds;
  if (delta > 0) {
    rest(delta);
  }
  last = next;

  return 1;
}

bool Timer::did_vsync ()
{
  bool i = _did_vsync;
  _did_vsync = 0;
  return i;
}
