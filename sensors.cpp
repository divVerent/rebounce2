#define __RP__OBJECTS_CPP__

#include "objects.h"

#include "entity.h"
#include "world.h"

#include "enemies.h"
#include "sensors.h"

namespace Objects
{

  OBJECT (Target)
  {
    std::string left, right, target, up, down;
    bool State, dontLR, dontUD;
    friend class Teleporter;
   public:
    Target (nsEntity::ConfigMap &m, nsEntity::Entity *w): Entity (m, w), State (0),
      initlist_string (left),
      initlist_string (right),
      initlist_string (up),
      initlist_string (down),
      initlist_string (target),
      initlist_int (dontLR),
      initlist_int (dontUD)
    {
      world->Register (this, World::_world::NOGRAVITY, 1);
    }
    bool HandleInput (int key, bool pressed)
    {
      if (!pressed)
	return 0;
      if (key == SWITCHON)
	State = 1;
      if (key == SWITCHOFF)
	State = 0;
      if (key == TOGGLE)
	State = !State;
      return 0;
    }
  };

  /**************************************************************************
   *                                                                        *
   * The Sensor object.                                                     *
   *                                                                        *
   * Base for Killers, Switchers, Items, Exits ...                          *
   *                                                                        *
   **************************************************************************/


  OBJECT (Sensor)
  {
   protected:
    int Action;
    int delta;
   public:
    Sensor (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity (m, w)
    {
      nextthink = -1;
      initialize_int (delta);
      if (!delta)
	delta = 50;
      std::string action;
      initialize_string (action);
      Action = world->ParseAction (action);
      world->Register (this, World::_world::NOGRAVITY, 1);
    }
    void think(int remaining) {
    }
  };

  EXTEND (Wind, Sensor)
  {
    int xstate, ystate, xstate2, ystate2;
    bool State;
    int nominal_delta;
    double factor;
   public:
    Wind (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Sensor (m, w), State (0),
      initlist_int (xstate),
      initlist_int (ystate),
      initlist_int (xstate2),
      initlist_int (ystate2)
    {
      // Wind acts every frame, unlike other sensors.
      nominal_delta = delta;
      delta = 1;
    }
    static void MoveIt (nsEntity::Entity *Actor, nsEntity::Entity *Actend)
    {
      Wind *w;
      if (Actor == Actend)
	return;
      if (Actend->flags & World::_world::DISABLED)
	return;
      if (!(Actend->flags & World::_world::KILLABLE))
	return;
      if (!(w = dynamic_cast<Wind *> (Actor)))
	return;
      Actend->dx += w->xstate * w->factor;
      Actend->dy += w->ystate * w->factor;
    }
    void think (int remaining)
    {
      int actual_delta = delta - remaining;
      factor = actual_delta / nominal_delta;
      world->ActionBox (this, MoveIt, x, y, w, h);
      nextthink = delta;
    }
    bool HandleInput (int key, bool pressed)
    {
      if (!pressed)
	return 0;
      bool OldState = State;
      if (key == TOGGLE)
	State = !State;
      if (key == SWITCHON)
	State = 1;
      if (key == SWITCHOFF)
	State = 0;
      if (State != OldState)
      {
	int h = xstate;
	xstate = xstate2;
	xstate2 = h;
	h = ystate;
	ystate = ystate2;
	ystate2 = h;
      }
      return 0;
    }
  };

  EXTEND (Exit, Sensor)
  {
    std::string next2;
   public:
    Exit (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Sensor (m, w),
      initlist_string (next2)
    {
      std::string next;
      initialize_string (next);
      if (next == "")
	next = "\\END";
      if (next2 == "")
	next2 = next;
      targetname = next;
    }
    bool Homicide ()
    {
      return 0;
    }
    void think(int remaining)
    {
      world->Action (this, ::World::_world::EXIT, x, y, w, h, 1);
      if (world->Found)
      {
	std::string s = targetname;
	targetname = next2;
	next2 = s;
      }
      nextthink = delta;
    }
  };

  EXTEND (KillBox, Sensor)
  {
   public:
    KillBox (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Sensor (m, w)
    {
      Action = World::_world::KILL;
    }
  };

  EXTEND (Teleporter, Sensor)
  {
    nsEntity::Entity *Target;
    std::string target;
    int speed;
    nsEntity::Entity *ObjToTeleport;
    std::string current_target;
    bool use_left, use_right, use_target, use_up, use_down;
    bool uleft, uright, utarget, uup, udown;
   public:
    Teleporter (nsEntity::ConfigMap &m, nsEntity::Entity *w): Sensor (m, w),
      uleft(0), uright(0), utarget(0), uup(0), udown(0), Target(0),
      initlist_int (use_left),
      initlist_int (use_right),
      initlist_int (use_up),
      initlist_int (use_down),
      initlist_int (use_target),
      initlist_int (speed),
      initlist_string (target)
    {
      if (target == "")
	nextthink = 0;
      ObjToTeleport = 0;
      delta = 1; // do it every frame
    }
    void think(int remaining)
    {
      if (ObjToTeleport)
      {
	Target = world->FindTarget (current_target);
	if ((current_target != "") && Target && ((Target->x != ObjToTeleport->x) || (Target->y != ObjToTeleport->y)))
	{
	  int xdistance = Target->x - ObjToTeleport->x;
	  int ydistance = Target->y - ObjToTeleport->y;
	  if (ABS(xdistance) + ABS(ydistance) <= speed) // i am done
	  {
	    ObjToTeleport->x = Target->x;
	    ObjToTeleport->y = Target->y;
	    current_target = "";
	    if (::Objects::Target *tgt = dynamic_cast <Objects::Target *> (Target))
	    {
	      if (tgt->State)
	      {
		if (!tgt->dontLR)
		{
		  bool h = uleft;
		  uleft = uright;
		  uright = h;
		}
		if (!tgt->dontUD)
		{
		  bool h = uup;
		  uup = udown;
		  udown = h;
		}
	      }
	      if (uleft)
		current_target = tgt->left;
	      if (uright)
		current_target = tgt->right;
	      if (uup)
		current_target = tgt->up;
	      if (udown)
		current_target = tgt->down;
	      if (utarget)
		current_target = tgt->target;
	    }
	  }
	  else
	  {
	    ObjToTeleport->x += xdistance * speed / (ABS(xdistance) + ABS(ydistance));
	    ObjToTeleport->y += ydistance * speed / (ABS(xdistance) + ABS(ydistance));
	  }
	  ObjToTeleport->_x = ObjToTeleport->x << 8;
	  ObjToTeleport->_y = ObjToTeleport->y << 8;
	}
	else
	{
	  world->Register (ObjToTeleport, World::_world::DISABLED, 0);
	  ObjToTeleport->dx = 0;
	  ObjToTeleport->dy = 0;
	  ObjToTeleport->think (remaining);
	  ObjToTeleport = 0;
	}
      }
      else
      {
	world->Action (this, World::_world::FIND, x, y, w, h, 2);
	if (world->Found)
	{
	  current_target = target;
	  Target = world->FindTarget (target);
	  if (Target)
	  {
	    if (speed)
	    {
	      ObjToTeleport = world->Found;
	      world->Register (ObjToTeleport, World::_world::DISABLED, 1);
	      ObjToTeleport->nextthink = 0;
	      uleft = use_left;
	      uright = use_right;
	      udown = use_down;
	      uup = use_up;
	      utarget = use_target;
	    }
	    else
	    {
	      world->Found->HandleInput (TELEPORT, 1);
	      world->Found->_x = Target->x << 8;
	      world->Found->_y = Target->y << 8;
	      world->Found->dx = 0;
	      world->Found->dy = 0;
	    }
	  }
	}
      }
      nextthink = delta;
    }
  };

  typedef KillBox Killbox;

  BEGIN_OBJECTS_LIST (AddSensors)

    ADD_ENTITY_TYPE (KillBox);
  /*
     : public Sensor
     */

  ADD_ENTITY_TYPE (Killbox);
  /*
     == KillBox
     */

  ADD_ENTITY_TYPE (Sensor);
  /*
     string targetname;
     string action;
     int delta = 50;
     */

  ADD_ENTITY_TYPE (Exit);
  /*
     : public Sensor
     */

  ADD_ENTITY_TYPE (Wind);
  /*
     : public Sensor
     string targetname;
     int xstate, ystate;
     int xstate2, ystate2;
     */

  ADD_ENTITY_TYPE (Teleporter);
  /*
     string target;
     int use_up, use_down, use_left, use_right, use_target;
     int speed = 0;
     */

  ADD_ENTITY_TYPE (Target);
  /*
     string targetname;
     string left, right, up, down, target;
     bool dontLR = 0, dontUD = 0;
     friend class Teleporter;
     */

  END_OBJECTS_LIST
}
