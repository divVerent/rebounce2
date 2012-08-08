#define __RP__OBJECTS_CPP__

#include "objects.h"

#include "entity.h"
#include "world.h"

#include "player.h"
#include "enemies.h"
#include "sensors.h"
#include "gates.h"

namespace Objects
{
  OBJECT (SoundPlayer)
  {
    bool off;
    int loopmode, n;
    std::string sound;
   public:
    SoundPlayer (nsEntity::ConfigMap &m, nsEntity::Entity *w): Entity (m, w)
    {
      world->Register (this, World::_world::NOGRAVITY, 1);
      initialize_int (off);
      initialize_int (loopmode);
      initialize_string (sound);
      initialize_int (n);
      if (n)
	sound += int2str(Random(n) + 1) + ".MID";
      // randomizer!
      doit ();
    }
    bool Homicide ()
    {
      off = 1;
      doit ();
      return 1;
    }
    ~SoundPlayer ()
    {
      off = 1;
      doit ();
    }
    void doit()
    {
      if (off)
	world->StopMusic ();
      else
	world->PlaySound (0, sound, 255, 0, loopmode);
    }
  };

  BEGIN_OBJECTS_LIST (init)

    ADD_ENTITY_TYPE (SoundPlayer);
  /*
     int off = 0;
     int loopmode = 0;
     string sound;
     int n = 0;
     */

  AddEnemies ();
  AddPlayer ();
  AddSensors ();
  AddGates ();

  END_OBJECTS_LIST
}

