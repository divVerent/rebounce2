#ifndef __RP__ENTITY_H__
#define __RP__ENTITY_H__

#include "defs.h"

#include <string>
#include <map>
#include "sprite.h"
#include "coll.h"

#define initialize_int(x) x=str2int(m[#x])
#define initialize_string(x) x=m[#x]
#define initlist_int(x) x(str2int(m[#x]))
#define initlist_string(x) x(m[#x])

namespace World
{
  class _world;
}

namespace nsEntity
{
  typedef std::map<std::string, std::string> ConfigMap;
  struct IOEntity
  {
    int x, y, w, h;
    int groupnum, sprnum;
    ConfigMap properties;
  };
  class Entity
  {
   protected:

   public:
    int _x, _y, dx, dy, ddx, ddy;
    int z;
    int bnc_speed;
    bool floor;
    bool teleportable;
    int jumpnrun;

    ::Sprite::Sprite *mysprite;
    // unit: 256th pixels
    // so: (x >> 8 | y >> 8)
    // dx / framerate = speed
    // ddx / framerate = accel

    int sprnum, groupnum;
    int flags;
    // may only be set by world, so no description here
    ::World::_world *world;
    int x, y;
    int offsetx, offsety;
    int w, h;
    int nextthink;
    std::string targetname;
    Entity (ConfigMap &m, Entity *e = 0);
    virtual ~Entity ();

    void AutoMove (int framerate);

    inline void draw (bitmap buffer, int scrollx, int scrolly)
    {
      int newx = x - scrollx + offsetx;
      int newy = y - scrolly + offsety;
      if (sprnum >= 0)
	::Sprite::Sprites[groupnum][sprnum].draw (buffer, newx, newy);
      else if (mysprite)
	mysprite->draw (buffer, newx, newy);
    }

    enum { LEFT, RIGHT, UP, DOWN, BUTTONA, BUTTONB, BUTTONC, BUTTOND,
      SUICIDE, TOGGLE, SWITCHON, SWITCHOFF, TELEPORT, CHEAT };
      enum { PLAYER2 = 256 };

      virtual int think ();                                 // returns timer-ticks to next think
      virtual bool HandleInput (int key, bool pressed);     // every keypress and every release
      virtual bool Homicide ();
      virtual bool Collided (Entity *);                     // this is 0 normally
      virtual void HandleFlags (int);                       // Level::
  };

  class CREATOR
  {
   public:
    virtual nsEntity::Entity * operator () (ConfigMap &m, Entity *w = 0) = 0;
    virtual ~CREATOR() { }
  };

  template <class T> class Creator : public CREATOR
  {
   public:
    nsEntity::Entity * operator () (ConfigMap &m, Entity *w = 0)
    {
      return new T (m, w);
    }
  };

  typedef std::map <std::string, CREATOR * > CreateMap;
  extern CreateMap Create;

  inline Entity *make_entity (ConfigMap &m, Entity *w = 0)
  {
    CREATOR *maker = Create[m["type"]];
    if (!maker)
      throw std::string ("unknown Entity() type: ") + m["type"];
    return (*maker) (m, w);
  }

  inline void complete_entity (IOEntity &ient)
  {
    ConfigMap &m = ient.properties;
    m["x"] = int2str (ient.x);
    m["y"] = int2str (ient.y);
    m["w"] = int2str (ient.w);
    m["h"] = int2str (ient.h);
    m["sprnum"] = int2str (ient.sprnum);
    m["groupnum"] = int2str (ient.groupnum);
  }

  inline void uncomplete_entity (IOEntity &ient)
  {
    ConfigMap &m = ient.properties;
    ient.x = str2int (m["x"]);                            m.erase ("x");
    ient.y = str2int (m["y"]);                            m.erase ("y");
    ient.w = str2int (m["w"]);                            m.erase ("w");
    ient.h = str2int (m["h"]);                            m.erase ("h");
    ient.sprnum = str2int (m["sprnum"]);                  m.erase ("sprnum");
    ient.groupnum = str2int (m["groupnum"]);              m.erase ("groupnum");
  }

  inline Entity *make_entity (IOEntity ient, Entity *w = 0)
  {
    complete_entity (ient);
    return make_entity (ient.properties, w);
  }

  extern nsEntity::Entity *make_entity_ex_world;

  inline Entity *make_entity_ex_forworld (IOEntity *ient, Entity *w)
  {
    if (!w)
      w = make_entity_ex_world;
    complete_entity (*ient);
    Entity *e = make_entity (ient->properties, w);
    uncomplete_entity (*ient);
    return e;
  }
  inline Entity *make_entity_ex (IOEntity *ient)
  {
	  return make_entity_ex_forworld (ient, 0);
  }

  typedef std::vector < nsEntity::Entity *> EntityContainer;

  inline void DoneEntity ()
  {
    for (CreateMap::iterator i = Create.begin(); i != Create.end(); ++i)
      if (i->second)
	delete i->second;
  }
}

#define ADD_ENTITY_TYPE(t) nsEntity::Create[#t] = new nsEntity::Creator < t > ()

#endif
