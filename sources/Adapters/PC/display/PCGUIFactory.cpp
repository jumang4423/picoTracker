
#include "PCGUIFactory.h"
#include "PCGUIWindowImp.h"

I_GUIWindowImp &PCGUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  return *(new PCGUIWindowImp(p));
}
