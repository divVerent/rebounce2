#ifndef __ENEMIES_H__
#define __ENEMIES_H__

namespace Objects
{
  void AddEnemies ();
  void throw_bomb (nsEntity::Entity *thrower, int radius, int time, bool visible, bool sticky = 0);
  void throw_nitroglycerine (nsEntity::Entity *thrower, int x, int y, int dx, int dy, int radius, int explradius, int sprnum, int groupnum);
}

#endif
