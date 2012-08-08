#ifndef __RP__OBJECTS_H__
#define __RP__OBJECTS_H__

#include "defs.h"

namespace Objects
{
  void init ();
}

#ifdef __RP__OBJECTS_CPP__

#define BEGIN_OBJECTS_LIST(name) 	void name () {
#define END_OBJECTS_LIST                }
#define OBJECT(x)		        class x : public nsEntity::Entity
#define EXTEND(x,y)		        class x : public y

#endif

#endif
