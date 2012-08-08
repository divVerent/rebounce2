#include "defs.h"

#include <algorithm>
#include <functional>
#include <map>
#include <list>

#include "map.h"
#include "world.h"
#include "coll.h"
#include "keys.h"
#include "sound.h"
#include "bet_find.h"
#include "border.h"


typedef nsEntity::Entity Ent;
typedef ::Sound::Sound Snd;
std::map <Ent *, Snd *> Positional_Sounds;
std::list <Snd *> Independent_Sounds;
// is in fact a queue
// limited to SOUND_LIMIT sounds
Snd *CurrentMIDI = 0;


bool ZOrder (Ent *e1, Ent *e2)
{
  if (!e1)
    return 0;
  if (!e2)
    return 1;
  return e1->z < e2->z;
}


World::_world::_world (nsEntity::ConfigMap &m, nsEntity::Entity *) : Entity (m, 0), target (""), mapname(m["map"]), first_players(0), second_players(0)
{
  // load needed SpriteSets
  // they are stored in "SPRITESET#i", they are "SPRITESETS" many of them
  iendtimer = 0;
  initialize_int (timelimit) * 1000;
  if (!(initialize_int (time_factor)))
    time_factor = 1000;
  initialize_string (target);
  std::string initialize_string (SPRITESET0);
  if (SPRITESET0 == "")
    SPRITESET0 = "BASE.PCX";
  ::IO::ReadSpriteData (Filename(SPRITESET0).c_str(), 0);
  int nSets = str2int(m["SPRITESETS"]);
  for (int i = 1; i <= nSets; ++i)
    ::IO::ReadSpriteData (Filename(m[std::string("SPRITESET") + int2str(i)]).c_str(), i);
}


void World::_world::initworld (::IO::IOEntityContainer &e, bitmap lower, bitmap upper, bool CheatActivated, bool MarkCheater)
{
  if (MarkCheater)
  {
    // FCKGW mode
    ::Sprite::Sprites[0][142] = ::Sprite::Sprites[0][140];
    ::Sprite::Sprites[0][143] = ::Sprite::Sprites[0][141];
    // FCKGW mode
  }

  Silence ();
  timer = 0;
  itimer = 0;
  lower_hud = lower;
  upper_hud = upper;
  _ent = new nsEntity::EntityContainer;
  nsEntity::make_entity_ex_world = this;
  std::transform (e.begin(), e.end(), std::back_inserter(*_ent), nsEntity::make_entity_ex);
  _zlist = new nsEntity::EntityContainer (*_ent);
  std::sort (_zlist->begin(), _zlist->end(), ZOrder);
  _map = new Level::Map (::IO::ReadLevelMapData (Filename(mapname).c_str()));
  _map->cheat = CheatActivated;
  _map->InvalidateCache ();
  multiplayer = !!second_players;
}

class drawer
{
 private:
  bitmap bmp;
  int x;
  int y;
 public:
  drawer (bitmap screen, int _x, int _y) : bmp(screen), x(_x), y(_y)
  {
  }
  void operator () (nsEntity::Entity *ent)
  {
    ent->draw (bmp, x, y);
  }
};

void World::_world::drawall (bitmap screen)
{
  x = _map->scrxpos;
  y = _map->scrypos;
  _map->draw (screen);
  Collision::for_each (*_zlist, drawer (screen, x, y), x, y, screen->w, screen->h);
}

class World::_world::Ticker
{
  _world *w;
  int f;
  bool why;
 public:
  Ticker (_world *we, int framerate, bool wh) : w(we), f(framerate), why(wh)
  {
    w->s_x = 0;
    w->s_y = 0;
    w->s_n = 0;
  };
  void operator () (nsEntity::Entity *e)
  {
    if (why)
    {
      if (e->flags & INPUT)
      {
	w->s_x += e->x + e->w / 2;
	w->s_y += e->y + e->h / 2;
	++w->s_n;
	for (World::keymap::iterator i = w->changes.begin(); i != w->changes.end(); ++i)
	{
	  if (!((e->flags ^ i->first) & nsEntity::Entity::PLAYER2))
	    e->HandleInput (i->first & ~nsEntity::Entity::PLAYER2, i->second);
	}
      }
    }
    else
    {
      if (e->nextthink)
      {
	e->nextthink -= 1000 / f;
	if (e->nextthink <= 0)
	{
	  e->nextthink = 0;
	  e->think ();
	}
      }
      if (!(e->flags & DISABLED))
      {
	int xadd = 0, yadd = 0;
	if (!(e->flags & NOGRAVITY))
	{
	  nsEntity::Entity *ent = 0;
	  if (w->target != "")
	    ent = w->FindTarget (w->target);
	  if (ent)
	  {
	    int xdist = ent->x + ent->w / 2 - e->x - e->w / 2;
	    int ydist = ent->y + ent->h / 2 - e->y - e->h / 2;
	    int distance = xdist * xdist + ydist * ydist;
	    if (distance >= 256)
	    {
	      xadd = (100 * ((w->ddx >> 8) * xdist - (w->ddy >> 8) * ydist) / distance) << 8;
	      yadd = (100 * ((w->ddx >> 8) * ydist + (w->ddy >> 8) * xdist) / distance) << 8;
	      // the problem were too big numbers...
	    }
	  }
	  else
	  {
	    xadd = w->ddx;
	    yadd = w->ddy;
	  }

	  e->ddx += xadd;
	  e->ddy += yadd;
	}

	e->AutoMove (f);

	e->ddx -= xadd;
	e->ddy -= yadd;
      }
    }
  }
  int X()
  {
    return w->s_n ? w->s_x / w->s_n : 0;
  }
  int Y()
  {
    return w->s_n ? w->s_y / w->s_n : 0;
  }
};

void Kill9 (nsEntity::Entity *ent)
{
  ent->world->kill_9 (ent);
}

int World::_world::tick (bitmap screen, int framerate, float factor)
{
  timer += 1000 / framerate;
  framerate = int (framerate * factor);
  itimer += 1000 / framerate;

  _tokill.erase (_tokill.begin(), _tokill.end());
  Ticker t(this, framerate, 0);   // movement only
  Collision::for_each (*_ent, t);
  Collision::PrepareEntityContainer (*_ent);
  t = Ticker(this, framerate, 1); // everything else
  Collision::for_each (*_ent, t);
  if (s_n)
    _map->scrollto (t.X() - screen->w / 2, t.Y() - screen->h / 2, 0, screen);
  Collision::for_each (_tokill, Kill9);
  SoundPos ();
  std::copy (_tospawn.begin(), _tospawn.end(), std::back_inserter (*_ent));
  std::copy (_tospawn.begin(), _tospawn.end(), std::back_inserter (*_zlist));
  std::sort (_zlist->begin(), _zlist->end(), ZOrder);
  _tospawn.erase (_tospawn.begin(), _tospawn.end());
  return multiplayer ? first_players * second_players : first_players + second_players;
}

void World::update_keys (volatile char keys[], keymap &oldkeys, keymap &changes)
  // Allegro's keys-Array in keys
  // oldkeys: pair <my_keyocde, bool>
{
  std::map <int, int> m;
  for (int *p = keycodes; *p; ++p)
    if (keys[*p++])
      ++m[*p];
  changes.clear ();
  for (keymap::iterator i = oldkeys.begin(); i != oldkeys.end(); ++i)
  {
    if (!i->second != !m[i->first])    // casting to bool
      changes[i->first] = m[i->first];
    i->second = m[i->first];
    m.erase (i->first);
  }
  {
    for (std::map<int, int>::iterator i = m.begin(); i != m.end(); ++i)
    {
      if (i->second)
	changes [i->first] = i->second; // casting to bool
      oldkeys [i->first] = i->second;
    }
  }
}

void World::_world::update_keys (const keymap &chgs)
{
  changes = chgs;
}

int World::_world::ParseAction (const std::string &action)
{
  if (action == "EXIT")
    return EXIT;
  if (action == "KILL")
    return KILL;
  if (action == "SWITCH")
    return SWITCH;
  if (action == "FIND")
    return FIND;
  // ...

  return 0;
}

nsEntity::Entity *ActionEnt;

void World::_world::Exit (nsEntity::Entity *ent)
{
  if (ent == ActionEnt)
    return;
  if (ent->flags & DISABLED)
    return;
  if (ent->flags & EXITTED)
    return;
  if (!(ent->flags & INPUT))
    return;
  ent->world->Register (ent, EXITTED, 1);
  ent->Homicide ();
  ent->world->next = ActionEnt->targetname;
  ent->world->Found = ent;
  // not needing new property
  // exits cannot be remote-controlled then, but
  // never mind
}

void World::_world::Kill (nsEntity::Entity *ent)
{
  if (ent == ActionEnt)
    return;
  if (ent->world->OnlyPlayers && (!(ent->flags & INPUT) || (ent->flags & DISABLED)))
    return;
  if (!(ent->flags & KILLABLE) || (ent->flags & DISABLED))
    return;
  ent->Homicide ();
  // has to setup some things and call world->kill
}

void World::_world::Find (nsEntity::Entity *ent)
{
  if (ent == ActionEnt)
    return;
  if (ent->world->OnlyPlayers == 2 && (!ent->teleportable || ent->flags & DISABLED))
    return;
  if (ent->world->OnlyPlayers == 1 && (!(ent->flags & INPUT) || (ent->flags & DISABLED)))
    return;
  ent->world->Found = ent;
}

void World::_world::Toggle (nsEntity::Entity *ent)
{
  if (ent == ActionEnt)
    return;
  if (ent->world->OnlyPlayers && (!(ent->flags & INPUT) || (ent->flags & DISABLED)))
    return;
  if (ent->flags & SWITCHABLE)
  {
    ent->HandleInput (nsEntity::Entity::TOGGLE, 1);
    ent->HandleInput (nsEntity::Entity::TOGGLE, 0);
  }
  else if ((ent->flags & KILLABLE) && !(ent->flags & DISABLED))
  {
    ent->Homicide ();
  }
}

void World::_world::Action (nsEntity::Entity *Actor, int Act, int x, int y, int w, int h, int Only_Players)
{
  Found = 0;
  ActionEnt = Actor;
  OnlyPlayers = Only_Players;
  switch (Act)
  {
   case EXIT:
    Collision::for_each_sorted (*_ent, Exit, x, y, w, h);
    break;
   case KILL:
    Collision::for_each_sorted (*_ent, Kill, x, y, w, h);
    break;
   case SWITCH:
    Collision::for_each_sorted (*_ent, Toggle, x, y, w, h);
    break;
   case FIND:
    Collision::for_each_sorted (*_ent, Find, x, y, w, h);
    break;
  }
}

namespace
{
  template <class E, class Func> struct ActionAdaptor
  {
    E *ent;
    Func f;
    ActionAdaptor (E *e, Func fnc) : ent(e), f(fnc)
    {
    }
    void operator () (E *e)
    {
      if (e == ent)
	return;
      f (ent, e);
    }
  };

  typedef ActionAdaptor <Ent, void (*) (Ent *, Ent *)> Adaptor;
}

void World::_world::Action (nsEntity::Entity *Actor, void (*what) (nsEntity::Entity *ActionEnt, nsEntity::Entity *FoundEnt), int x, int y, int w, int h)
{
  Collision::for_each_sorted (*_ent, Adaptor (Actor, what), x, y, w, h);
}


void World::_world::kill_9 (nsEntity::Entity *ent)
{
  if (ent->flags & INPUT)
  {
    if (ent->flags & PLAYER2)
      --second_players;
    else
      --first_players;
  }
  // find all sounds associated with ent
  ::Sound::Sound *my_sound = Positional_Sounds[ent];
  if (my_sound)
    delete my_sound;
  Positional_Sounds.erase (Positional_Sounds.find (ent));

  _ent->erase (std::find (_ent->begin(), _ent->end(), ent));
  _zlist->erase (std::find(_zlist->begin(), _zlist->end(), ent));
  delete ent;
  // like linux's "kill -9 %d"
}

void World::_world::kill (nsEntity::Entity *ent)
{
  if (std::find (_tokill.begin(), _tokill.end(), ent) == _tokill.end())
    _tokill.push_back (ent);
}


void World::_world::PlaySound (nsEntity::Entity *ent, std::string FN, int volume, bool positional, int loopmode)
{
  // positional audio?
  if (ent && positional)
  {
    if (Positional_Sounds[ent])
      delete Positional_Sounds[ent];
    Snd *snd = Positional_Sounds[ent] = new Snd (Filename (FN));
    snd->volume (volume);
    snd->position (ent->x - _map->scrxpos - SCRWIDTH / 2, ent->y - _map->scrypos - SCRHEIGHT / 2);
    snd->play (loopmode);
  }
  else
  {
    Snd *snd = new Snd (Filename (FN));
    if (snd->IsMIDI())
      StopMusic ();
    snd->volume (volume);
    snd->position (0, 0);
    snd->play (loopmode);
    if (snd->IsMIDI())
      CurrentMIDI = snd;
    else
      Independent_Sounds.push_back (snd);
  }
}

void World::_world::StopMusic ()
{
  if (CurrentMIDI)
  {
    delete CurrentMIDI;
    CurrentMIDI = 0;
  }
}

void World::_world::Silence ()
{
  {
    for (std::map < nsEntity::Entity *, Snd *>::iterator iter = Positional_Sounds.begin(); iter != Positional_Sounds.end(); ++iter)
      if (iter->second)
      {
	delete iter->second;
	iter->second = 0;
      }
  }
  for (std::list <Snd *>::iterator iter = Independent_Sounds.begin(); iter != Independent_Sounds.end(); ++iter)
    if (*iter)
    {
      delete *iter;
      *iter = 0;
    }
  StopMusic ();
  Positional_Sounds.erase (Positional_Sounds.begin(), Positional_Sounds.end());
  Independent_Sounds.erase (Independent_Sounds.begin(), Independent_Sounds.end());
}

void World::_world::SoundPos ()
{
  for (std::map < nsEntity::Entity *, Snd *>::iterator iter = Positional_Sounds.begin(); iter != Positional_Sounds.end(); ++iter)
    if (iter->second)
    {
      if (iter->first)
	iter->second->position (iter->first->x - x, iter->first->y - y);
      else
      {
	delete iter->second;
	iter->second = 0;
	// dead entries
	// will be killed on exit
	// because THIS should never happen
      }
    }
  while (Independent_Sounds.size() > SOUND_LIMIT)
  {
    Snd *snd = Independent_Sounds.front();
    Independent_Sounds.pop_front();
    delete snd;
  }
}

nsEntity::Entity *World::_world::FindTarget (std::string targetname)
{
  for (nsEntity::EntityContainer::iterator i = _ent->begin(); i != _ent->end(); ++i)
    if ((*i)->targetname == targetname)
      return *i;
  return 0;
}
/*
   template <class T> void swap (T &x, T &y)
   {
   T h = x;
   x = y;
   y = h;
   }
   */

void World::_world::Switch (int x, int y, int w, int h)
{
  int mx, my, mX, mY;
  ::Collision::get_map_range (x, y, w, h, SPRWIDTH, SPRHEIGHT, &mx, &my, &mX, &mY);
  for (int X = mx; X <= mX; ++X)
    for (int Y = my; Y <= mY; ++Y)
    {
      Level::Maptile m = (*_map)(X, Y);
      if (m.flags & ::SWITCHABLE)
      {
	std::swap (m.sprnum, m.sprswitchnum);
	std::swap (m.flags, m.swflags);
	_map->set (X, Y, m);
      }
    }
  Action (0, SWITCH, x, y, w, h);
}

namespace
{
  std::map <std::string, int> groups;
  int lastgroup = 0;
}

int World::_world::NamedGroup (std::string filename, int width, int height)
{
  if (groups[filename])
    return groups[filename];
  --lastgroup;
  int r, g, b;
  GetBorder (r, g, b);
  Border ();
  ::IO::ReadSpriteData (Filename (filename).c_str(), lastgroup, width, height);
  Border (r, g, b);
  groups[filename] = lastgroup;
  return lastgroup;
}

namespace
{
  template <class T> T Null (T &x)
  {
    T y = x;
    x = 0;
    return y;
  }
}

bitmap World::_world::get_hud (int n)
{
  if (n == 1)
    return Null(upper_hud);
  if (n == 2)
    return Null(lower_hud);
  return 0;
}

void World::_world::give_back_hud (int n, bitmap hud)
{
  if (n == 1)
    upper_hud = hud;
  if (n == 2)
    lower_hud = hud;
}

nsEntity::Entity *World::_world::spawn (nsEntity::ConfigMap &m)
{
  nsEntity::Entity *ent = nsEntity::make_entity (m, this);
  _tospawn.push_back (ent);
  return ent;
}

int World::_world::CountObjects ()
{
  return _ent->size ();
}
