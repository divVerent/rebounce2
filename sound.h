#ifndef __RP__SOUND_H__
#define __RP__SOUND_H__

#include <string>

struct MIDI;

namespace Sound
{
  class Sound
  {
   private:

    Sound ();
    void Calc ();
    mutable MIDI *midi;
    int _volume;
    int _balance;
    int Volume;
    int X;
    int Y;
    void SetSpeaker ();
    void destroy_me ();
    std::string fn;

   public:
    Sound& operator= (const Sound &snd);
    Sound (std::string);
    Sound (const Sound &);
    ~Sound ();
    void play (int loopmode = 0); // 0 -> no loop, 1 -> loop, 2 -> special loopmode, -n -> Allow n
    void stop ();
    void volume (int vol);
    void position (int x, int y);
    int IsMIDI();
  };


  void init ();
  void done ();
  void sndLoadSound (const std::string &fn);
}
#endif
