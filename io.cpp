#include "alleg.h"
#include "io.h"
#include "sprite.h"

void IO::ReadSpriteData (const char *filename, int groupnum, int w, int h)
{
  PALETTE pal;
  bitmap bmp2 = load_bitmap (filename, pal);
  bitmap bmp = Sprite::duplicate_bitmap (bmp2);
  destroy_bitmap (bmp2);
  // now tile the pcx...
  // in (SPRWIDTH+1)*(SPRHEIGHT+1) tiles
  int width = (bmp->w+1) / (w+1);
  int height = (bmp->h+1) / (h+1);
  Sprite::Sprites [groupnum].resize (width * height);
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      Sprite::Sprites [groupnum] [x + width * y] = Sprite::Sprite (bmp, x * (w+1), y * (h+1), w, h);
  free_optimized_bitmap (bmp);
}

Level::Map IO::ReadLevelMapData (const char *filename)
{
  PACKFILE *f = pack_fopen (filename, "r");
  uint64_t sz = file_size_ex(filename);
  char *data = new char[sz];
  pack_fread (data, sz, f);
  pack_fclose (f);
  return Level::Map (data, sz);
}

bool IO::WriteLevelMapData (const char *filename, Level::Map &lvl)
{
  PACKFILE *f = pack_fopen (filename, "w");
  char *data = lvl.OutputMap ();
  pack_fwrite (const_cast <char *> (data), lvl.OutputMap_Size(), f);
  pack_fclose (f);
  delete [] data;
  return 1;
}

void WRITE_ENTRY (const std::string &name, const std::string &value, PACKFILE *f)
{
  pack_fwrite (const_cast <char *> (name.c_str()), name.length() + 1, f);
  pack_fwrite (const_cast <char *> (value.c_str()), value.length() + 1, f);
}

std::string READ_LINE (PACKFILE *f)
{
  std::string s;
  char c;
  s = "";
  while (!pack_feof (f) && (c = pack_getc (f)))
    s += c;
  return s;
}

void READ_ENTRY (std::string &name, std::string &value, PACKFILE *f)
{
  name = READ_LINE (f);
  value = READ_LINE (f);
}

#define SIGNATURE "ReBounce"

IO::IOEntityContainer IO::ReadEntityData (const char *filename)
{
  PACKFILE *f;
  IO::IOEntityContainer ents;
  try
  {
    nsEntity::IOEntity current;
    current.properties.clear();
    f = pack_fopen (filename, "r");
    std::string name, value;
    READ_ENTRY (name, value, f);
    if (name != SIGNATURE)
      throw "unknown map format";
    while (!pack_feof(f))
    {
      READ_ENTRY (name, value, f);
      if (name == "")
      {
	if (value == "end")
	{
	  nsEntity::uncomplete_entity (current);
	  ents.push_back (new nsEntity::IOEntity(current));
	  current.properties.clear();
	}
      }
      else
	current.properties[name] = value;
    }
  }
  catch (...)
  {
    pack_fclose (f);
    throw;
  }
  pack_fclose (f);
  return ents;
}
bool IO::WriteEntityData (const char *filename, IO::IOEntityContainer &ents)
{
  PACKFILE *f = pack_fopen (filename, "w");
  WRITE_ENTRY (SIGNATURE, "(c) 2000 Rudolf Polzer", f);
  for (IO::IOEntityContainer::iterator i = ents.begin(); i != ents.end(); ++i)
  {
    nsEntity::complete_entity (**i);
    for (nsEntity::ConfigMap::iterator k = (*i)->properties.begin(); k != (*i)->properties.end(); ++k)
    {
      WRITE_ENTRY (k->first, k->second, f);
    }
    nsEntity::uncomplete_entity (**i);
    WRITE_ENTRY ("", "end", f);
  }
  pack_fclose (f);
  return 1;
}

