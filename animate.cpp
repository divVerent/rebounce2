#include "animate.h"
#include "defs.h"

namespace Objects
{
  int Animate (std::string &s)
  {
    return str2int (AnimateString (s));
  }
  std::string AnimateString (std::string &s)
  {
    if (s == "")
      return 0;
    int p = s.find (";");
    if (p == std::string::npos)
      return s;
    else
    {
      std::string r = s.substr (0, s.find (";"));
      s.erase (0, p + 1) += ";" + r;
      return r;
    }
  }
}
