#ifndef __RP__IO_H__
#define __RP__IO_H__

#include "defs.h"

#include "map.h"
#include "entity.h"

namespace IO
{
  const char SIGNATURE[] = "RebounceEntities1";
  void ReadSpriteData (const char *filename, int groupnum, int w = SPRWIDTH, int h = SPRHEIGHT);
  Level::Map ReadLevelMapData (const char *filename);
  bool WriteLevelMapData (const char *filename, Level::Map &lvl);
  typedef std::vector < nsEntity::IOEntity *> IOEntityContainer;
  IOEntityContainer ReadEntityData (const char *filename);
  bool WriteEntityData (const char *filename, IOEntityContainer &ents);
}

#endif
