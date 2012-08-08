#include "dialogs.h"
#include "alleg.h"

namespace
{
  char msg_ok[] = "OK";
  char msg_cancel[] = "Abbrechen";
  char msg_yes[] = "Ja";
  char msg_no[] = "Nein";
  char msg_abort[] = "Abbrechen";
  char msg_retry[] = "Wiederholen";
  char msg_ignore[] = "Ignorieren";
  const char *GetMessageString (Dialogs::DialogButton but)
  {
    switch (but)
    {
     case Dialogs::OK:
      return msg_ok;
     case Dialogs::CANCEL:
      return msg_cancel;
     case Dialogs::YES:
      return msg_yes;
     case Dialogs::NO:
      return msg_no;
     case Dialogs::ABORT:
      return msg_abort;
     case Dialogs::RETRY:
      return msg_retry;
     case Dialogs::IGNORE:
      return msg_ignore;
     default:
      return "FCKGW-RHQQ2-YXRKT-8TG6W-2B7Q8";
    }
  }
}

Dialogs::DialogButton Dialogs::MessageBox (std::string Message, Dialogs::DialogButton b1, Dialogs::DialogButton b2, Dialogs::DialogButton b3)
{
  typedef const char *CString;
  int NumButtons = 0;
  CString buttons [3];
  if (b1 != 0)
    buttons[NumButtons++] = GetMessageString (b1);
  if (b2 != 0)
    buttons[NumButtons++] = GetMessageString (b2);
  if (b3 != 0)
    buttons[NumButtons++] = GetMessageString (b3);
  int Result = 0;
  if (NumButtons == 3)
    Result = ::alert3 (Message.c_str(), 0, 0, buttons[0], buttons[1], buttons[2], '1', '2', '3');
  if (NumButtons == 2)
    Result = ::alert (Message.c_str(), 0, 0, buttons[0], buttons[1], '1', '2');
  if (NumButtons == 1)
    Result = ::alert (Message.c_str(), 0, 0, buttons[0], 0, '1', 0);
  switch (Result)
  {
   case 1:
    return b1;
   case 2:
    return b2;
   case 3:
    return b3;
   default:
    return 0;
  }
}

namespace
{
  DIALOG readln[] =
  {
    { d_shadow_box_proc, 0, 0, 300, 70, 0, 0, 0, 0, 0, 0, 0, 0 },
    { d_text_proc, 10, 10, 280, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { d_edit_proc, 10, 30, 280, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { d_button_proc, 10, 50, 280, 10, 0, 0, 0, D_EXIT, 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };
}

std::string Dialogs::ReadLn (std::string prompt, std::string def)
{
  char buffer1[256];
  char buffer2[256];
  strcpy (buffer1, prompt.c_str());
  strcpy (buffer2, def.c_str());
  readln[1].dp = buffer1;
  readln[2].dp = buffer2;
  readln[2].d1 = 255;
  readln[3].dp = msg_ok;
  centre_dialog (readln);
  set_dialog_color (readln, gui_fg_color, gui_bg_color);
  popup_dialog (readln, 2);
  return buffer2;
}

namespace
{
  char **list = 0;
  const char *empty = "";
  const char *lister (int index, int *length)
  {
    if (!list)
      return empty;
    char **l = list;
    int len = 0;
    while (*l++)
      ++len;
    if (index < 0)
    {
      *length = len;
      return 0;
    }
    if (index > len)
      return empty;
    return list[index];
  }
  DIALOG listbox[] =
  {
    { d_shadow_box_proc, 0, 0, 300, 170, 0, 0, 0, 0, 0, 0, 0, 0 },
    { d_text_proc, 10, 10, 280, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { d_list_proc, 10, 30, 280, 110, 0, 0, 0, D_EXIT, 0, 0, (void *) lister, 0, 0 },
    { d_button_proc, 10, 150, 280, 10, 0, 0, 0, D_EXIT, 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };
}



int Dialogs::ListBox (std::string prompt, char **buf, int len)
{
  listbox[1].dp = const_cast <char *> (prompt.c_str());
  listbox[2].d1 = 1;
  listbox[3].dp = msg_ok;
  list = buf;
  centre_dialog (listbox);
  set_dialog_color (listbox, gui_fg_color, gui_bg_color);
  popup_dialog (listbox, 2);
  return listbox[2].d1;
}
