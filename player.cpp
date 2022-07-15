#define __RP__OBJECTS_CPP__

#include "objects.h"

#include "entity.h"
#include "world.h"
#include "border.h"
#include "sprite.h"
#include "alleg.h"
#include "player.h"

#include <stdlib.h>

#include "enemies.h"

namespace Objects
{

  /**************************************************************************
   *                                                                        *
   * The Player object. Accepts keys and is vulnerable by default.          *
   *                                                                        *
   **************************************************************************/

  int const SPR_EXTRALIFE = 46;
  int const SPR_BOMBS = 56;
  int const SPR_RADIUS = 57;
  int const SPR_TIME = 58;
  int const SPR_ENERGY = 59;

  OBJECT (Player)
  {
    static int toggled;
    static void Toggle (nsEntity::Entity *me, nsEntity::Entity *ent)
    {
      if (ent->flags & ::World::_world::SWITCHABLE)
      {
	toggled = 1;
	ent->HandleInput (TOGGLE, 1);
	ent->HandleInput (TOGGLE, 0);
      }
    }

    int accel, limit;
    bool teleporting;
    int r;
    bitmap my_hud;
    bool left, right, up, down, e;
    int efactor;
    int savesprnum;
   public:
    int numbombs;
    int radius;
    int maxbombs;
    int num;
    int maxenergy;
    int energy;
    int loser;
    typedef std::vector<std::pair<int, int> > lives_t;
    lives_t lives;
    Player (nsEntity::ConfigMap &m, nsEntity::Entity *wld) : Entity (m, wld)
      , up(0), down(0), left(0), right(0), e(0), loser(0), savesprnum(sprnum)
      {
	teleportable = 1;
	Border (0, 255, 0);
	// additional things for a player
	initialize_int (maxenergy);
	initialize_int (efactor);
	if (!efactor)
	  efactor = 100;
	energy = 0;
	initialize_int (num);
	if (!num)
	  num = 1;
	initialize_int (accel);
	if (!accel)
	  accel = 200000;
	initialize_int (limit);
	if (!limit)
	  limit = 600000;
	initialize_int (r);
	if (!r)
	  r = 900;
	// wg. Aenderung von nextthink:
	// r = sqrt (r * 1000.0); funktioniert mit DJGPP nicht
	r = (r + 1000) / 2;       // grobe N„herung
	world->Register (this, World::_world::PLAYER2, num == 2);
	world->Register (this, World::_world::INPUT, 1);
	world->Register (this, World::_world::KILLABLE, 1);
	nextthink = -1;
	z = 15;
	teleporting = 0;
	bnc_speed = 800;
	numbombs = 0;
	radius = 8;
	maxbombs = 0;
	my_hud = world->get_hud (num);

	// Outer pixel row is just border.
	offsetx -= 1;
	offsety -= 1;
	w -= 2;
	h -= 2;
	x += 1;
	y += 1;
	_x += 1 << 8;
	_y += 1 << 8;
      }

    ~Player ()
    {
      world->give_back_hud (num, my_hud);
      Border ();
    }

    bool HandleInput (int key, bool pressed)
    {
      switch (key)
      {
       case CHEAT:
	world->Register (this, World::_world::KILLABLE, !pressed);
	break;
       case UP:
	up = pressed;
	if (up && jumpnrun && floor && !teleporting && !(flags & ::World::_world::DISABLED))
	{
	  world->PlaySound (this, "sprung.wav", 128, 1);
	  dy = -jumpnrun;   // nach OBEN
	  floor = 0;
	}
	if (jumpnrun)
	  up = 0;
	break;
       case DOWN:
	down = pressed;
	if (jumpnrun)
	  down = 0;
	break;
       case LEFT:
	left = pressed;
	break;
       case RIGHT:
	right = pressed;
	break;
       case BUTTONA:
	e = pressed;
	break;
       case TOGGLE:
	if (flags & ::World::_world::DISABLED)
	  break;
	if (pressed)
	{
	  toggled = 0;
	  world->ActionBox (this, Toggle, x, y, w, h);
	  if (!toggled)
	  {
	    if (numbombs)
	    {
	      --numbombs;
	      throw_bomb (this, radius, 1000 + rand() % 500, 1, !!jumpnrun);
	    }
	  }
	}
	break;
       case TELEPORT:
	Border (0, pressed ? 0 : 255, pressed ? 255 : 0);
	world->Register (this, World::_world::DISABLED, pressed);
	teleporting = pressed;
	if (!pressed)
	  break;
	world->PlaySound (this, "teleport.wav", 128, 1);
	nextthink = 300;
	teleporting = 1;
	break;
       case SUICIDE:
	Homicide ();
	break;
       default:
	return 0;
      }
      return 1;
    }

    bool Homicide ()
    {
      if ((flags & ::World::_world::DISABLED))
	return 0;
      world->Register (this, ::World::_world::DISABLED, 1);
      world->Register (this, ::World::_world::KILLABLE, 0);
      if (!world->iendtimer)
	world->iendtimer = world->itimer;
      if (flags & ::World::_world::EXITTED)
      {
        world->StopMusic ();
	Border (255, 255, 255);
	world->PlaySound (this, "super.wav", 128, 1);
	nextthink = 2500;
      }
      else
      {
        if(lives.empty())
          world->StopMusic ();
	Border (255, 0, 0);
	throw_bomb (this, 32, -1, 0);
	nextthink = 1300;
	loser = 1;
	sprnum = -1;
      }
      return 0;
    }

    void HandleFlags (int flags)
    {
      this->Entity::HandleFlags (flags);
      if (jumpnrun && flags & FLOOR)
      {
	dy = 0;
	dx = 0;
	world->PlaySound (this, "boden.wav", 128, 1);
      }
      else if (flags & COLLIDES && !floor)
	world->PlaySound (this, "wand.wav", 128, 1);
    }

    int timer;

    void think(int remaining)
    {
      nextthink = 1;  // Think every frame.
      if (teleporting)
      {
	HandleInput (TELEPORT, 0);
	x = _x >> 8;
	y = _y >> 8;
	return;
      }
      if (flags & ::World::_world::DISABLED)
      {
	if (loser)
        {
          if(!lives.empty())
          {
            // unkill
            world->Register (this, ::World::_world::DISABLED, 0);
            world->Register (this, ::World::_world::KILLABLE, 1);
            world->iendtimer = 0;
            Border (0, 255, 0);
            loser = 0;
            sprnum = savesprnum;
            std::pair<int, int> xy = lives.back();
	    lives.pop_back();
            x = xy.first - w/2;
            y = xy.second - h/2;
	    _x = x << 8;
	    _y = y << 8;
	    dx = 0;
	    dy = 0;
	    world->PlaySound (this, "respawn.wav", 128, 1);
	    return;
          }
	  world->PlaySound (this, "gelost.wav", 128, 1);
        }
	world->kill (this);
	return;
      }

      int speed = 0;

      if (e && (energy > 0))
      {
	speed = accel * (100 + energy * efactor / maxenergy) / 100;
	energy -= 10;
	if (energy < 0)
	  energy = 0;
      }
      else
      {
	speed = accel;
	if (++energy > maxenergy)
	  energy = maxenergy;
      }

      if (jumpnrun)
	dx = (int(right) - int(left)) * speed;
      else
      {
	ddx = (int(right) - int(left)) * speed;
	ddy = (int(down) - int(up)) * speed;
	dx *= r;
	dx /= 1000;
	dx = (dx > limit) ? limit : dx;
	dx = (dx < -limit) ? -limit : dx;
      }
      dy *= r;
      dy /= 1000;
      dy = (dy > limit) ? limit : dy;
      dy = (dy < -limit) ? -limit : dy;
      if (numbombs == maxbombs)
	timer = 0;
      else
	if (++timer * maxbombs >= 100)
	{
	  ++numbombs;
	  timer = 0;
	}

      if (my_hud)
      {
	clear_to_color (my_hud, makecol (0, 0, 255));

	// 32px: bombs
	// 16px: spacer
	// 32px: radius
	// 16px: spacer
	// 112px: time
	// 16px: spacer
	// 32px: lives
	// 16px: spacer
	// 48px: energy

	// display the bombs
	if (maxbombs)
	{
	  ::Sprite::Sprites[0][SPR_BOMBS].draw (my_hud, 0, (HUD - SPRHEIGHT) / 2);
	  int i = 1;
	  for (; i <= 8; ++i)
	  {
	    vline (my_hud, 16 + i * 2 - 1, 1, HUD-2, makecol ((maxbombs >= i) ? 255 : 0, (numbombs >= i) ? 255 : 0, 0));
	  }
	  ::Sprite::Sprites[0][SPR_RADIUS].draw (my_hud, 48, (HUD - SPRHEIGHT) / 2);
	  for (i = 1; i <= 8; ++i)
	  {
	    vline (my_hud, 64 + i * 2 - 1, 1, HUD-2, makecol (0, (radius / 4 >= i) ? 255 : 0, 0));
	  }
	}
	if (world->timelimit)
	{
	  ::Sprite::Sprites[0][SPR_TIME].draw (my_hud, 96, (HUD - SPRHEIGHT) / 2);
	  for (int i = 1; i <= 48; ++i)
	  {
	    vline (my_hud, 112 + i * 2 - 1, 1, HUD-2, makecol (0, (world->itimer * 48 <= world->timelimit * (48-i)) ? 255 : 0, 0));
	  }
	}
	if (!lives.empty())
	{
	  ::Sprite::Sprites[0][SPR_EXTRALIFE].draw (my_hud, 224, (HUD - SPRHEIGHT) / 2);
	  for (int i = 1; i <= 8; ++i)
	  {
	    vline (my_hud, 240 + i * 2 - 1, 1, HUD-2, makecol (0, (lives.size() >= i) ? 255 : 0, 0));
	  }
	}
	if (maxenergy)
	{
	  ::Sprite::Sprites[0][SPR_ENERGY].draw (my_hud, 272, (HUD - SPRHEIGHT) / 2);
	  for (int i = 1; i <= 16; ++i)
	  {
	    vline (my_hud, 288 + i * 2 - 1, 1, HUD-2, makecol (0, (energy * 16 >= maxenergy * i) ? 255 : 0, 0));
	  }
	}
      }
      else
	my_hud = world->get_hud (num);
    }
  };

  int Player::toggled = 0;

  OBJECT (Item)
  {
    bool disabled;
   public:
    Item (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Entity (m, w)
    {
      world->Register (this, ::World::_world::NOGRAVITY, 1);
      world->Register (this, ::World::_world::KILLABLE, 1);
      nextthink = 100;
      disabled = 0;
    }
    virtual void take_me (Player *pl) = 0;
    static void doit (nsEntity::Entity *e, nsEntity::Entity *him)
    {
      Item *me;
      if (!(me = dynamic_cast<Item *>(e)))
	return;
      if (me->disabled)
	return;
      if (Player *pl = dynamic_cast<Player *>(him))
      {
	me->take_me (pl);
	me->Homicide ();
      }
    }
    void think (int remaining)
    {
      world->ActionBox (this, doit, x, y, w, h);
      nextthink = 100;
    }
    bool Homicide ()
    {
      disabled = 1;
      world->PlaySound (this, "BLUB.WAV", 128, 1);
      world->kill (this);
      return 1;
    }
  };

  EXTEND (Item_Bomb, Item)
  {
   public:
    Item_Bomb (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Item (m, w)
    {
    }
    void take_me (Player *pl)
    {
      ++pl->maxbombs;
      ++pl->numbombs;
    }
  };

  EXTEND (Item_Radius, Item)
  {
   public:
    Item_Radius (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Item (m, w)
    {
    }
    void take_me (Player *pl)
    {
      pl->radius += 4;
    }
  };

  EXTEND (Item_Energy, Item)
  {
    int value;
   public:
    Item_Energy (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Item (m, w)
    {
      initialize_int (value);
    }
    void take_me (Player *pl)
    {
      pl->maxenergy += value;
      pl->energy += value;
    }
  };

  EXTEND (Item_ExtraLife, Item)
  {
   public:
    Item_ExtraLife (nsEntity::ConfigMap &m, nsEntity::Entity *w) : Item (m, w)
    {
    }
    void take_me (Player *pl)
    {
      pl->lives.push_back(std::make_pair(x + w/2, y + h/2));
    }
  };

  BEGIN_OBJECTS_LIST (AddPlayer)

    ADD_ENTITY_TYPE (Player);
  /*
     int num = 1;
     int accel = 200000;
     int limit = 600000;
     int r = 900;
     int maxenergy = 0;
     int efactor = 100;
     int jumpnrun = 0;
  // sonst Speed nach oben bei Pfeil-Hoch-Taste
  // Pfeil-Runter-Taste in diesem Modus wirkungslos!
  */

  ADD_ENTITY_TYPE (Item_Bomb);
  /*
  */

  ADD_ENTITY_TYPE (Item_Radius);
  /*
  */

  ADD_ENTITY_TYPE (Item_Energy);
  /*
     int value = 10;
     */

  ADD_ENTITY_TYPE (Item_ExtraLife);
  /*
  */

  END_OBJECTS_LIST
}
