
#ifndef _PICOTRACKERSYSTEM_PC_H_
#define _PICOTRACKERSYSTEM_PC_H_

#include <map>
#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

class picoTrackerSystem : public System {
public:
  static void Boot(int argc, char **argv);
  static void Shutdown();
  static int MainLoop();

public: // System implementation
  virtual unsigned long GetClock();
  virtual void GetBatteryState(BatteryState &state);
  virtual void SetDisplayBrightness(unsigned char value);
  virtual void Sleep(int millisec);
  virtual void PostQuitMessage();
  virtual unsigned int GetMemoryUsage();
  virtual void PowerDown(){};
  virtual void SystemBootloader();
  virtual void SystemReboot();
  virtual void SystemPutChar(int c);
  virtual uint32_t GetRandomNumber();
  virtual uint32_t Micros();
  virtual uint32_t Millis();

private:
  static bool invert_;
  static unsigned int lastBeatCount_;
  static EventManager *eventManager_;
};
#endif
