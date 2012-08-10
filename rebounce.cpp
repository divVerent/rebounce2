#include "defs.h"

#include "milli.h"
#include "entity.h"
#include "io.h"
#include "world.h"
#include "border.h"

bool play (bitmap buf, std::string &LvlName, int &time_left, int &time_factor, float game_speed, bool CheatActivated = 0, bool MarkCheater = 0, bool AutoScreenshot = 0)
{
  clear (buf);

  IO::IOEntityContainer cont = IO::ReadEntityData (Filename((LvlName + ".ent")).c_str());

  nsEntity::IOEntity worldent = **(cont.end()-1);
  delete *(cont.end()-1);
  cont.pop_back ();

  nsEntity::Entity *e = nsEntity::make_entity (worldent);
  World::_world *world;
  if (!(world = dynamic_cast <World::_world *> (e)))
    throw "Invalid Level!";

  bitmap screen = create_sub_bitmap (buf, 0, HUD, buf->w, buf->h - 2 * HUD);
  bitmap upper = create_sub_bitmap (buf, 0, 0, buf->w, HUD);
  bitmap lower = create_sub_bitmap (buf, 0, buf->h - HUD, buf->w, HUD);

  world->initworld (cont, lower, upper, CheatActivated, MarkCheater);
  destroy_ptr_container (cont);

  bool mp = !!world->second_players;

  world->tick(screen, FRAMERATE, game_speed);
  world->drawall(screen);

  if(AutoScreenshot)
  {
    PALETTE p;
    get_palette(p);
    save_bitmap((LvlName + ".bmp").c_str(), screen, p);
  }
  else
  {
    fade_screen (buf);

    World::keymap keys_old, keys_changes;

    Timer::synchronize (FRAMERATE, 0, 1);

    int deathtimer = 0;

    int framecounter = 0;
    int frametime = 0;
    int fps = 0;
    bool profile = 0;
    bool pausekey = 0;
    bool pause = 0;
    int pause_r = 0, pause_g = 0, pause_b = 0;

    INIT_PROFILE ();
    BEGIN_PROFILE ("tick");
    for(;;)
    {
      if(!pause)
      {
        if(!world->tick(screen, FRAMERATE, game_speed))
          if((deathtimer ? deathtimer : (deathtimer = world->timer)) < world->timer - 1000)
            break;
      }
      END_PROFILE ();

      BEGIN_PROFILE ("input");
      poll_keyboard ();

      if(key[KEY_P] && !world->multiplayer)
      {
        if(!pausekey)
        {
          pause = !pause;
          if(pause)
          {
            GetBorder(pause_r, pause_g, pause_b);
            Border(255, 255, 0);
          }
          else
            Border(pause_r, pause_g, pause_b);
        }
        pausekey = 1;
      }
      else
        pausekey = 0;

      if(!pause)
      {
        World::update_keys (key, keys_old, keys_changes);
        world->update_keys (keys_changes);
      }
      END_PROFILE ();

      BEGIN_PROFILE ("idle");
      bool ok = Timer::synchronize (FRAMERATE);
      END_PROFILE ();

      if (ok)
      {
        ++framecounter;
        BEGIN_PROFILE ("world -> buf");
        world->drawall(screen);
        END_PROFILE ();

        if(pause)
          textout_centre_border_ex(buf, font, "PAUSE", SCRWIDTH / 2, SCRHEIGHT / 2 - 4, makecol(255, 255, 0), makecol(0, 0, 0));

        ADD_PROFILE_DATA ("*fps", fps);
        ADD_PROFILE_DATA ("no. of entities", world->CountObjects());
        if (key[KEY_F5])
          profile = 1;
        if (key[KEY_F6])
          profile = 0;
        if (profile)
          VIEW_PROFILE (buf);
        BEGIN_PROFILE ("buf -> screen");
        update_screen (buf);
        END_PROFILE ();
      }

      if (Timer::milliseconds >= frametime + 1000)
      {
        fps = framecounter;
        framecounter = 0;
        RESET_PROFILE ();
        frametime = Timer::milliseconds;
      }

      BEGIN_PROFILE ("tick");
    }

    (void) fps; // cppcheck

    END_PROFILE ();
    DONE_PROFILE ();
  }

  destroy_bitmap (screen);
  destroy_bitmap (upper);
  destroy_bitmap (lower);

  time_left = world->timelimit - world->iendtimer;
  time_factor = world->time_factor;

  if (time_left < 0)
    time_left = 0;

  if (mp && world->first_players)
  {
    delete world;
    LvlName = "1";
    return 0;
  }
  if (mp && world->second_players)
  {
    delete world;
    LvlName = "2";
    return 0;
  }
  if (mp)
  {
    delete world;
    LvlName = "0";
    return 0;
  }

  if (world->next != "")
  {
    LvlName = world->next;
    delete world;
    return 1;
  }
  else
  {
    delete world;
    return 0;
  }

}


