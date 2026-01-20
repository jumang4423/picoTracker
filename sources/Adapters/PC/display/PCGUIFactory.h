
#ifndef _PC_GUIFACTORY_H_
#define _PC_GUIFACTORY_H_

#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "PCEventManager.h"

class PCGUIFactory : public I_GUIWindowFactory {
private:
    PCEventManager eventManager_;
public:
  virtual I_GUIWindowImp &CreateWindowImp(GUICreateWindowParams &p) override;
  virtual EventManager *GetEventManager() override { return &eventManager_; }
};
#endif
