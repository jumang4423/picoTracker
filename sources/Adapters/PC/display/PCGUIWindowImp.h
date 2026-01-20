
#ifndef _PC_GUIWINDOWIMP_H_
#define _PC_GUIWINDOWIMP_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "UIFramework/BasicDatas/GUICreateWindowParams.h"
#include "Services/Midi/MidiService.h"
#include "chargfx.h"
#include <SDL.h>

class PCGUIWindowImp : public I_GUIWindowImp, public I_Observer {
public:
    PCGUIWindowImp(GUICreateWindowParams &p);
    virtual ~PCGUIWindowImp();

    // I_GUIGraphics implementation
    virtual void Clear(GUIColor &, bool overlay = false) override;
    virtual void SetColor(GUIColor &) override;
    virtual void ClearTextRect(GUIRect &) override;

    // Actually chargfx has chargfx_print.
    // But I_GUIGraphics defines DrawString.
    // picoTrackerGUIWindowImp implementation of DrawString? 
    // It's NOT implemented in the file I read! 
    // Ah, I_GUIWindowImp inherits I_GUIGraphics.
    
    virtual void DrawChar(const char c, GUIPoint &pos, GUITextProperties &) override;
    virtual void DrawString(const char *string, GUIPoint &pos, GUITextProperties &p, bool overlay) override {
        // Default implementation or loop over chars
        while(*string) {
             DrawChar(*string++, pos, p);
             pos._x += 8; // Assuming 8px width?
        }
    }

    virtual void DrawRect(GUIRect &r) override;
    virtual GUIRect GetRect() override;
    virtual void Invalidate() override;
    virtual void Lock() override;
    virtual void Unlock() override;
    virtual void Flush() override;

    // I_GUIWindowImp implementation
    virtual void PushEvent(GUIEvent &) override;

    // I_Observer implementation
    virtual void Update(Observable &o, I_ObservableData *d) override;

    void ProcessEvent(SDL_Event &event); // To handle input mapping if needed here, or just helper

private:
   static PCGUIWindowImp *instance_;
   chargfx_color_t GetColor(GUIColor &c);
   bool remoteUIEnabled_ = false;
};
#endif
