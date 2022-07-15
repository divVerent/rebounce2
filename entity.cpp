#include "entity.h"
#include "world.h"

nsEntity::CreateMap nsEntity::Create;
nsEntity::Entity *nsEntity::make_entity_ex_world = 0;

nsEntity::Entity::Entity (ConfigMap &m, Entity *e) : world(dynamic_cast<World::_world *>(e))
{
  initialize_int (teleportable);
  initialize_int (x);
  initialize_int (y);
  _x = x << 8;
  _y = y << 8;
  initialize_int (offsetx);
  initialize_int (offsety);
  initialize_int (dx) << 8;
  initialize_int (dy) << 8;
  initialize_int (ddx) << 8;
  initialize_int (ddy) << 8;
  initialize_int (w);
  initialize_int (h);
  initialize_int (jumpnrun);
  initialize_int (sprnum);
  initialize_int (groupnum);
  initialize_string (targetname);
  initialize_int (bnc_speed);
  if (!bnc_speed)
    bnc_speed = 1000;
  mysprite = 0;
  flags = 0;
  z = 0;
  nextthink = 0;
  floor = 0;
}

nsEntity::Entity::~Entity ()
{
}

bool nsEntity::Entity::HandleInput (int key, bool pressed)
{
  switch (key)
  {
   case SUICIDE:
    Homicide ();
    return 1;
  }
  return 0;
}
void nsEntity::Entity::think (int remaining)
{
}
bool nsEntity::Entity::Homicide ()
{
  world->kill (this);
  return true;
}
bool nsEntity::Entity::Collided (nsEntity::Entity *)
{
  return 0;
}

void nsEntity::Entity::AutoMove (int framerate)
{
  int collision = 0;
  dx += (ddx + framerate / 2 - 1) / framerate;
  int x2 = _x + (dx + framerate / 2 - 1) / framerate;
  dy += (ddy + framerate / 2 - 1) / framerate;
  int y2 = _y + (dy + framerate / 2 - 1) / framerate;
  if (world->RectFlags (x2 >> 8, y, w, h) & COLLIDES)
  {
    // alles sind RectFlags, keine Vergleiche mit anderen Objekten!
    // Also kann man rechnen. (ich bevorzuge aber den einfachen Weg)
    if (dx < 0)  // nach links
    {
      while ((x2 >> 8) < x && world->RectFlags (x2 >> 8, y, w, h) & COLLIDES)
      {
	x2 += 1 << 8;
      }
    }
    else if (dx)        // nach rechts
    {
      while ((x2 >> 8) > x && world->RectFlags (x2 >> 8, y, w, h) & COLLIDES)
      {
	x2 -= 1 << 8;
      }
    }
    collision = 1;
    if (jumpnrun)
      dx = 0;
    else
      dx = -dx * bnc_speed / 1000;
  }
  if (world->RectFlags (x2 >> 8, y2 >> 8, w, h) & COLLIDES)
  {
    // alles sind RectFlags, keine Vergleiche mit anderen Objekten!
    // Also kann man rechnen. (ich bevorzuge aber den einfachen Weg)
    if (dy < 0)  // nach oben
    {
      while ((y2 >> 8) < y && world->RectFlags (x2 >> 8, y2 >> 8, w, h) & COLLIDES)
      {
	y2 += 1 << 8;
      }
    }
    else if (dy)        // nach unten
    {
      while ((y2 >> 8) > y && world->RectFlags (x2 >> 8, y2 >> 8, w, h) & COLLIDES)
      {
	y2 -= 1 << 8;
      }
    }
    if (dy > 0)  // nach unten kollidiert
      collision = -1;
    else
      collision = 1;
    if (floor && jumpnrun)
      dy = 0;
    else
      dy = -dy * bnc_speed / 1000;
  }
  _x = x2;
  x = _x >> 8;
  _y = y2;
  int y0 = y;
  y = _y >> 8;

  HandleFlags (world->RectFlags (x2 >> 8, y2 >> 8, w, h)
      | (COLLIDES * (collision != 0))
      | (FLOOR * (collision == -1 && !floor))
      | (AIR * (y != y0 && floor))
      );

  if (!floor || y != y0)
  {
    floor = collision == -1;
  }
}

void nsEntity::Entity::HandleFlags (int fl)
{
  if ((fl & DEADLY) && (flags & ::World::_world::KILLABLE))
    Homicide ();
}
