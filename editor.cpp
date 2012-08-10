#include "alleg.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>

using namespace std;

#include "defs.h"
#include "io.h"
#include "map.h"
#include "entity.h"
#include "milli.h"
#include "dialogs.h"
#include "sound.h"

const int MAP_WIDTH = 256;
const int MAP_HEIGHT = 256;

::Sound::Sound *snd = 0;

void StopSound ()
{
  if (snd)
    delete snd;
  snd = 0;
}

void StartSound (string filename)
{
  StopSound ();
  (snd = new ::Sound::Sound (Filename (filename)))->play (0);
}

void MapCpy (Level::Map &m1, Level::Map &m2)
{
  for (int x = 0; x < m1.mapwidth; ++x)
    for (int y = 0; y < m1.mapheight; ++y)
      m1.set (x, y, m2(x, y), 0);
  m1.InvalidateCache ();
}

void MapCpy (Level::Map &m, int x1, int y1, int x2, int y2, int xt, int yt)
{
  if (x1 > x2)
  {
    swap (x1, x2);
    xt = xt - x2 + x1;
  }
  if (y1 > y2)
  {
    swap (y1, y2);
    yt = yt - y2 + y1;
  }
  bool copy_xdir, copy_ydir;
  copy_xdir = x1 < xt;
  copy_ydir = y1 < yt;
  for (int x = (copy_xdir ? x2 : x1); x != (copy_xdir ? x1 : x2) + (copy_xdir ? -1 : 1); x += (copy_xdir ? -1 : 1))
    for (int y = (copy_ydir ? y2 : y1); y != (copy_ydir ? y1 : y2) + (copy_ydir ? -1 : 1); y += (copy_ydir ? -1 : 1))
      m.set (x + xt - x1, y + yt - y1, m(x, y));
}

void MapDimensions (Level::Map &m, int &x, int &y)
{
  for (x = m.mapwidth-1; x != 0; --x)
  {
    // if nonempty tiles in col x
    bool found = 0;
    for (int t = 0; t < MAP_HEIGHT; ++t)
      if (m(x, t) != Level::Maptile ())
	found = 1;
    if (found)
      break;
  }
  for (y = m.mapheight-1; y != 0; --y)
  {
    // if nonempty tiles in line y
    bool found = 0;
    for (int t = 0; t < x; ++t)
      if (m(t, y) != Level::Maptile ())
	found = 1;
    if (found)
      break;
  }
  ++x;
  ++y;
}

void edit_properties (nsEntity::ConfigMap &m)
{
  // first convert m to a string
  // let the user edit it in a box
  // and convert it back

  static char **buf; // 64k should be large enough
  typedef char *pchar;

  buf = new pchar[256];

  m[" fertig"] = "#";
  m[" neu"] = "+";

  char **current = buf;
  for (nsEntity::ConfigMap::iterator iter = m.begin(); iter != m.end(); ++iter, ++current)
  {
    *current = new char[256];
    sprintf (*current, "%s=%s", iter->first.c_str(), iter->second.c_str());
  }

  *current = 0;

  for (;;)
  {
    int i = Dialogs::ListBox ("Eigenschaft", buf, current - buf);
    char *selected = buf[i];
    if (*selected == ' ')
    {
      if (*(selected+strlen(selected)-1) == '+')
      {
	*current = new char[256];
	strcpy (*current, "unnamed=0");
	++current;
	*current = 0;
      }
      else if (*(selected+strlen(selected)-1) == '#')
	break;
    }
    else
      // edit property
    {
      strcpy (selected, Dialogs::ReadLn ("Eigenschaft bearbeiten", selected).c_str());
      while (*selected == ' ')
	memmove (selected, selected+1, strlen(selected)-1);
    }
  }

  m.clear ();

  for (current = buf; *current; ++current)
  {
    if (**current != ' ')
    {
      if (char *c = strchr (*current, '='))
      {
	*c = 0;
	m[*current] = c+1;
      }
    }
    delete [] *current;
  }

  delete [] buf;
}


bitmap buffer;

void select_sprite (int &groupnum, int &sprnum)
{
  bool quit = 0;
  while (!quit)
  {
    clear (buffer);
    // start at 0 to screenmax
    int x, sprnum2;
    for ((sprnum2 = sprnum), (x = 0); x < buffer->w; (sprnum2 = (sprnum2 + 1) % Sprite::Sprites[groupnum].size()), (x += SPRWIDTH))
    {
      Sprite::Sprites[groupnum][sprnum2].draw (buffer, x, 0);
    }
    update_screen (buffer);
    switch (readkey() >> 8)
    {
     case KEY_RIGHT:
      sprnum = (sprnum + 1) % (Sprite::Sprites[groupnum].size());
      break;
     case KEY_LEFT:
      sprnum = (sprnum + Sprite::Sprites[groupnum].size() - 1) % (Sprite::Sprites[groupnum].size());
      break;
     case KEY_ENTER:
     case KEY_SPACE:
      quit = 1;
      break;
     case KEY_DOWN:
      ++groupnum;
      if (Sprite::Sprites[groupnum].size() == 0)
	groupnum = 0;
      sprnum = 0;
      break;
     case KEY_UP:
      if (groupnum)
	--groupnum;
      sprnum = 0;
      break;
    }
  }
}

void draw_entities (bitmap screen, IO::IOEntityContainer &cont, int scrx, int scry, nsEntity::IOEntity *selected)
{
  IO::IOEntityContainer list;
  Collision::FindCollidingEntities (scrx, scry, screen->w, screen->h, cont, list);
  for (unsigned int i = 0; i < list.size(); ++i)
  {
    Sprite::Sprites[list[i]->groupnum][list[i]->sprnum].draw (screen, list[i]->x-scrx, list[i]->y-scry);
    long col = list[i] == selected ? makecol (255, 255, 255) : makecol (128, 128, 128);
    rect (screen, list[i]->x-scrx, list[i]->y-scry, list[i]->x-scrx+list[i]->w-1, list[i]->y-scry+list[i]->h-1, col);
    std::string info;
    if(list[i]->properties.find("targetname") != list[i]->properties.end())
      info = list[i]->properties["targetname"];
    if(list[i]->properties.find("target") != list[i]->properties.end())
      info = info + "->" + list[i]->properties["target"];
    if(info != "")
    {
      int x = list[i]->x+list[i]->w/2-scrx;
      int y = list[i]->y+list[i]->h-scry;
      int col0 = makecol(0,0,0);
      textout_centre_border_ex(screen, font, info.c_str(), x, y, col, col0);
    }
  }
}

nsEntity::IOEntity *add_entity (IO::IOEntityContainer &cont, int x, int y, int w, int h, int groupnum, int sprnum)
{
  nsEntity::IOEntity *e = new nsEntity::IOEntity;
  e->x = x;
  e->y = y;
  e->w = w;
  e->h = h;
  e->sprnum = sprnum;
  e->groupnum = groupnum;
  cont.push_back (e);
  return e;
}

void redraw(bitmap buffer, Level::Map &m, IO::IOEntityContainer &ents, nsEntity::IOEntity *selected)
{
  clear (buffer);
  // m.InvalidateCache ();
  m.draw (buffer);
  draw_entities (buffer, ents, m.scrxpos, m.scrypos, selected);
  rect (buffer,
      m.selxpos * SPRWIDTH + 1 - m.scrxpos, m.selypos * SPRHEIGHT + 1 - m.scrypos,
      m.selxpos * SPRWIDTH + SPRWIDTH - 2 - m.scrxpos, m.selypos * SPRHEIGHT + SPRHEIGHT - 2 - m.scrypos,
      makecol (128, 0, 255));
  rect (buffer,
      m.blockxpos * SPRWIDTH + 1 - m.scrxpos, m.blockypos * SPRHEIGHT + 1 - m.scrypos,
      m.blockxpos * SPRWIDTH + SPRWIDTH - 2 - m.scrxpos, m.blockypos * SPRHEIGHT + SPRHEIGHT - 2 - m.scrypos,
      makecol (255, 128, 0));

  update_screen (buffer);
}

void load_additional_sprites (nsEntity::IOEntity &e)
{
  Sprite::Sprites.erase(Sprite::Sprites.begin(), Sprite::Sprites.end());
  IO::ReadSpriteData (Filename ("BASE.PCX").c_str (), 0);
  int nSets = str2int(e.properties["SPRITESETS"]);
  for (int i = 1; i <= nSets; ++i)
    ::IO::ReadSpriteData (Filename (e.properties[string("SPRITESET") + int2str(i)]).c_str(), i);
}

int main(int argc, char **argv)
{
  if(argc>2) SCRWIDTH=str2int(argv[2]);
  if(argc>3) SCRHEIGHT=str2int(argv[3]);

  allegro_init ();
  install_keyboard ();
  if (!graphics_init (1, 0))
    return 0;
  install_mouse ();
  sound_init ();
  Sound::init ();
  Timer::init_timer ();

  string s = (argc>1) ? std::string(argv[1]) : Dialogs::ReadLn("Name des zu ladenden Levels", "new");

  try
  {
    IO::ReadSpriteData ("base.pcx", 0);
  }
  catch (const char *s)
  {
    allegro_exit ();
    cerr << "exception: " << s << endl;
    return 0;
  }

  {
    Level::Map m(MAP_WIDTH, MAP_HEIGHT);
    IO::IOEntityContainer ents;
    nsEntity::IOEntity worldent;
    worldent.properties["type"] = "_world";
    worldent.properties["dx"] = "0";
    worldent.properties["dy"] = "0";
    worldent.properties["ddx"] = "0";
    worldent.properties["ddy"] = "9.81";
    worldent.properties["map"] = "LEVEL.MAP";

    if (s != string("new"))
    {
      // read entities
      ents = IO::ReadEntityData ((s + ".ent").c_str());
      nsEntity::IOEntity *e = *(ents.end()-1);
      if (e->properties["type"] != "_world")
	throw "world entity not found";
      worldent = *(*(ents.end()-1));
      delete *(ents.end()-1);
      ents.pop_back ();
      load_additional_sprites (worldent);
      Level::Map m2 = IO::ReadLevelMapData (Filename (worldent.properties["map"]).c_str());
      MapCpy (m, m2);
    }

    m.cheat = 1;
    m.selxpos = 0;
    m.selypos = 0;
    m.blockxpos = 0;
    m.blockypos = 0;
    buffer = create_bitmap (SCRWIDTH, SCRHEIGHT);

    m.scrollto (0, 0, 0, buffer);
    m.InvalidateCache ();

    int copy_x = 0;
    int copy_y = 0;

    bool quit = 0;
    Level::Maptile current(0, 0, 0, 0, 0), background(0, 0, 0, 0, 0);

    nsEntity::IOEntity *selected_ent = 0;

    while (!quit)
    {
      m.fix_selpos();
      background = m(m.blockxpos, m.blockypos);
      Level::Maptile temp (m(m.selxpos, m.selypos));
      m.set (m.selxpos, m.selypos, current);
      redraw (buffer, m, ents, selected_ent);
      m.set (m.selxpos, m.selypos, temp);
      switch (readkey () >> 8)
      {
       case KEY_UP:
	--m.selypos;
	break;
       case KEY_DOWN:
	++m.selypos;
	break;
       case KEY_LEFT:
	--m.selxpos;
	break;
       case KEY_RIGHT:
	++m.selxpos;
	break;
       case KEY_ESC:
	quit = 1;
       case KEY_PGUP:
	m.scrollby (0, -SPRHEIGHT, 0, buffer);
	--m.selypos;
	break;
       case KEY_PGDN:
	m.scrollby (0, SPRHEIGHT, 0, buffer);
	++m.selypos;
	break;
       case KEY_HOME:
	m.scrollby (-SPRWIDTH, 0, 0, buffer);
	--m.selxpos;
	break;
       case KEY_END:
	m.scrollby (SPRWIDTH, 0, 0, buffer);
	++m.selxpos;
	break;
       case KEY_SPACE:
       case KEY_INSERT:
	m.set(m.selxpos, m.selypos, current);
	break;
       case KEY_DEL:
	m.set(m.selxpos, m.selypos, background);
	break;
       case KEY_TAB:
	select_sprite(current.groupnum, current.sprnum);
	break;
       case KEY_G:
	current = m(m.selxpos, m.selypos);
	break;
       case KEY_S:
	m.blockxpos = m.selxpos;
	m.blockypos = m.selypos;
	break;
       case KEY_ENTER:
	for (int x = m.blockxpos; x <= m.selxpos; ++x)
	{
	  for (int y = m.blockypos; y <= m.selypos; ++y)
	    m.set(x, y, current);
	  for (int y = m.blockypos; y >= m.selypos; --y)
	    m.set(x, y, current);
	}
	for (int x = m.blockxpos; x >= m.selxpos; --x)
	{
	  for (int y = m.blockypos; y <= m.selypos; ++y)
	    m.set(x, y, current);
	  for (int y = m.blockypos; y >= m.selypos; --y)
	    m.set(x, y, current);
	}
	break;
       case KEY_F5:
	{
	  Level::Maptile f5_cur = current;
	  for (int y = m.blockypos; y <= m.selypos; ++y)
	  {
	    for (int x = m.blockxpos; x <= m.selxpos; ++x)
	    {
	      m.set(x, y, current);
	      current.sprnum = (current.sprnum + 1) % (Sprite::Sprites[current.groupnum].size());
	    } 
	    for (int x = m.blockxpos; x >= m.selxpos; --x)
	    {
	      m.set(x, y, current);
	      current.sprnum = (current.sprnum + 1) % (Sprite::Sprites[current.groupnum].size());
	    } 
	  }
	  for (int y = m.blockypos; y >= m.selypos; --y)
	  {
	    for (int x = m.blockxpos; x <= m.selxpos; ++x)
	    {
	      current.sprnum = (current.sprnum + 1) % (Sprite::Sprites[current.groupnum].size());
	      m.set(x, y, current);
	    } 
	    for (int x = m.blockxpos; x >= m.selxpos; --x)
	    {
	      current.sprnum = (current.sprnum + 1) % (Sprite::Sprites[current.groupnum].size());
	      m.set(x, y, current);
	    } 
	  }
	  current = f5_cur;	
	  break;
	}
       case KEY_BACKSPACE:
	for (int x = m.blockxpos; x <= m.selxpos; ++x)
	{
	  for (int y = m.blockypos; y <= m.selypos; ++y)
	    m.set(x, y, background);
	  for (int y = m.blockypos; y >= m.selypos; --y)
	    m.set(x, y, background);
	}
	for (int x = m.blockxpos; x >= m.selxpos; --x)
	{
	  for (int y = m.blockypos; y <= m.selypos; ++y)
	    m.set(x, y, background);
	  for (int y = m.blockypos; y >= m.selypos; --y)
	    m.set(x, y, background);
	}
	break;
       case KEY_1:
	current.flags ^= COLLIDES;
	break;
       case KEY_2:
	current.flags ^= DEADLY;
	break;
       case KEY_3:
	current.flags ^= SWITCHABLE;
	break;
       case KEY_0:
	current.flags = 0;
	break;
       case KEY_T:
	swap (current.sprnum, current.sprswitchnum);
	swap (current.flags, current.swflags);
	break;
       case KEY_N:
	selected_ent = add_entity (ents, m.selxpos * SPRWIDTH, m.selypos * SPRHEIGHT, SPRWIDTH, SPRHEIGHT, current.groupnum, current.sprnum);
	break;
       case KEY_E:
	{
	  IO::IOEntityContainer cont;
	  Collision::FindCollidingEntities (m.selxpos * SPRWIDTH, m.selypos * SPRHEIGHT, SPRWIDTH, SPRHEIGHT, ents, cont);
	  if (!cont.empty())
	  {
	    bool quit = 0;
	    IO::IOEntityContainer::iterator iter = cont.begin();
	    while (!quit)
	    {
	      redraw (buffer, m, ents, selected_ent = *iter);
	      switch (readkey() >> 8)
	      {
	       case KEY_LEFT:
		if (iter == cont.begin())
		  iter = cont.end();
		--iter;
		break;
	       case KEY_RIGHT:
		++iter;
		if (iter == cont.end())
		  iter = cont.begin();
		break;
	       case KEY_ENTER:
		quit = 1;
	      }
	    }
	  }
	}
	break;
       case KEY_R:
	// resize entity
	if (selected_ent)
	{
	  bool quit = 0;
	  while (!quit)
	  {
	    redraw (buffer, m, ents, selected_ent);
	    switch (readkey() >> 8)
	    {
	     case KEY_LEFT:
	      if (selected_ent->w > 1)
		--(selected_ent->w);
	      break;
	     case KEY_RIGHT:
	      ++selected_ent->w;
	      break;
	     case KEY_UP:
	      if (selected_ent->h > 1)
		--selected_ent->h;
	      break;
	     case KEY_DOWN:
	      ++selected_ent->h;
	      break;
	     case KEY_HOME:
	      if (selected_ent->w > 8)
		selected_ent->w -= 8;
	      break;
	     case KEY_END:
	      selected_ent->w += 8;
	      break;
	     case KEY_PGUP:
	      if (selected_ent->h > 8)
		selected_ent->h -= 8;
	      break;
	     case KEY_PGDN:
	      selected_ent->h += 8;
	      break;
	     case KEY_ENTER:
	      quit = 1;
	    }
	  }
	}
	break;
       case KEY_M:
	// move entity
	if (selected_ent)
	{
	  bool quit = 0;
	  while (!quit)
	  {
	    redraw (buffer, m, ents, selected_ent);
	    switch (readkey() >> 8)
	    {
	     case KEY_LEFT:
	      if (selected_ent->x > 0)
		--(selected_ent->x);
	      break;
	     case KEY_RIGHT:
	      if (selected_ent->x < m.mapwidth * SPRWIDTH - 1)
		++selected_ent->x;
	      break;
	     case KEY_UP:
	      if (selected_ent->y > 0)
		--selected_ent->y;
	      break;
	     case KEY_DOWN:
	      if (selected_ent->y < m.mapheight * SPRHEIGHT - 1)
		++selected_ent->y;
	      break;
	     case KEY_HOME:
	      if (selected_ent->x > 7)
		selected_ent->x -= 8;
	      break;
	     case KEY_END:
	      if (selected_ent->x < m.mapwidth * SPRWIDTH - 8)
		selected_ent->x += 8;
	      break;
	     case KEY_PGUP:
	      if (selected_ent->y > 7)
		selected_ent->y -= 8;
	      break;
	     case KEY_PGDN:
	      if (selected_ent->y < m.mapheight * SPRHEIGHT - 8)
		selected_ent->y += 8;
	      break;
	     case KEY_ENTER:
	      quit = 1;
	    }
	  }
	}
	break;
       case KEY_O:
	if (selected_ent)
	{
	  selected_ent->sprnum = current.sprnum;
	  selected_ent->groupnum = current.groupnum;
	}
	break;
       case KEY_X:
	// entity removal
	if (selected_ent)
	  ents.erase (find(ents.begin(), ents.end(), selected_ent));
	selected_ent = 0;
	break;
       case KEY_D:
	// entity duplication
	if (selected_ent)
	{
	  ents.push_back (new nsEntity::IOEntity (*selected_ent));
	  selected_ent->x = m.selxpos * SPRWIDTH;
	  selected_ent->y = m.selypos * SPRHEIGHT;
	}
	break;
       case KEY_W:
	// world properties
	edit_properties (worldent.properties);
	load_additional_sprites (worldent);
	break;
       case KEY_P:
	if (selected_ent)
	  edit_properties (selected_ent->properties);
	break;
       case KEY_C:
	// set copy rect
	copy_x = m.selxpos;
	copy_y = m.selypos;
	break;
       case KEY_V:
	// copy
	MapCpy (m, copy_x, copy_y, m.blockxpos, m.blockypos, m.selxpos, m.selypos);
	break;
       case KEY_F1:
	if (selected_ent && selected_ent->properties["type"] == "SoundPlayer")
	{
	  string fn = selected_ent->properties["sound"];
	  StartSound (fn);
	}
	else
	  StopSound ();
	break;
       case KEY_F2:
	StopSound ();
	break;
      }
    }

    if (Dialogs::MessageBox ("Speichern?", Dialogs::YES, Dialogs::NO, 0) == Dialogs::YES)
    {
      string s = Dialogs::ReadLn ("Levelname (ohne .ENT und .MAP!)", "");
      worldent.properties["map"] = s + ".map";
      worldent.properties["type"] = "_world";
      ents.push_back (new nsEntity::IOEntity (worldent));
      IO::WriteEntityData ((s + ".ent").c_str(), ents);

      // get m's width and height

      int x, y;

      MapDimensions (m, x, y);

      Level::Map m2(x, y);
      MapCpy (m2, m);

      IO::WriteLevelMapData (worldent.properties["map"].c_str(), m2);
    }
    destroy_ptr_container (ents);
  }

  ::Sprite::Sprites.erase (::Sprite::Sprites.begin(), ::Sprite::Sprites.end());

  Timer::done_timer ();

  StopSound ();
  Sound::done ();

  allegro_exit ();

  return 0;
}

END_OF_MAIN ();
