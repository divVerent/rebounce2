#ifndef __RP__WORLD_H__
#define __RP__WORLD_H__

#include "defs.h"

#include <string>
#include <map>

#include "entity.h"
#include "coll.h"
#include "io.h"

namespace World
{
  typedef std::map<int, bool> keymap;
  void update_keys (volatile char [], keymap &, keymap &);

  class _world : public nsEntity::Entity
  {
   public:
    _world (nsEntity::ConfigMap &, nsEntity::Entity * = 0);


   private:
    int OnlyPlayers;
    class Ticker;
    std::string target;
    bitmap upper_hud;
    bitmap lower_hud;
    nsEntity::EntityContainer *_ent;
    nsEntity::EntityContainer *_zlist;
    nsEntity::EntityContainer _tokill;
    nsEntity::EntityContainer _tospawn;
    Level::Map *_map;
    std::string mapname;
    keymap changes;
    friend class ::World::_world::Ticker;
    static void Kill (nsEntity::Entity *); 
    static void Exit (nsEntity::Entity *);
    static void Find (nsEntity::Entity *);
    static void Toggle (nsEntity::Entity *);
    int s_x, s_y, s_n;
    void SoundPos ();
    void Silence ();

   public:

    int timer, itimer, timelimit;         // timelimit: internal time
    int iendtimer;
    int time_factor;

    int first_players, second_players;
    bool multiplayer;
    // now the services for "friend class Entity" and all derived classes
    // therefore "public"

    inline int RectFlags (int x, int y, int w, int h)
    {
      return ::Collision::RectFlags (*_map, x, y, w, h);
    }

    // flags
    enum {
      INPUT = 1,
      KILLABLE = 2,
      EXITTED = 4,
      SWITCHABLE = 16,
      NOGRAVITY = 32,
      DISABLED = 64,
      PLAYER2 = nsEntity::Entity::PLAYER2
    };

    inline void Register (nsEntity::Entity *e, int what, bool onoff)
    {
      int oldflags = e->flags;
      e->flags = onoff ? (e->flags | what) : (e->flags & ~what);
      if (e->flags == oldflags)
	return;
      if (what == INPUT)
	first_players += onoff ? 1 : -1;
      if (what == PLAYER2)
      {
	first_players -= onoff ? 1 : -1;
	second_players += onoff ? 1 : -1;
      }
    }

    // actions
    enum {
      EXIT = 1,
      KILL = 2,
      SWITCH = 3,
      FIND = 4
    };

    int ParseAction (const  std::string &action);
    void Action (nsEntity::Entity *Actor, int Act, int x, int y, int w, int h, int Only_Players = 0);
    void Action (nsEntity::Entity *Actor, void (*what) (nsEntity::Entity *ActionEnt, nsEntity::Entity *FoundEnt), int x, int y, int w, int h);

    void kill_9 (nsEntity::Entity *ent);
    void kill (nsEntity::Entity *ent);

    ~_world()
    {
      Silence ();
      if (_map)
	delete _map;
      if (_zlist)
	delete _zlist;
      if (_ent)
      {
	destroy_ptr_container (*_ent);
	delete _ent;
      }
      destroy_ptr_container (_tospawn);
    }

    void PlaySound (nsEntity::Entity *ent, std::string FN, int volume = 128, bool positional = 0, int loopmode = 0);
    void StopMusic ();
    void initworld (IO::IOEntityContainer &, bitmap upper = 0, bitmap lower = 0, bool CheatActivated = 0, bool MarkCheater = 0);
    void drawall (bitmap);
    int tick (bitmap, int, float = 1.0);
    void update_keys (const keymap &changes);
    nsEntity::Entity *FindTarget (std::string targetname);
    std::string next;
    nsEntity::Entity *Found;
    void Switch (int x, int y, int w, int h);
    int NamedGroup (std::string filename, int width = SPRWIDTH, int height = SPRHEIGHT);

    bitmap get_hud (int n);
    void give_back_hud (int n, bitmap);

    nsEntity::Entity *spawn (nsEntity::ConfigMap &m);

    int CountObjects ();
  };

  inline void init ()
  {
    ADD_ENTITY_TYPE (_world);
  }
}

#endif
