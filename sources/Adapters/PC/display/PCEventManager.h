#ifndef _PC_EVENTMANAGER_H_
#define _PC_EVENTMANAGER_H_

#include "UIFramework/SimpleBaseClasses/EventManager.h"

class PCEventManager : public EventManager {
public:
    PCEventManager();
    virtual ~PCEventManager();
    
    virtual int MainLoop() override;
    virtual void PostQuitMessage() override;
    virtual int GetKeyCode(const char *name) override;
};
#endif
