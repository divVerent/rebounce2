#include "keys.h"

#include "alleg.h"
#include "entity.h"

namespace
{
  typedef nsEntity::Entity K;
}

int keycodes[] =
{
  KEY_MAX,       K::CHEAT,
  // this key does not exist
  KEY_UP,        K::UP,
  KEY_DOWN,      K::DOWN,
  KEY_LEFT,      K::LEFT,
  KEY_RIGHT,     K::RIGHT,
  KEY_SPACE,     K::TOGGLE,
  KEY_ENTER,     K::TOGGLE,
  KEY_RCONTROL,  K::BUTTONA,
  KEY_LCONTROL,  K::BUTTONA,
  KEY_ESC,       K::SUICIDE,
  KEY_T,         K::UP | K::PLAYER2,
  KEY_G,         K::DOWN | K::PLAYER2,
  KEY_F,         K::LEFT | K::PLAYER2,
  KEY_H,         K::RIGHT | K::PLAYER2,
  KEY_TAB,       K::TOGGLE | K::PLAYER2,
  KEY_Q,         K::BUTTONA | K::PLAYER2,
  0,             0
};
