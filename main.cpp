#include "defs.h"

#include <string>
#include <iostream>
#include <map>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rebounce.h"

#include "milli.h"
#include "objects.h"
#include "world.h"
#include "sound.h"
#include "dialogs.h"
#include "border.h"
#include "keys.h"
#include "animate.h"

#define VERSION "2.6.0"

std::string Spielername = "*** N/A ***";
std::string SpielerID = "1234567890";

void TheEnd ();
void Credits (int = 0);

::Sound::Sound *MenuSound = 0;
bool CheatActivated = 0, CheatActivationAllowed = 0, NoHiscore = 0;

void SoundOn (std::string s = "intro.mid", int lm = 1)
{
  if (MenuSound)
    return;
  (MenuSound = new ::Sound::Sound (Filename(s)))->play (lm);
}

void SoundOff ()
{
  if (!MenuSound)
    return;
  delete MenuSound;
  MenuSound = 0;
}

::Sound::Sound *ClickSound = 0;

void MenuClickSound ()
{
  if (!ClickSound)
    ClickSound = new ::Sound::Sound (Filename("menu.wav"));
  ClickSound->play ();
}

void DoneMenuClickSound ()
{
  if (ClickSound)
    delete ClickSound;
}



template <int initval> class init_int
{
 private:
  int i;
 public:
  init_int (int i2 = initval) : i(i2)
  {
  }
  /*
     operator int& ()
     {
     return i;
     }
     */
  operator const int& () const
  {
    return i;
  }
};


std::map <std::string, std::string> hints, texts;
typedef std::map <std::string, init_int<-2> > sp_t;
sp_t SingleplayerDaten;
sp_t MultiplayerDaten;
std::string LevelWaehlen (const sp_t &Levels, bool NoScore = 0);
bool Verbessert;
bool very_first_run;


bitmap ConvertBitmap (bitmap bmp)
{
  if (!bmp)
    throw "null pointer: file not found?";
  bitmap bmp2 = create_optimized_bitmap (bmp->w, bmp->h);
  blit (bmp, bmp2, 0, 0, 0, 0, bmp->w, bmp->h);
  destroy_bitmap (bmp);
  return bmp2;
}

bitmap title_screen, background_screen;

void TextOut (bitmap buf, int x, int y, const std::string& _s);

void load_images()
{
  PALETTE rgb;
  title_screen = ConvertBitmap (load_bitmap (Filename("TITLE.PCX").c_str(), rgb));
  TextOut (title_screen, 160, 230, "v" VERSION " https://rebounce2.rm.cloudns.org");
  background_screen = ConvertBitmap (load_bitmap (Filename("back.pcx").c_str(), rgb));
}
void unload_images()
{
  free_optimized_bitmap (title_screen);
  free_optimized_bitmap (background_screen);
}



// change to mode 13h byte sequence
void ReadSinglePlayerData ()
{
  if(SAVEPREFIX != "")
    MKDIR(SAVEPREFIX.c_str(), 0755);

  errno = 0;

  PACKFILE *f = pack_fopen ((SAVEPREFIX + "rebounce.sav").c_str(), F_READ_PACKED);
  very_first_run = !!errno;

  SingleplayerDaten.erase (SingleplayerDaten.begin(), SingleplayerDaten.end());

  if (errno)
  {
    SpielerID = "RB_";
    for (int i = 0; i < 8; ++i)
      SpielerID += char (65 + Random(26));

    for (;/*ever*/;)
    {
      fade_screen (title_screen);
      Spielername = Dialogs::ReadLn ("Name, ID: " + SpielerID, "");
      if (!Spielername.length ())
	continue;                                      // Bitte den Namen!
      if (Spielername.find ("&") != std::string::npos)
	continue;                                      // kein Name mit &
      if (Spielername.find ("=") != std::string::npos)
	continue;                                      // kein Name mit =
      if (Spielername.find (":") != std::string::npos)
	continue;                                      // kein Name mit :
      if (Spielername.find ("'") != std::string::npos)
	continue;                                      // kein Name mit '
      if (Spielername.find ("\"") != std::string::npos)
	continue;                                      // kein Name mit "
      if (Spielername.find ("<") != std::string::npos)
	continue;                                      // kein Name mit <
      if (Spielername.find (">") != std::string::npos)
	continue;                                      // kein Name mit >
      break;
    }

    return;
  }

  char buf1[80], buf2[80];

  pack_fgets (buf1, sizeof buf1, f);
  char *p;
  if (!(p = strchr (buf1, ':')))
  {
    Spielername = "Seme Hacker";
  }
  else
  {
    *p = 0;
    // SpielerAddr = p + 1;
    Spielername = buf1;
  }

  pack_fgets (buf2, sizeof buf1, f);
  SpielerID = buf2;

  while (!pack_feof (f) && (errno == 0))
  {
    buf1[79] = 0;
    buf2[79] = 0;
    pack_fgets (buf1, sizeof buf1, f);
    pack_fgets (buf2, sizeof buf1, f);
    SingleplayerDaten [buf1] = init_int<-2>(str2int (buf2));
  }

  pack_fclose (f);
}

void WriteSinglePlayerData ()
{
  if(SAVEPREFIX != "")
    MKDIR(SAVEPREFIX.c_str(), 0755);

  errno = 0;

  PACKFILE *f = pack_fopen ((SAVEPREFIX + "rebounce.sav").c_str(), F_WRITE_PACKED);
  pack_fputs (std::string(Spielername + ":" /* + SpielerAddr */).c_str(), f);
  pack_fputs ("\n", f);
  pack_fputs (SpielerID.c_str(), f);
  pack_fputs ("\n", f);

  FILE *f2 = fopen ((SAVEPREFIX + "scores.htm").c_str(), "w");
  fprintf (f2, "<HTML><BODY onLoad=\"document.forms[0].submit();\"><H1>ReBounce II scores - %s</H1>", Spielername.c_str());
  fprintf (f2, "<SCRIPT LANGUAGE=\"JavaScript\">document.write('<FORM METHOD=POST ACTION=\"https://rebounce2.rm.cloudns.org/hiscore.pl\">');</SCRIPT>");
  fprintf (f2, "<NOSCRIPT>Please activate JavaScript to make this work!</NOSCRIPT>");
  fprintf (f2, "<INPUT TYPE=hidden NAME=uid VALUE=\"%s\">", SpielerID.c_str());
  fprintf (f2, "<INPUT TYPE=hidden NAME=name VALUE=\"%s\">", Spielername.c_str());
  // fprintf (f2, "<INPUT TYPE=hidden NAME=addr VALUE=\"%s\">", SpielerAddr.c_str());
  fprintf (f2, "<UL>");
  int nr = 0;

  std::string s = " " + SpielerID;

  sp_t::iterator i;

  for (i = SingleplayerDaten.begin(); i != SingleplayerDaten.end(); ++i)
  {
    pack_fputs (i->first.c_str(), f);
    pack_fputs ("\n", f);
    pack_fputs (const_cast <char *> (int2str (i->second).c_str()), f);
    pack_fputs ("\n", f);
    std::string s2 = hints [i->first];
    size_t p;
    while ((p = s2.find ("\"")) != std::string::npos)
      s2.replace (p, 1, "''");
    if (i->second > 0)
    {
      fprintf (f2,
	  "<LI>%s - %d Punkte<INPUT TYPE=hidden NAME=score%d VALUE=\"%s:%s:%d\"></LI>\n",
	  s2.c_str(),
	  i->second / 100,
	  ++nr,
	  i->first.c_str(),
	  s2.c_str(),
	  int (i->second)
	  );
      s += "&" + i->first + ":" + s2 + ":" + int2str (i->second);
    }
  }
  int sc = 0;
  for (i = SingleplayerDaten.begin(); i != SingleplayerDaten.end(); ++i)
    sc += (int(i->second) > 0) ? int(i->second) : 0;
  fprintf (f2,
      "<LI>%s - %d Punkte<INPUT TYPE=hidden NAME=score%d VALUE=\"%s:%s:%d\"></LI>\n",
      "total score",
      sc / 100,
      ++nr,
      "~",
      "total score",
      sc
      );
  s += "&~:total score:" + int2str (sc);
  int mychksum = 31337;
  for (int n = s.length() - 1; n >= 0; --n)
  {
    mychksum = (mychksum << 3) | (mychksum >> 13);
    mychksum += s[n];
    mychksum &= 0xFFFF;
  }
  fprintf (f2, "</UL><BR>If there were some JavaScript errors, they may be because you are offline. You have to be online for registering!"
      "<INPUT TYPE=hidden NAME=chksum VALUE=\"%d\">"
      "<INPUT TYPE=submit VALUE=\"Send\">"
      "</FORM>"
      "</BODY></HTML>", mychksum);

  fclose (f2);

  pack_fclose (f);
}

std::string upcase (const std::string &s)
{
  const char *c = s.c_str();
  char *str = new char [strlen(c) + 1];
  strcpy (str, c);
  strupr (str);
  std::string s2 = str;
  delete [] str;
  return s2;
}

namespace Multiplayer
{
  void Reset (bool mp, int AnzSiege = 3);
}

float const EASY = 2.5;
float const MEDIUM = 1.5;
float const HARD = 1.0;
int timeleft;
float difficulty = EASY;

void GetEMH (std::string &s)
{
  switch (s[s.length() - 1])
  {
   case '0': difficulty = EASY;        break;
   case '1': difficulty = MEDIUM;      break;
   case '2': difficulty = HARD;        break;
  }
  s = s.substr (0, s.length() - 1);
}

void SetEMH (std::string &s)
{
  if (ABS(difficulty - EASY) < 0.1)
    s += '0';
  else if (ABS(difficulty - MEDIUM) < 0.1)
    s += '1';
  else
    s += '2';
}

void ReadHints ()
{
  PACKFILE *f = pack_fopen (Filename("hints.txt").c_str(), F_READ);
  if (!f)
    throw "File not found: " + Filename("hints.txt");

  char name[20], descr[80];

  name[19] = 0;
  descr[79] = 0;
  char *textfile;
  while (!pack_feof (f))
  {
    *name = 0;
    pack_fgets (name, sizeof (name) - 1, f);
    *descr = 0;
    pack_fgets (descr, sizeof (descr) - 1, f);
    if ((textfile = strchr (descr, '^')))
    {
      texts[upcase(name) + "0"] =
	texts[upcase(name) + "1"] =
	texts[upcase(name) + "2"] = std::string(textfile + 1);
      *textfile = 0;
    }
    hints[upcase(name) + "0"] = std::string(descr) + " (w/o)";
    hints[upcase(name) + "1"] = std::string(descr) + " (try)";
    hints[upcase(name) + "2"] = std::string(descr);
    if (name[MAX(unsigned (0U),unsigned (strlen(name)-1))] == 'M')
    {
      MultiplayerDaten[upcase(name) + "2"] = 0;
      MultiplayerDaten[upcase(name) + "1"] = 0;
      MultiplayerDaten[upcase(name) + "0"] = 0;
    }
  }

  pack_fclose (f);
}

void SearchAndReplace (std::string &s, const std::string &search, const std::string &replace)
{
  int i;
  while ((i = s.find (search)) != std::string::npos)
    s.replace (i, search.length(), replace);
}


bitmap buf;

void title ()
{
  fade_screen (title_screen);
  readkey ();
  MenuClickSound ();
}

std::string CodeEingeben ();

void Background (bitmap buf)
{
  blit (background_screen, buf, 0, 0, 0, 0, buf->w, buf->h);
}

void TextOut (bitmap buf, int x, int y, const std::string& _s)
{
  if (!_s.length ())
    return;
  std::string s = _s;
  bool selected = s [0] == '\001';
  if (selected)
    s = s.substr (1);
  y -= text_height (font) / 2;
  long bg = selected ? makecol (255, 128, 128) : makecol (0, 0, 0);
  long fg = selected ? makecol (0, 0, 0) : makecol (255, 255, 0);
  textout_centre_border_ex (buf, font, s.c_str(), x+1, y+1, fg, bg);
}
#include <stdarg.h>
std::vector <std::string> MakeVector (const char *p ...)
  // array of const char *
{
  va_list argv;
  va_start (argv, p);
  std::vector<std::string> v;
  while (p)
  {
    v.push_back (p);
    p = va_arg (argv, const char *);
  }
  va_end (argv);
  return v;
}
static int const LINE_HEIGHT = 12;
void DisplayMessage (bitmap buf, int x, int y, const std::vector <std::string> &str)
{
  y -= LINE_HEIGHT * (str.size () - 1) / 2;
  for (int i = 0; i < str.size (); ++i)
  {
    const std::string &s = str [i];
    TextOut (buf, x, y, s);
    y += LINE_HEIGHT;
  }
}

int SelectBox (bitmap buf, std::string head, const std::vector <std::string> &selections, bool NoEscape = 0)
{
  std::vector <std::string> v (buf->h / LINE_HEIGHT * 2 / 3);
  int pos = 0;
  int scrollpos = 0;
  bool first = true;
  for (;/*ever*/;)
  {
    Background (buf);
    TextOut (buf, buf->w / 2, buf->h / 8, head);
    while (pos < scrollpos)
      --scrollpos;
    while (pos > scrollpos + v.size () - 3)
      ++scrollpos;
    if (scrollpos)
      v [0] = "<UP>";
    else
      v [0] = "";
    if (scrollpos + v.size ()  - 2 < selections.size ())
      v [v.size () - 1] = "<DOWN>";
    else if (scrollpos + v.size () - 2 == selections.size ())
      v [v.size () - 1] = "<END>";
    else
      v [v.size () - 1] = "";
    for (int i = 1; i < v.size () - 1; ++i)
    {
      int idx = i + scrollpos - 1;
      if (idx < selections.size ())
	v [i] = selections [idx];
      else
	v [i] = "";
      if (idx == pos)
	v [i] = "\001" + v [i];
    }
    DisplayMessage (buf, buf->w / 2, buf->h * 9 / 16, v);
    (first ? fade_screen : update_screen) (buf);
    first = 0;
    switch (readkey () >> 8)
    {
     case KEY_ESC:
      if (NoEscape)
	break;
      MenuClickSound ();
      return -1;
      break;
     case KEY_UP:
      pos
	&& (MenuClickSound (), --pos);
      break;
     case KEY_DOWN:
      ((++pos == selections.size ())
	&& pos--)
	|| (MenuClickSound (), 1);
      break;
     case KEY_HOME:
      if (pos) MenuClickSound ();
      pos = 0;
      break;
     case KEY_END:
      if (pos != selections.size () - 1) MenuClickSound ();
      pos = selections.size () - 1;
      break;
     case KEY_PGUP:
      if (pos)
      {
	pos -= v.size() - 2;
	(pos < 0) && (pos = 0);
	MenuClickSound ();
      }
      break;
     case KEY_PGDN:
      if (pos != selections.size () - 1)
      {
	pos += v.size() - 2;
	(pos >= selections.size ()) && (pos = selections.size () - 1);
	MenuClickSound ();
      }
      break;
     case KEY_ENTER:
      MenuClickSound ();
      return pos;
      break;
    }
  }
  throw "Cannot get here!";
  return -2;
}


std::string ChooseEMH (std::string s)
{
  std::string h = hints [s];
  GetEMH (s);

  static const float D[] = { EASY, MEDIUM, HARD };
  int n;
  n = SelectBox (buf, h,
      MakeVector ("WIMPS ONLY", "TRY IT", "FORGET IT", (const char *) 0),
      0);
  if (n < 0)
    return "";
  difficulty = D [n];
  SetEMH (s);
  return s;
}
std::string SelectStartLevel ()
{
  std::vector <std::string> Levels;
  std::vector <std::string> _Levels;
  bool first = 1;
  for (std::string s0 = startlevel;
      first || s0 != startlevel;
      first = 0)
  {
    std::string s = Objects::AnimateString (startlevel);
    _Levels.push_back (s);
    Levels.push_back (hints [s]);
  }
  int n = SelectBox (buf, "Choose a starting level", Levels);
  if (n < 0)
    return "";
  return _Levels [n];
}

bool main_menu (std::string &s)
{
  static bool first_run = 1;
  if (first_run && !CheatActivationAllowed)
  {
    if (very_first_run)
      Credits ();
    else
      title ();
  }

  first_run = 0;

  Multiplayer::Reset (0, 0);

  for (;;)
  {
    int sc = 0;
    for (sp_t::iterator i = SingleplayerDaten.begin(); i != SingleplayerDaten.end(); ++i)
      sc += (int(i->second) > 0) ? int(i->second) : 0;
    switch (SelectBox (buf, "ReBounce II - Score: " + int2str (sc / 100), MakeVector (
	    "new single-player game",
	    "continue single-player game",
	    "new duel",
	    "credits",
	    "exit to your OS",
	    (CheatActivationAllowed ? "" : (const char *) 0),
	    (const char *) 0), 1)
	)
    {
     case 0:
      s = SelectStartLevel ();
      if (s == "")
	break;
      s = ChooseEMH (s);
      if (s == "")
	break;
      return 1;
      break;
     case 1:
      s = LevelWaehlen (SingleplayerDaten);
      if (s == "")
	break;
      return 1;
      break;
     case 2:
      s = LevelWaehlen (MultiplayerDaten, 1);
      if (s == "")
	break;
      Multiplayer::Reset (1, 3);
      return 1;
      break;
     case 3:
      Credits (0);
      break;
     case 4:
      s = "";                    // QUIT!
      return 0;
      break;
     case 5:
      if (CheatActivationAllowed)
      {
	Credits (1);
        CheatActivated = 1;
      }
      break;
    }
  }
}

bool DisplayHint (std::string s)
{
  Background (buf);
  std::vector <std::string> v;
  v.push_back (hints[upcase(s)]);
  v.push_back ("");
  v.push_back ("- press any key -");
  DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
  fade_screen (buf);

  while (keypressed())
    readkey ();

  readkey ();

  MenuClickSound ();

  return 1;
}

namespace Multiplayer
{
  int AnzahlSiege = 3;
  bool Multiplayer = 0;
  bool Vertauscht = 0;
  int p1 = 0;
  int p2 = 0;

  void Reset (bool mp, int AnzSiege)
  {
    Multiplayer = mp;
    AnzahlSiege = AnzSiege;
    Vertauscht = 0;
    p1 = 0;
    p2 = 0;
  }

  void DrawGame ()
  {
    Background (buf);
    std::vector <std::string> v;
    v.push_back ("draw game");
    v.push_back ("");
    v.push_back ("- press any key -");
    DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
    fade_screen (buf);
    while (keypressed())
      readkey ();
    readkey ();
    MenuClickSound ();
  }

  void Player1Wins ()
  {
    Background (buf);
    std::vector <std::string> v;
    v.push_back ("player 1 wins");
    v.push_back ("");
    v.push_back ("- press any key -");
    DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
    fade_screen (buf);
    ++p1;
    while (keypressed())
      readkey ();
    readkey ();
    MenuClickSound ();
  }

  void Player2Wins ()
  {
    Background (buf);
    std::vector <std::string> v;
    v.push_back ("player 2 wins");
    v.push_back ("");
    v.push_back ("- press any key -");
    DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
    fade_screen (buf);
    ++p2;
    while (keypressed())
      readkey ();
    readkey ();
    MenuClickSound ();
  }

  bool Spieler1Sieg ()
  {
    return 1;
  }

  bool Spieler2Sieg ()
  {
    return 1;
  }

  bool DisplayMultiplayerInfo ()
  {
    std::string Spieler1Siege = "";
    std::string Spieler2Siege = "";
    for (int i = 0; i < AnzahlSiege; ++i)
    {
      Spieler1Siege += (i < p1 ? "X" : "_");
      Spieler2Siege += (i < p2 ? "O" : "_");
    }
    Background (buf);
    std::vector <std::string> v;
    v.push_back ("player 1: " + Spieler1Siege);
    v.push_back ("player 2: " + Spieler2Siege);
    v.push_back ("- press any key -");
    DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
    fade_screen (buf);
    while (keypressed())
      readkey ();
    char c = readkey() >> 8;
    MenuClickSound ();
    if (p1 >= AnzahlSiege)
      return Spieler1Sieg ();
    if (p2 >= AnzahlSiege)
      return Spieler2Sieg ();
    return c == KEY_ESC;
    // Vertauscht = !Vertauscht;
  }
}

bool MultiplayerMenu (bool b, std::string &s, std::string s2)
{
  using namespace Multiplayer;
  if (s[0] == '0')
    DrawGame ();
  else
  {
    if ((s[0] == '1') ^ Vertauscht)
      Player1Wins ();
    else
      Player2Wins ();
  }
  if (DisplayMultiplayerInfo ())
  {
    s = "";
    return 0;
  }
  s = s2;
  return 1;
}

bool menu (bool b, std::string &s, std::string s2)
{
  if (Multiplayer::Multiplayer)
  {
    if (MultiplayerMenu (b, s, s2))
      return 1;
    else
    {
      s = "";
      b = 1;
    }
  }

  if (Verbessert)
  {
    blit (savescreen, buf, 0, 0, 0, 0, buf->w, buf->h);
    std::vector <std::string> v;
    v.push_back ("New record!");
    v.push_back (hints [upcase (s2)]);
    v.push_back ("Score: " + (int2str (SingleplayerDaten [upcase(s2)] / 100)));
    v.push_back ("- press any key -");
    DisplayMessage (buf, buf->w / 2, buf->h / 2, v);
    fade_screen (buf);
    while (keypressed())
      readkey ();
    readkey ();
    MenuClickSound ();
  }

  if (s.substr(0, 4) == "\\END")
  {
    s = "";
    TheEnd ();
  }

  if (b && s == "")
  {
    SoundOn ();
    b = main_menu (s);
    SoundOff ();
    if (s == "")
      return 0;
  }
  if (b)
  {
    return DisplayHint (s);
  }
  blit (savescreen, buf, 0, 0, 0, 0, buf->w, buf->h);
  DisplayMessage (buf, buf->w / 2, buf->h / 2, MakeVector
      (
       "G A M E   O V E R",
       "",
       "Did you choose the wrong way?",
       "",
       "press ENTER to try again",
       "press ESC to return to main menu",
       (const char *) 0
      ));
  fade_screen (buf);

  while (keypressed())
    readkey ();

  char c;
  do
  {
    c = readkey() >> 8;
  }
  while ((c != KEY_ENTER) && (c != KEY_ESC));

  MenuClickSound ();

  if (c == KEY_ENTER)
  {
    s = s2;
    return DisplayHint (s);
  }
  else
  {
    s = "";
    return menu (1, s, "");
  }
}


void Eintragen (std::string s, bool b, int timeleft)
{
  s = upcase (s);
  std::string s2 = s;
  for (Verbessert = 0; s[s.length() - 1] >= '0'; --s[s.length() - 1])
  {
    GetEMH (s);
    SetEMH (s);
    if (b)
    {
      if (int (SingleplayerDaten[s]) < timeleft / difficulty / difficulty)
      {
	Verbessert = 1;
	SingleplayerDaten [s] = init_int<-2>(static_cast<int>(timeleft / difficulty / difficulty));
      }
    }
    else
      if (SingleplayerDaten [s] == -2)
	SingleplayerDaten[s] = init_int<-2>(-1);
  }
  GetEMH (s2);
  SetEMH (s2);
}


extern "C" int _preload (const char *fn, int, void *)
{
  Sound::sndLoadSound (fn);
  return 0;
}

void PreloadSounds ()
{
  for_each_file_ex (Filename ("*.wav").c_str(), 0, 0, _preload, 0);
}
void ScrollTextFile (std::string s);
int main(int argc, char **argv)
{
#ifndef DONOTCATCH
  try
#endif
  {
    bool Use24 = 1;
    int ExitCode = 0;
    int EnableFullscreen = 1;
    bool AutoScreenshot = 0;
    for (int i = 1; i < argc; ++i)
    {
      if (!strcmp (argv [i], "low"))
      {
	Use24 = 0;
	continue;
      }
      char s[16];
      if (sscanf (argv [i], "start=%15s", s) == 1)
      {
	startlevel = s;
	NoHiscore = 1;
	continue;
      }
      if (!strcmp (argv [i], "novsync"))
      {
	EnableVSync = 0;
	continue;
      }
      if (!strcmp (argv [i], "nosound"))
      {
	EnableSound = 0;
	continue;
      }
      if (!strcmp (argv [i], "window"))
      {
	EnableFullscreen = 0;
	continue;
      }
      if (!strcmp (argv [i], "bigwindow"))
      {
	EnableFullscreen = -1;
	continue;
      }
      if (!strcmp (argv [i], "thomas-jaeger"))
      {
	for (int *p = keycodes; *p != 0; ++++p)
	  if (*(p+1) == nsEntity::Entity::CHEAT)
	    *p = KEY_V;
	CheatActivationAllowed = 1;
	NoHiscore = 1;
	continue;
      }
      if (!strcmp (argv [i], "screenshot"))
      {
        AutoScreenshot = 1;
	NoHiscore = 1;
	continue;
      }
      std::cerr << "Options: " << std::endl;
      std::cerr << " window    disable fullscreen" << std::endl;
      std::cerr << " bigwindow disable fullscreen, big window" << std::endl;
      std::cerr << " novsync   disable vsync" << std::endl;
      std::cerr << " low       Use lower color depth" << std::endl;
      return 0;
    }

    set_uformat (U_ASCII);

    allegro_init ();
    set_display_switch_mode (SWITCH_PAUSE);
    Timer::init_timer ();
    graphics_init (Use24, EnableFullscreen);
    sound_init ();
    Border (0, 0, 0);

    install_mouse ();
    install_keyboard ();
    set_keyboard_rate (300, 300);  // enough!
    Objects::init ();
    World::init ();
    Sound::init ();

    clear (screen);
    buf = create_optimized_bitmap (SCRWIDTH, SCRHEIGHT);
    clear (buf);
    TextOut (buf, SCRWIDTH / 2, SCRHEIGHT / 2 - 4, "loading...");
    update_screen (buf);

    ReadHints ();

    load_images ();

#ifndef DONOTCATCH
    try
#endif
    {
      bool b = 1;
      std::string s = "";
      std::string s2 = s;

      PreloadSounds ();
      SoundOn ();

      if(AutoScreenshot)
      {
        int timefactor;
	s = startlevel;
        GetEMH (s);
        b = play (buf, s, timeleft, timefactor, difficulty, 0, CheatActivationAllowed, 1);
      }
      else
      {
        ReadSinglePlayerData ();
        Verbessert = 0;
        while (menu(b, s, s2))
        {
          s2 = s;
          GetEMH (s);
          Border();
          Verbessert = 0;
          int k = 0;
          if (Multiplayer::Multiplayer)
          {
            for (int *p = keycodes; *p; ++p)
              if (*++p == nsEntity::Entity::SUICIDE)
              {
                k = *--p;
                *p++ = KEY_MAX;
              }
          }
          int timefactor;
          b = play (buf, s, timeleft, timefactor, difficulty, CheatActivated, NoHiscore, 0);

          if (!Multiplayer::Multiplayer)
          {
            Eintragen (s2, b, timeleft * timefactor / 1000);
            if (!NoHiscore) {
              WriteSinglePlayerData ();
            }
          }
          else
          {
            for (int *p = keycodes; *p; ++p)
              if (*++p == nsEntity::Entity::SUICIDE)
                *(p-1) = k;
          }
          Verbessert = Verbessert && timeleft;
          Border(0, 0, 0);
          SetEMH (s);
          if (b && texts [upcase (s)] != "")
          {
            ScrollTextFile (texts [upcase (s)]);
          }
        }
      }
    }
#ifndef DONOTCATCH
    catch (const std::string &msg)
    {
      Dialogs::MessageBox (msg, Dialogs::CANCEL);
    }
    catch (const char *msg)
    {
      Dialogs::MessageBox (msg, Dialogs::CANCEL);
    }
    catch (...)
    {
      Dialogs::MessageBox ("UNKNOWN EXCEPTION", Dialogs::CANCEL);
      throw;
    };
#endif

    if (!NoHiscore)
    {
      WriteSinglePlayerData ();
#ifndef NOHISCORE
      Background (buf);
      DisplayMessage (buf, buf->w / 2, buf->h / 2, MakeVector
	  (
	   "Do you want to update the",
	   "worldwide highscores?",
	   "\001https://rebounce2.rm.cloudns.org",
	   "(Y / N)",
	   (const char *) 0
	  )
	  );
      fade_screen (buf);
      char c;
      do
      {
	c = toupper (readkey () & 0xFF);
      }
      while (c != 'N' && c != 'Y');
      if (c == 'Y')
	ExitCode = 1;
      MenuClickSound ();
#endif
    }

    clear (background_screen);

    if(!AutoScreenshot)
      fade_screen (background_screen);

    free_optimized_bitmap (buf);
    unload_images ();

    DoneMenuClickSound ();

    Sprite::Sprites.erase (Sprite::Sprites.begin(), Sprite::Sprites.end());
    Sound::done ();

    graphics_done ();

    Timer::done_timer ();

    nsEntity::DoneEntity ();

    allegro_exit ();

    if (ExitCode)
#ifdef ALLEGRO_DOS
      system (("start " + SAVEPREFIX + "scores.htm").c_str());
#else
#ifdef ALLEGRO_WINDOWS
      system (("start " + SAVEPREFIX + "scores.htm").c_str());
#else
#ifdef ALLEGRO_UNIX
      system (("xdg-open " + SAVEPREFIX + "scores.htm").c_str());
#else
    std::cout << "Please open " + SAVEPREFIX + "scores.htm in your" << std::endl
      << "favorite Web browser. It has to support JavaScript, however." << std::endl
      << "Netscape, IE, Opera and Konqueror are suitable for this." << std::endl
      << std::endl;
#endif
#endif
#endif

    check_bitmap_memory ();

    return ExitCode;
  }
#ifndef DONOTCATCH
  catch (std::bad_alloc)
  {
    allegro_exit ();
    std::cerr << "Out of memory" << std::endl;
    check_bitmap_memory ();
  }
  catch (const std::string &s)
  {
    allegro_exit ();
    std::cerr << "Uncaught exception - " << s << std::endl;
    check_bitmap_memory ();
  }
  catch (const char *s)
  {
    allegro_exit ();
    std::cerr << "Uncaught exception - " << s << std::endl;
    check_bitmap_memory ();
  }
  catch (...)
  {
    allegro_exit ();
    std::cerr << "Unknown exception" << std::endl;
    check_bitmap_memory ();
  }
#endif
  return 0;
}

END_OF_MAIN ();


std::string LevelWaehlen (const sp_t &Levels, bool NoScores)
{
  std::multimap <std::string, int> lvl;
  for (sp_t::const_iterator i = Levels.begin(); i != Levels.end(); ++i)
  {
    lvl.insert (std::pair<std::string, int>(upcase(i->first), i->second));
  }
  if (lvl.begin() == lvl.end())
    return "";
  std::vector <std::string> LevelIDs;
  std::vector <std::string> LevelNames;
  for (std::multimap <std::string, int>::iterator i = lvl.begin (); i != lvl.end (); ++i)
  {
    LevelIDs.push_back (i->first);
    LevelNames.push_back (hints [i->first] +
	(
	 NoScores ? "" :
	 (i->second < 0) ? " - not completed" :
	 (i->second ==0) ? " - time was up" :
	 (" - score: " + int2str (i->second / 100))
	)
	);
  }
  int result = SelectBox (buf, "choose a level", LevelNames);
  if (result < 0)
    return "";
  else
    return LevelIDs [result];
}

const char WAHLLOS[] = "### WAHLLOS ###";
void ScrollText (const char * const s[], int UseWahllos = 1, std::string sound = "")
{
  bool SwitchBack = 0;
  if (sound != "")
  {
    SwitchBack = MenuSound;
    SoundOff ();
    SoundOn (sound, 0);
  }
  // Abspann-Code
  std::vector <std::string> lines ((buf->h + 3 * LINE_HEIGHT - 1) / LINE_HEIGHT);
  const char * const * next = s;
  int y = 0;
  for (;/*ever*/;)
  {
    ++y %= LINE_HEIGHT;
    if (!y)
    {
      copy (lines.begin () + 1, lines.end (), lines.begin ());
      if (!*++next)
	break;
      if (!strcmp (*next, WAHLLOS))
      {
	int first = 0;
	do
	{
	  ++next;
	}
	while (!UseWahllos && (*(next - 1) != WAHLLOS || !first++));
      }
      *(lines.end () - 1) = std::string (*next);
    }
    Background (buf);
    DisplayMessage (buf, buf->w / 2, buf->h / 2 - y, lines);
    update_screen (buf);       // 1 VBL
    if (!keypressed ())
    {
      if (UseWahllos)
      {
	update_screen (buf);       // 1 VBL
	update_screen (buf);       // 1 VBL
	update_screen (buf);       // 1 VBL
	update_screen (buf);       // 1 VBL
      }
    }
  }
  while (keypressed ())
    readkey ();
  readkey ();
  if (SwitchBack)
  {
    SoundOff ();
    SoundOn ();
  }
}
void ScrollTextFile (std::string s)
{
  PACKFILE *f = pack_fopen (Filename (s).c_str (), "r");
  if (!f)
    return;
  char *lines [256];
  char **p = lines;
  strcpy (*p = new char [50], "");
  char c [2];
  *c = 0;
  *(c + 1) = 0;
  while (!pack_feof (f))
  {
    pack_fread (c, 1, f);
    if (*c == 13)
      continue;
    if (*c == 10)
    {
      strcpy (*++p = new char [50], "");
      continue;
    }
    strcat (*p, c);
  }
  pack_fclose (f);
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "");
  strcpy (*++p = new char [50], "- press any key -");
  strcpy (*++p = new char [50], "");
  *++p = 0;
  ScrollText (lines, 0, lines [0]);
  for (p = lines; *p; ++p)
  {
    delete [] *p;
  }
}
void Credits (int EndingGame)
{
  const char *CreditStr[] =
  {
    0,
    WAHLLOS,
    "Yes, you CAN play this game!",
    "I really thought no one would",
    "ever see this ending sequence.",
    "I was wrong, as you now see.",
    "Try the duel mode for further",
    "\001challenge!",
    "",
    "You played",
    WAHLLOS,
    "\001ReBounce II",
    "\001===========",
    "",
    "\001written in C++ using the ALLEGRO",
    "\001library by Shawn Hargreaves",
    "",
    "\001available for MS-DOS, Win32/DirectX",
    "\001and Linux (X-Window and console",
    "\001support) on",
    "\001https://rebounce2.rm.cloudns.org",
    "",
    "",
    "",
    "\001-- programming --",
    "Rudolf Polzer",
    "",
    "\001-- leveldesign --",
    "Rudolf Polzer",
    "",
    "\001-- music --",
    "Rudolf Polzer",
    "",
    "\001-- debugging --",
    "Rudolf Polzer",
    "Christian Kottmann",
    "",
    "\001-- thanks to --",
    "\001,DJ Delorie' (www.delorie.com)",
    "for DJGPP, the DOS port of",
    "the well-known GCC compiler",
    "",
    "\001GNU",
    "for this GCC compiler",
    "",
    "\001Shawn Hargreaves",
    "\001(http://www.allegro.cc)",
    "for ALLEGRO, a good C library with",
    "everything a game programmer needs",
    "in it",
    "",
    "\001Bjarne Stroustrup",
    "for C++, the programming language",
    "I used",
    "",
    "\001Robert Hoehne",
    "for RHIDE, an IDE which works on DOS",
    "and Linux and which is mostly",
    "B*rland-compatible",
    WAHLLOS,
    "",
    "\001Bill Gate$",
    "for a big number of Windoze",
    "\001CRASHes",
    "",
    "\001Stephanie Polzer",
    "for lots of useless comments",
    "",
    "\001Norbert Gassel",
    "for strictly rejecting C++",
    "",
    "\001mycgiserver.com",
    "They canceled CGI-support to increase",
    "Java performance, but they didn't",
    "change their domain name",
    "",
    "\001Cult of the Dead Cow",
    "for BO2k, two times in my mail",
    "",
    "\001GrammarSoft",
    "for LOVELETTER-FOR-YOU.txt.vbs",
    "8 times in my mail",
    "",
    "\001Thomas Jaeger",
    "for cheating when playing cards",
    "",
    "\001Linus Torvalds",
    "for a stable OS which is still",
    "lacking a lot of device drivers",
    "",
    "\001M$'s ScanDisk team",
    "for their useless waste of time",
    "",
    "\001McPuke's",
    "for the BSE burger",
    "",
    "\001s#(e.{1,2}?)e\\\\#lc$1#g;",
    "print $_,<STDIN>;",
    "for Perl",
    "",
    "\001Deutsche Telekom",
    "for T-DSL",
    "",
    "\001T-Online",
    "for the ADSL flatrate",
    "",
    "\001AOL",
    "for only supporting crappy",
    "systems like Windoze",
    "",
    "\001http://www.debian.org",
    "for a clean Linux distribution",
    "and compiling the mingw32 cross",
    "compiler so that you can download",
    "it using apt-get install mingw32",
    "",
    "\001Micro$oft",
    "for Outlook Express, a newsreader",
    "that just does not work",
    "",
    "\001anyone",
    "I've forgotten",
    "",
    "\001you",
    "for playing this game up to the end",
    "",
    "\001T H X !",
    WAHLLOS,
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "END",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "- press any key -",
    "",
    0
  };
  ScrollText (CreditStr, EndingGame, EndingGame ? "ende.mid" : "");
}
void TheEnd ()
{
  Credits (1);
}
