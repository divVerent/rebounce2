#include "defs.h"

#include <map>
#include "sound.h"
#include <math.h>
#include "alleg.h"
#include <ctype.h>
#include <algorithm>

typedef ::Sound::Sound *snd;

namespace
{
  std::map <std::string, SAMPLE *> Sounds;
  void sndPlaySound (const std::string &fn, int vol, int bal)
  {
    if(!EnableSound)
      return;
    if (SAMPLE *s = Sounds[fn])
      play_sample (s, vol, bal, 1000, 0);
  }
}

void Sound::sndLoadSound (const std::string &SoundFilename)
{
  if(!EnableSound)
    return;
  if (!Sounds[SoundFilename])
    Sounds[SoundFilename] = load_wav (SoundFilename.c_str());
}

Sound::Sound::Sound() : midi (0)
{
}

void Sound::Sound::SetSpeaker ()
{
  return;
  // nothing to do here at the moment
}

namespace
{
  std::string LoCase (std::string s)
  {
    std::transform (s.begin (), s.end (), s.begin (), tolower);
    return s;
  }
}

Sound::Sound::Sound (std::string Filename) : fn(Filename), Volume (127), X (0), Y (0), midi(0)
{
  if(EnableSound)
  {
    if (LoCase (Filename.substr (Filename.length () - 4)) == ".mid")
      midi = load_midi (Filename.c_str());
    else
      sndLoadSound (fn);
  }
  Calc();
}

Sound::Sound::Sound (const ::Sound::Sound &snd)
{
  midi = snd.midi;
  Volume = snd.Volume;
  X = snd.X;
  Y = snd.Y;
  snd.midi = 0;
  SetSpeaker ();
}

Sound::Sound& Sound::Sound::operator= (const ::Sound::Sound &snd)
{
  destroy_me ();
  midi = snd.midi;
  Volume = snd.Volume;
  X = snd.X;
  Y = snd.Y;
  snd.midi = 0;
  SetSpeaker ();
  return *this;
}

Sound::Sound::~Sound ()
{
  stop ();
  if (midi)
    destroy_midi (midi);
}

void Sound::Sound::destroy_me ()
{
}

void Sound::Sound::play (int loopmode)
{
  if(!EnableSound)
    return;
  Calc ();
  if (midi)
    play_midi (midi, loopmode != 0);
  else
    sndPlaySound (fn, _volume, _balance);
  SetSpeaker ();
}

void Sound::Sound::stop ()
{
  if (midi)
    stop_midi ();
}

void Sound::Sound::volume (int vol)
{
  Volume = vol;
}

void Sound::Sound::position (int x, int y)
{
  X = x;
  Y = y;
}

void Sound::Sound::Calc ()
{
  _balance = int (127 * tanh (float(X) / 200) + 128);
  _volume = 50000 * Volume / (X*X + Y*Y + 50000);
}

void Sound::init ()
{
  if(!EnableSound)
    return;
  set_volume (255, 192); // MIDI war zu laut
}

void Sound::done ()
{
  if(!EnableSound)
    return;
  for (std::map<std::string, SAMPLE *>::iterator i = Sounds.begin(); i != Sounds.end(); ++i)
    if (i->second)
      destroy_sample (i->second);
}

int Sound::Sound::IsMIDI ()
{
  return !!midi; // boolean!
}
