#ifndef __RP__DIALOGS_H__
#define __RP__DIALOGS_H__

#include "defs.h"

namespace Dialogs
{
  std::string ReadLn (std::string Prompt, std::string Default);
  int ListBox (std::string prompt, char **buf, int len);
  enum _DialogButton { OK=1, CANCEL=2, YES=3, NO=4, ABORT=5, RETRY=6, IGNORE=7 };
  typedef int DialogButton;
  DialogButton MessageBox (std::string Message, DialogButton b1, DialogButton b2 = 0, DialogButton b3 = 0);
}

#endif
