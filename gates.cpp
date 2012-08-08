#define __RP__OBJECTS_CPP__

#include "objects.h"
#include "gates.h"
#include "entity.h"
#include "world.h"

namespace Objects
{
  OBJECT (Switch)
  {
   protected:
    nsEntity::Entity *Target;
    std::string target;
    int first_image;
    std::string on_sound, off_sound;
    bool nogfx;
   public:
    bool State;
    Switch (nsEntity::ConfigMap &m, nsEntity::Entity *w): Entity (m, w),
      initlist_string (on_sound),
      initlist_string (off_sound),
      initlist_int (nogfx)
    {
      Target = 0;
      first_image = sprnum;
      world->Register (this, ::World::_world::NOGRAVITY, 1);
      world->Register (this, ::World::_world::SWITCHABLE, 1);
      State = 0;
      initialize_string (target);
    }
    void UpdateGfx ()
    {
      sprnum = first_image + (State && !nogfx ? 1 : 0);
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
      UpdateGfx ();

      if (OldState != State && !nogfx)
      {
	if (State)
	{
	  if (on_sound != "")
	    world->PlaySound (this, on_sound, 128, 1);
	  else
	    world->PlaySound (this, "sw-on.wav", 128, 1);
	}
	else
	{
	  if (off_sound != "")
	    world->PlaySound (this, off_sound, 128, 1);
	  else
	    world->PlaySound (this, "sw-off.wav", 128, 1);
	}
      }

      if (OldState != State && target != "")
	if ((Target = world->FindTarget (target)))
	{
	  Target->HandleInput (TOGGLE, 1);
	  Target->HandleInput (TOGGLE, 0);
	}

      return 0;
    }
  };

  EXTEND (OneTimeSwitch, Switch)
  {
   public:
    OneTimeSwitch (nsEntity::ConfigMap &m, nsEntity::Entity *w): Switch (m, w)
    {
    }
    bool HandleInput (int key, bool pressed)
    {
      if (State)
	return 0;
      return Switch::HandleInput (key, pressed);
    }
  };

  EXTEND (RolloverSwitch, OneTimeSwitch)
  {
    int delta;
   public:
    RolloverSwitch (nsEntity::ConfigMap &m, nsEntity::Entity *w): OneTimeSwitch (m, w)
    {
      world->Register (this, ::World::_world::SWITCHABLE, 0);
      nextthink = -1;
      initialize_int (delta);
      if (!delta)
	delta = 50;
    }
    int think ()
    {
      world->Action (this, ::World::_world::FIND, x, y, w, h, 1);
      if (world->Found)
      {
	HandleInput (TOGGLE, 1);
	HandleInput (TOGGLE, 0);
      }
      nextthink = delta;
      return 1;
    }
  };

  EXTEND (Gate, Switch)
  {
    enum { AND, NAND, OR, NOR, XOR, EQU, RANDXOR };
    int gtype;
    bool ready;
    std::string targeted_by;
    std::string target;
   public:
    Gate (nsEntity::ConfigMap &m, nsEntity::Entity *w): Switch (m, w),
      initlist_string (target),
      initlist_string (targeted_by)
    {
      std::string initialize_string (gatetype);
      gtype = XOR;
      if (gatetype == "AND")
	gtype = AND;
      if (gatetype == "NAND")
	gtype = NAND;
      if (gatetype == "OR")
	gtype = OR;
      if (gatetype == "NOR")
	gtype = NOR;
      if (gatetype == "XOR")
	gtype = XOR;
      if (gatetype == "EQU")
	gtype = EQU;
      if (gatetype == "RANDXOR")
	gtype = RANDXOR;
      std::string remaining = targeted_by + ";";
      int n = 0;
      while (remaining != "")
      {
	remaining = remaining.substr (remaining.find (";") + 1);
	++n;
      }
      ready = 0;
      initialize_int (nextthink);
      if (!nextthink)
	nextthink = -1;
      // time to initialization
    }
    int think ()
    {
      // not again, just once
      HandleInput (TOGGLE, 1);
      ready = 1;
      // just to load the state into State
      return 1;
    }
    bool HandleInput (int key, bool pressed)
    {
      if (!pressed)
	return 0;
      if ((key == TOGGLE) || (key == SWITCHON) || (key == SWITCHOFF))
      {
	bool OldState = State;
	switch (gtype)
	{
	 case AND:
	 case NOR:
	 case EQU:
	  State = 1;
	  break;
	 case NAND:
	 case OR:
	 case XOR:
	  State = 0;
	  break;
	 case RANDXOR:
	  State = rand () % 2;
	  break;
	}
	std::string current;
	std::string remaining = targeted_by + ";";
	while (remaining != "")
	{
	  current = remaining.substr (0, remaining.find (";"));
	  remaining = remaining.substr (remaining.find (";") + 1);
	  nsEntity::Entity *e = world->FindTarget (current);
	  Switch *sw;
	  if (e && (sw = dynamic_cast <Switch *> (e)))
	  {
	    bool s = sw->State;
	    switch (gtype)
	    {
	     case AND:
	      State = State && s;
	      break;
	     case NAND:
	      State = !(!State && s);
	      break;
	     case OR:
	      State = State || s;
	      break;
	     case NOR:
	      State = !(!State || s);
	      break;
	     case XOR:
	     case RANDXOR:
	      State = State != s;
	      break;
	     case EQU:
	      State = State == s;
	      break;
	    }
	  }
	}
	if (State != OldState && ready)
	{
	  remaining = target + ";";
	  while (remaining != "")
	  {
	    current = remaining.substr (0, remaining.find (";"));
	    remaining = remaining.substr (remaining.find (";") + 1);
	    nsEntity::Entity *e = world->FindTarget (current);
	    if (e)
	    {
	      e->HandleInput (TOGGLE, 1);
	      e->HandleInput (TOGGLE, 0);
	    }
	  }
	}
      }
      return 0;
    }
  };

  OBJECT (Timer)
  {
    std::vector <std::pair <int, std::string> > TimeTable;
    int TTIndex;
    bool off;
   public:
    Timer (nsEntity::ConfigMap &m, nsEntity::Entity *w): Entity (m, w)
    {
      std::string initialize_string (delays) + ";";
      std::string initialize_string (targets) + ";";
      while (targets != "")
      {
	TimeTable.push_back (
	    std::pair <int, std::string>
	    (
	     str2int (delays.substr (0, delays.find (";"))),
	     targets.substr (0, targets.find (";"))
	    )
	    );
	delays = delays.substr (delays.find (";") + 1);
	targets = targets.substr (targets.find (";") + 1);
      }
      initialize_int (off);
      TTIndex = 0;
      if (TimeTable.size() && !off)
	nextthink = TimeTable[0].first;
    }
    int think ()
    {
      nsEntity::Entity *e = world->FindTarget (TimeTable[TTIndex].second);
      ++TTIndex %= TimeTable.size ();      // cannot be 0 because of the if in
      // the constructor, so this is safe
      nextthink = TimeTable[TTIndex].first;
      if (!nextthink)
	off = 1;

      // this is here to avoid chain reactions which make the timer not stop
      if (e)
      {
	e->HandleInput (TOGGLE, 1);
	e->HandleInput (TOGGLE, 0);
      }
      return 1;
    }
    bool HandleInput (int key, bool pressed)
    {
      if ((key != TOGGLE && key != SWITCHON && key != SWITCHOFF) || !pressed)
	return 0;
      if (!off)
	nextthink = 0;     // stop timer
      else
      {
	TTIndex = 0;
	if (!TimeTable.empty())
	  nextthink = TimeTable[0].first;
      }
      off = !off;
      return 0;
    }
  };

  OBJECT (Switcher)
  {
    bool State;
    std::string target;
    nsEntity::Entity *Target;
    bool doit;
    bool switching;
   public:
    Switcher (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity(m, w),
      initlist_string (target)
    {
      State = 0;
      Target = 0;
      bool initialize_int(vuln);
      world->Register (this, ::World::_world::KILLABLE, vuln);
      world->Register (this, ::World::_world::NOGRAVITY, 1);
      switching = 0;
    }
    bool HandleInput (int key, bool pressed)
    {
      if (switching)
	return 0;
      if (!pressed)
	return 0;
      switching = 1;
      bool OldState = State;
      if (key == TOGGLE)
	State = !State;
      if (key == SWITCHON)
	State = 1;
      if (key == SWITCHOFF)
	State = 0;
      if (State != OldState)
      {
	world->Switch (x, y, w, h);
	if (target != "")
	  if ((Target = world->FindTarget (target)))
	  {
	    Target->HandleInput (TOGGLE, 1);
	    Target->HandleInput (TOGGLE, 0);
	  }
      }
      switching = 0;
      return 1;
    }
    bool Homicide ()
    {
      if (switching)
	return 0;
      world->Register (this, ::World::_world::KILLABLE, 0);
      HandleInput (TOGGLE, 1);
      HandleInput (TOGGLE, 0);
      world->kill (this);
      return 1;
    }
  };

  typedef Switcher SwitchBox;

  BEGIN_OBJECTS_LIST (AddGates)

    ADD_ENTITY_TYPE (Switch);
  /*
     string on_sound = "sw-on.wav";
     string off_sound = "sw-off.wav";
     string target;
     bool nogfx = 0;
     */

  ADD_ENTITY_TYPE (OneTimeSwitch);
  /*
     : public Switch
     */

  ADD_ENTITY_TYPE (RolloverSwitch);
  /*
     : public OneTimeSwitch
     int delta = 50;
     */

  ADD_ENTITY_TYPE (Gate);
  /*
     : public Switch
     string targeted_by; // SDF string
     string target;
     string gatetype = "XOR";
  // AND NAND OR NOR XOR EQU RANDXOR
  string targetname;
  */

  ADD_ENTITY_TYPE (Switcher);
  /*
     string targetname;
     bool vuln = 0;
     */

  ADD_ENTITY_TYPE (SwitchBox);
  /*
     == Switcher
     */

  ADD_ENTITY_TYPE (Timer);
  /*
     string delays;      // SDF string
     string targets;     // SDF string
     bool off = 0;
     string targetname;
     */

  END_OBJECTS_LIST
}
