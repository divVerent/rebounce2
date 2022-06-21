#define __RP__OBJECTS_CPP__

#include "objects.h"

#include "entity.h"
#include "world.h"
#include "sprite.h"
#include "alleg.h"
#include "animate.h"
#include "enemies.h"

#include <math.h>

namespace Objects
{
  void throw_bomb (nsEntity::Entity *thrower, int radius, int time, bool visible, bool sticky)
  {
    nsEntity::ConfigMap m;
    m["x"] = int2str (thrower->x);
    m["y"] = int2str (thrower->y);
    m["dx"] = int2str (thrower->dx >> 8);
    m["dy"] = int2str (thrower->dy >> 8);
    m["w"] = int2str (thrower->w);
    m["h"] = int2str (thrower->h);
    m["type"] = "Bomb";
    m["radius"] = int2str (radius);
    m["sprnum"] = visible ? "0" : "-1";
    m["time"] = int2str (time);
    m["sticky"] = int2str (sticky);
    m["bnc_speed"] = "800";
    thrower->world->spawn (m);
  }

  void throw_nitroglycerine (nsEntity::Entity *thrower, int x, int y, int dx, int dy, int radius, int explradius, int sprnum, int groupnum)
  {
    nsEntity::ConfigMap m;
    m["x"] = int2str (thrower->x + thrower->w / 2 + x - radius);
    m["y"] = int2str (thrower->y + thrower->h / 2 + y - radius);
    m["dx"] = int2str (dx);
    m["dy"] = int2str (dy);
    m["w"] = int2str (radius * 2 + 1);
    m["h"] = int2str (radius * 2 + 1);
    m["type"] = "Nitroglycerine";
    m["expl"] = int2str (explradius);
    m["touchexpl"] = int2str (explradius);
    m["sprnum"] = int2str (sprnum);
    m["groupnum"] = int2str (groupnum);
    thrower->world->spawn (m);
  }

  int const SPR_BOMB = 55;

  OBJECT (Bomb)
  {
    bool start;
    int radius;
    bool sticky;
   public:
    Bomb (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity (m, w)
    {
      teleportable = 1;
      z = 17;
      if (!sprnum && !groupnum)
      {
	sprnum = SPR_BOMB;
	groupnum = 0;
      }
      initialize_int (radius);
      if (radius > 32)
	radius = 32;
      int time;
      initialize_int (time);
      initialize_int (sticky);
      nextthink = time ? time : 3000;
      world->Register (this, ::World::_world::KILLABLE, 1);
      mysprite = new ::Sprite::Sprite (create_optimized_bitmap (64, 64), 1);
      clear_to_color (*mysprite, bitmap(*mysprite)->vtable->mask_color);
      start = 1;
    }
    int think ()
    {
      Homicide ();
      return 1;
    }
    static void KillIt (nsEntity::Entity *Actor, nsEntity::Entity *Victim)
    {
      Bomb *me;
      if (!(me = dynamic_cast <Bomb *> (Actor)))
	return;
      if (!(Victim->flags & ::World::_world::KILLABLE))
	return;
      // Approximate ellipse-circle collision.
      int XDiff = me->x + me->w / 2 - Victim->x - Victim->w / 2;
      int YDiff = me->y + me->h / 2 - Victim->y - Victim->h / 2;
      if (hypot (XDiff, YDiff) < me->radius + std::min (Victim->w, Victim->h) / 2)
	Victim->Homicide ();
    }
    bool HandleInput (int key, bool pressed)
    {
      switch (key)
      {
       case TELEPORT:
	if (!pressed)
	  break;
	world->PlaySound (this, "teleport.wav", 128, 1);
	break;
      }
      return 0;
    }
    bool Homicide ()
    {
      teleportable = 0;
      bool first = (flags & ::World::_world::KILLABLE) != 0;
      if (first)
      {
	nextthink = 50;
	world->Register (this, ::World::_world::KILLABLE, 0);
      }

      int w2 = 2 * radius;
      int h2 = 2 * radius;
      _x += (w - w2) << 7;
      _y += (h - h2) << 7;         // center explosion
      x = _x >> 8;
      y = _y >> 8;
      w = w2;
      h = h2;

      if (radius < 4)
	world->kill (this);
      else
      {
	groupnum = world->NamedGroup ("expl.pcx", 64, 64);

	sprnum = -1;

	clear_to_color (*mysprite, bitmap(*mysprite)->vtable->mask_color);

	masked_stretch_blit (::Sprite::Sprites [groupnum] [0],
	    *mysprite,
	    0, 0, 64, 64,
	    0, 0, 2 * radius, 2 * radius);

	radius -= 4;

	if (start)
	  world->PlaySound (this, "expl.wav", 128, 1);

	world->Action (this, KillIt, x, y, w, h);
      }

      start = 0;
      nextthink = 50;
      return 1;
    }
    void HandleFlags (int f)
    {
      if (sticky && f & COLLIDES)
      {
	dx = 0;
	dy = 0;
	world->Register (this, World::_world::NOGRAVITY, 1);
      }
    }
    ~Bomb ()
    {
      if (mysprite)
	delete mysprite;
    }
  };

  OBJECT (Enemy)
  {
   protected:
    int expl, delta;
    int touchexpl;
    std::string target;
   public:
    Enemy (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity (m, w),
      initlist_string (target),
      initlist_int (delta),
      initlist_int (expl),
      initlist_int (touchexpl)
    {
      z = 16;
      int initialize_int (invuln);
      int initialize_int (grave);
      if (!delta)
	delta = 40;
      world->Register (this, ::World::_world::KILLABLE, !invuln);
      world->Register (this, ::World::_world::NOGRAVITY, !grave);
      nextthink = delta;
    }

    int think ()
    {
      if ((world->Action (this, ::World::_world::FIND, x, y, w, h, 1), world->Found))
      {
	if (touchexpl)
	{
	  expl = touchexpl;
	  Homicide ();
	}
      }
      world->Action (this, ::World::_world::KILL, x, y, w, h);
      nextthink = delta;
      return 0;
    }

    bool Homicide ()
    {
      if (expl)
	throw_bomb (this, expl, -1, 0);
      nsEntity::Entity *Target;
      if ((target != "") && (Target = world->FindTarget (target)))
      {
	Target->HandleInput (TOGGLE, 1);
	Target->HandleInput (TOGGLE, 0);
	target = "";
      }
      world->kill (this);
      return 1;
    }
  };


  EXTEND (Nitroglycerine, Enemy)
  {
   public:
    Nitroglycerine (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Enemy (m, w)
    {
      if (!expl)
	expl = 32;
      if (!touchexpl)
	touchexpl = 32;
    }
    void HandleFlags (int f)
    {
      if (f & COLLIDES)
	Homicide ();
    }
  };

  EXTEND (Electrocuter, Enemy)
  {
    int on;
    int _sprnum;
    std::string sprnums;
    int numspr;
    std::string target;
   public:
    Electrocuter (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Enemy (m, w)
    {
      int initialize_int (vuln);
      initialize_string (target);
      world->Register (this, ::World::_world::KILLABLE, !!vuln);
      initialize_int (numspr);
      initialize_string (sprnums);
      for (int i = 1; i <= numspr; ++i)
      {
	if (sprnums != "")
	  sprnums += ";";
	sprnums += int2str (i + sprnum);
      }
      _sprnum = sprnum;
      initialize_int (on);
      SetSprite ();
    }
    void SetSprite ()
    {
      if (on)
	sprnum = Animate (sprnums);
      else
	sprnum = _sprnum;
    }
    int think ()
    {
      if (on)
	Enemy::think ();
      SetSprite ();
      nextthink = delta;
      return 0;
    }
    bool HandleInput (int key, bool pressed)
    {
      if (!pressed)
	return 0;
      bool OldState = !!on;
      if (key == SWITCHON)
	on = 1;
      if (key == SWITCHOFF)
	on = 0;
      if (key == TOGGLE)
	on = !on;
      if (on && !OldState)
	world->PlaySound (this, "SCHUSS.WAV", 64, 1);     // positional!
      if (!OldState != !on)
      {
	nsEntity::Entity *Target;
	if ((target != "") && (Target = world->FindTarget (target)))
	{
	  Target->HandleInput (TOGGLE, 1);
	  Target->HandleInput (TOGGLE, 0);
	}
      }
      SetSprite ();
      return 0;
    }
  };

  EXTEND (Mover, Enemy)
  {
    int timer, delay, TimerRunning, speed;
    std::string targets;
   public:
    Mover (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Enemy (m, w)
    {
      initialize_int (timer);
      initialize_int (delay);
      initialize_string (targets);
      initialize_int (speed);
      int initialize_int (vuln);
      world->Register (this, ::World::_world::KILLABLE, !!vuln);
      if (timer)
	TimerRunning = 1;
    }
    int think ()
    {
      Enemy::think ();
      if (!TimerRunning)
      {
	std::string tmp = targets;
	nsEntity::Entity *Target = world->FindTarget (AnimateString (tmp = targets));
	if (Target)
	{
	  int xdistance = Target->x - x;
	  int ydistance = Target->y - y;
	  if (ABS(xdistance) + ABS(ydistance) <= speed) // i am done
	  {
	    x = Target->x;
	    y = Target->y;
	    TimerRunning = 1;
	    timer = delay;
	    targets = tmp;
	  }
	  else
	  {
	    x += xdistance * speed / (ABS(xdistance) + ABS(ydistance));
	    y += ydistance * speed / (ABS(xdistance) + ABS(ydistance));
	  }
	  _x = x << 8;
	  _y = y << 8;
	}
      }
      else
      {
	timer -= nextthink;
	if (timer < 0)
	  TimerRunning = 0;
      }
      return 0;
    }
  };


  OBJECT (Spawner)
  {
    nsEntity::ConfigMap map;
    void doit ()
    {
      world->spawn (map);
    }
   public:
    Spawner (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity (m, w), map(m)
    {
      map["type"] = "Enemy";
      map["target"] = targetname;
      map["targetname"] = "";

      for (nsEntity::ConfigMap::iterator i = m.begin(); i != m.end(); ++i)
      {
	if (i->first.find ("spawn") == 0)
	{
	  map.erase (map.find (i->first));
	  map[i->first.substr (std::string("spawn").length())] = i->second;
	}
      }
      if (m["spawnx"] != "")
	map["x"] = int2str (str2int (map["x"]) + str2int (m["x"]));
      if (m["spawny"] != "")
	map["y"] = int2str (str2int (map["y"]) + str2int (m["y"]));

      world->Register (this, ::World::_world::NOGRAVITY, 1);
      if (str2int (m["immed"]))
	doit ();
    }
    bool HandleInput (int key, bool pressed)
    {
      if (!pressed)
	return 0;
      if (key == TOGGLE)
	doit ();
      return 1;
    }
  };


  EXTEND (Boss, Enemy)
  {
   protected:
    bitmap hud;
    int time;
    int ictr;
    int lives;
    int blives;
    int itime;
    int sprstartnum;
    bool has_second;
    int nsize;
    int nexpl;
    int nsprnum;
    int ngroupnum;
    int d;
    enum
    {
      STANDING = 0,
      LEFT = 1,
      RIGHT = 2,
      UP = 3,
      DOWN = 4,
      SHOOTING = 5,
      DYING = 6
    };
   public:

    virtual void strategy() = 0;

    Boss (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Enemy (m, w)
    {
      nextthink = -1;
      time = 0;
      initialize_int (lives);
      blives = lives;
      initialize_int (has_second) != 0;
      initialize_int (itime);
      sprstartnum = sprnum;
      ictr = 0;
      initialize_int (nsize);
      initialize_int (nexpl);
      initialize_int (nsprnum);
      initialize_int (ngroupnum);
      initialize_int (d);
      hud = world->get_hud (2);
    }
    ~Boss ()
    {
      if (hud)
	world->give_back_hud (2, hud);
    }
    virtual void shoot (int nx, int ny, int dx, int dy)
    {
      throw_nitroglycerine (this, nx, ny, dx, dy, nsize, nexpl, nsprnum, ngroupnum);
    }
    int think ()
    {
      Enemy::think ();
      time += delta;

      sprnum = sprstartnum;
      strategy ();

      if (ictr > 0)
      {
	ictr -= delta;
	sprnum = sprstartnum + DYING;
      }
      else
      {
	if (sprnum == sprstartnum)
        {
	  if ((dx > d) && (ABS(dy) < dx))
	    sprnum += RIGHT;
	  else if ((-dx > d) && (ABS(dy) < -dx))
	    sprnum += LEFT;
	  else if ((dy > d) && (ABS(dx) < dy))
	    sprnum += DOWN;
	  else if ((-dy > d) && (ABS(dx) < -dy))
	    sprnum += UP;
	  else
	    sprnum += STANDING;
        }
      }

      if (hud)
      {
	int lmax = blives + 1;
	int l = lives + 1;
	int x = (hud->w-1) * l / lmax;
	rectfill (hud, 0, 0, x, hud->h-1, makecol (255, 0, 0));
	rectfill (hud, x, 0, hud->w-1, hud->h-1, makecol (0, 255, 0));
      }

      return 0;
    }
    bool Homicide ()
    {
      if (ictr > 0)
	return 0;
      if (!lives--)
      {
	if (hud)
	  rectfill (hud, 0, 0, hud->w-1, hud->h-1, makecol (0, 255, 0));
	Enemy::Homicide ();
      }
      else
	ictr = itime;
      return 1;
    }
  };

  EXTEND (Boss1, Boss)
  {
    std::string ospr;
   protected:
    int movex, movey, nx, ny, ndx, ndy, prob;
   public:
    Boss1 (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Boss (m, w)
    {
      initialize_int (movex);
      initialize_int (movey);
      initialize_int (nx);
      initialize_int (ny);
      initialize_int (ndx);
      initialize_int (ndy);
      initialize_int (prob);
      if (!prob)
	prob = 1000 / delta;
      if (!itime)
	itime = 500;
      initialize_string (ospr);
      groupnum = world->NamedGroup (ospr, this->w, this->h);
      sprnum = 0;
      sprstartnum = 0;

    }
    void strategy ()
    {
      // random movement
      ddx = (Random(3) - 1) * movex;
      ddy = (Random(3) - 1) * movey;
      // random shooting
      if (!Random(prob))
      {
	shoot (nx, ny, ndx, ndy);
	sprnum = sprstartnum + SHOOTING;
      }
    }
  };

  EXTEND (Boss2, Boss1)
  {
   protected:
    int intellegx;
    int intellegy;
    int seemex;
    int seemey;
    int nx_vary;
    int ny_vary;
    int ndx_vary;
    int ndy_vary;
    int r;
   public:
    Boss2 (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Boss1 (m, w)
    {
      initialize_int (intellegx);
      initialize_int (intellegy);
      initialize_int (seemex);
      initialize_int (seemey);
      initialize_int (r);
      initialize_int (nx_vary);
      initialize_int (ny_vary);
      initialize_int (ndx_vary);
      initialize_int (ndy_vary);
      if (!r)
	r = 900;
    }
    void strategy ()
    {
      // random movement
      // random shooting
      world->Action (this, World::_world::FIND, x - seemex, y - seemey, w + 2 * seemex, h + 2 * seemey, 1);
      if (world->Found)
      {
	int playerx = world->Found->x + world->Found->w / 2;
	int playery = world->Found->y + world->Found->h / 2;
	int myx = x + w / 2;
	int myy = y + h / 2;
	int fx = (myx < playerx) ? 1 : -1;
	int fy = (myy < playery) ? 1 : -1;
	ddx = intellegx * fx;
	ddy = intellegy * fy;
	if (!Random(prob))
	{
	  int vx = - nx_vary + Random(2 * nx_vary + 1);
	  int vy = - ny_vary + Random(2 * ny_vary + 1);
	  int _dx = playerx - myx - nx - vx;
	  int _dy = playery - myy - ny - vy;
	  if (nx)
	    _dx = int (fabs (_dx) * SGN (nx));
	  if (ny)
	    _dy = int (fabs (_dy) * SGN (ny));
	  int nabs = int (hypot (ndx, ndy));
	  int _dabs = int (hypot (_dx, _dy));
	  if (_dabs)
	  {
	    shoot (nx + vx, ny + vy,
		_dx * nabs / _dabs, _dy * nabs / _dabs);
	    sprnum = sprstartnum + SHOOTING;
	  }
	}
      }
      else
      {
	ddx = (Random(3) - 1) * movex;
	ddy = (Random(3) - 1) - movey;
	if (!Random(prob))
	{
	  shoot (nx - nx_vary + Random(2 * nx_vary + 1), ny - ny_vary + Random(2 * ny_vary + 1),
	      ndx - ndx_vary + Random(2 * ndx_vary + 1), ndy - ndy_vary + Random(2 * ndy_vary + 1));
	  sprnum = sprstartnum + SHOOTING;
	}
      }
      dx *= r;
      dx /= 1000;
      dy *= r;
      dy /= 1000;
    }
  };

  EXTEND (Boss3, Boss2)
  {
    std::string nsprnums;
    int bigprob;
   public:
    Boss3 (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Boss2 (m, w),
      initlist_string (nsprnums),
      initlist_int (bigprob)
    {
    }
    void shoot (int nx, int ny, int dx, int dy)
    {
      int nsprnum = Animate (nsprnums);
      throw_nitroglycerine (this, nx, ny, dx, dy, nsize, Random(bigprob) ? nexpl : 31, nsprnum, ngroupnum);
    }
    /*
       void strategy ()
       {
    // nothing yet
    }
    */
  };


  BEGIN_OBJECTS_LIST (AddEnemies)

    ADD_ENTITY_TYPE (Enemy);
  /*
     string target;
     int delta = 40;
     int expl = 0;
     int touchexpl = 0;
     bool invuln = 0;
     bool grave = 0;
     */

  ADD_ENTITY_TYPE (Nitroglycerine);
  /*
     : public Enemy
     */

  ADD_ENTITY_TYPE (Mover);
  /*
     : public Enemy
     DO NOT set grave!
     DO NOT set ddx/ddy!
     int delay = 0;
     int timer = 0;
     string targets;     // SDF
     invuln is ignored, use int vuln = 0;
     */

  ADD_ENTITY_TYPE (Electrocuter);
  /*
     : public Enemy;
     visible and killing only if on
     animated if sprnums contains a ;
     int numspr = 0;
  // if numspr == 0, use sprnums sprites. Otherwise the first numspr sprites after sprnum
  string sprnums;
  string targetname;
  string target; // next in chain
  invuln is ignored, use int vuln = 0;
  */

  ADD_ENTITY_TYPE (Boss1);

  ADD_ENTITY_TYPE (Boss2);

  ADD_ENTITY_TYPE (Boss3);

  ADD_ENTITY_TYPE (Spawner);
  /*
     string targetname;
     bool immed = 0;
     int spawnx = 0;
     int spawny = 0;
     */

  ADD_ENTITY_TYPE (Bomb);
  /*
  // NOT FOR USE IN EDITOR
  int radius = 16;
  int time = 3000;
  */

  END_OBJECTS_LIST
}
